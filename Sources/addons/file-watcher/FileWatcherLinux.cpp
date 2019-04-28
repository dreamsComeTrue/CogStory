/**
		Copyright (c) 2009 James Wynn (james@jameswynn.com)

		Permission is hereby granted, free of charge, to any person obtaining a copy
		of this software and associated documentation files (the "Software"), to deal
		in the Software without restriction, including without limitation the rights
		to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
		copies of the Software, and to permit persons to whom the Software is
		furnished to do so, subject to the following conditions:

		The above copyright notice and this permission notice shall be included in
		all copies or substantial portions of the Software.

		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
		THE SOFTWARE.

		James Wynn james@jameswynn.com
*/

#include <FileWatcherLinux.h>

#if FILEWATCHER_PLATFORM == FILEWATCHER_PLATFORM_LINUX

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

/* "readdir" etc. are defined here. */
#include <dirent.h>

#define BUFF_SIZE ((sizeof (struct inotify_event) + FILENAME_MAX) * 1024)

namespace FW
{

	struct WatchStruct
	{
		WatchID mWatchID;
		String mDirName;
		FileWatchListener* mListener;
	};

	//--------
	FileWatcherLinux::FileWatcherLinux ()
	{
		mFD = inotify_init ();
		if (mFD < 0)
			fprintf (stderr, "Error: %s\n", strerror (errno));

		mTimeOut.tv_sec = 0;
		mTimeOut.tv_usec = 0;

		FD_ZERO (&mDescriptorSet);
	}

	//--------
	FileWatcherLinux::~FileWatcherLinux ()
	{
		WatchMap::iterator iter = mWatches.begin ();
		WatchMap::iterator end = mWatches.end ();
		for (; iter != end; ++iter)
		{
			delete iter->second;
		}
		mWatches.clear ();
	}

	//--------
	WatchID FileWatcherLinux::addWatch (const String& directory, FileWatchListener* watcher, bool recursive)
	{
		int wd = inotify_add_watch (
			mFD, directory.c_str (), IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE | IN_MOVED_FROM | IN_DELETE | IN_MODIFY);
		if (wd < 0)
		{
			if (errno == ENOENT)
				throw FileNotFoundException (directory);
			else
				throw Exception (strerror (errno));

			//			fprintf (stderr, "Error: %s\n", strerror(errno));
			//			return -1;
		}

		WatchStruct* pWatch = new WatchStruct ();
		pWatch->mListener = watcher;
		pWatch->mWatchID = wd;
		pWatch->mDirName = directory;

		mWatches.insert (std::make_pair (wd, pWatch));

		if (recursive)
		{
			addWatchRecursive (directory, watcher);
		}

		return wd;
	}

	//--------
	void FileWatcherLinux::addWatchRecursive (const String& directory, FileWatchListener* watcher)
	{
		DIR* d = opendir (directory.c_str ());

		/* Check it was opened. */
		if (!d)
		{
			fprintf (stderr, "Cannot open directory '%s': %s\n", directory.c_str (), strerror (errno));

			return;
		}

		while (1)
		{
			/* "Readdir" gets subsequent entries from "d". */
			dirent* entry = readdir (d);

			if (!entry)
			{
				/* There are no more entries in this directory, so break
				   out of the while loop. */
				break;
			}

			const char* d_name = entry->d_name;

			/* Print the name of the file and directory. */
			// printf ("%s/%s\n", dir_name, d_name);

			if (entry->d_type & DT_DIR)
			{
				/* Check that the directory is not "d" or d's parent. */
				if (strcmp (d_name, "..") != 0 && strcmp (d_name, ".") != 0)
				{
					int path_length;
					char path[PATH_MAX];

					path_length = snprintf (path, PATH_MAX, "%s/%s", directory.c_str (), d_name);
					
					//printf ("%s\n", path);
					
					if (path_length >= PATH_MAX)
					{
						fprintf (stderr, "Path length has got too long.\n");
						return;
					}

					addWatch (path, watcher, false);

					/* Recursively call "list_dir" with the new path. */
					addWatchRecursive (path, watcher);
				}
			}
		}
		/* After going through all the entries, close the directory. */
		if (closedir (d))
		{
			fprintf (stderr, "Could not close '%s': %s\n", directory.c_str (), strerror (errno));
			return;
		}
	}

	//--------
	void FileWatcherLinux::removeWatch (const String& directory)
	{
		WatchMap::iterator iter = mWatches.begin ();
		WatchMap::iterator end = mWatches.end ();
		for (; iter != end; ++iter)
		{
			if (directory == iter->second->mDirName)
			{
				removeWatch (iter->first);
				return;
			}
		}
	}

	//--------
	void FileWatcherLinux::removeWatch (WatchID watchid)
	{
		WatchMap::iterator iter = mWatches.find (watchid);

		if (iter == mWatches.end ())
			return;

		WatchStruct* watch = iter->second;
		mWatches.erase (iter);

		inotify_rm_watch (mFD, watchid);

		delete watch;
		watch = 0;
	}

	//--------
	void FileWatcherLinux::update ()
	{
		FD_SET (mFD, &mDescriptorSet);

		int ret = select (mFD + 1, &mDescriptorSet, NULL, NULL, &mTimeOut);
		if (ret < 0)
		{
			perror ("select");
		}
		else if (FD_ISSET (mFD, &mDescriptorSet))
		{
			ssize_t len, i = 0;
			char action[81 + FILENAME_MAX] = {0};
			char buff[BUFF_SIZE] = {0};

			len = read (mFD, buff, BUFF_SIZE);

			while (i < len)
			{
				struct inotify_event* pevent = (struct inotify_event*)&buff[i];

				WatchStruct* watch = mWatches[pevent->wd];
				handleAction (watch, pevent->name, pevent->mask);
				i += sizeof (struct inotify_event) + pevent->len;
			}
		}
	}

	//--------
	void FileWatcherLinux::handleAction (WatchStruct* watch, const String& filename, unsigned long action)
	{
		if (!watch->mListener)
			return;

		if (IN_CLOSE_WRITE & action)
		{
			watch->mListener->handleFileAction (watch->mWatchID, watch->mDirName, filename, Actions::Modified);
		}
		if (IN_MOVED_TO & action || IN_CREATE & action)
		{
			watch->mListener->handleFileAction (watch->mWatchID, watch->mDirName, filename, Actions::Add);
		}
		if (IN_MOVED_FROM & action || IN_DELETE & action)
		{
			watch->mListener->handleFileAction (watch->mWatchID, watch->mDirName, filename, Actions::Delete);
		}
	}

}; // namespace FW

#endif // FILEWATCHER_PLATFORM_LINUX
