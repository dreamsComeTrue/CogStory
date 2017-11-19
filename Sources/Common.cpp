// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Common.h"

namespace aga
{
    float RadiansToRadians (float radians) { return radians * 180.0 / M_PI; }

    float DegressToRadians (float degrees) { return degrees * M_PI / 180.0; }

    float ToPositiveAngle (float degrees)
    {
        degrees = fmod (degrees, 360);

        if (degrees < 0)
        {
            degrees += 360;
        }

        return degrees;
    }

    //--------------------------------------------------------------------------------------------------

    bool AreSame (float a, float b)
    {
        float epsilon = 1.0E-8;
        return std::fabs (a - b) < epsilon;
    }

    std::vector<std::string> SplitString (const std::string& s, char seperator)
    {
        std::vector<std::string> output;

        std::string::size_type prev_pos = 0, pos = 0;

        while ((pos = s.find (seperator, pos)) != std::string::npos)
        {
            std::string substring (s.substr (prev_pos, pos - prev_pos));

            output.push_back (substring);

            prev_pos = ++pos;
        }

        output.push_back (s.substr (prev_pos, pos - prev_pos)); // Last word

        return output;
    }

    std::string& LeftTrimString (std::string& str)
    {
        auto it2 = std::find_if (str.begin (), str.end (), [](char ch) { return !std::isspace<char> (ch, std::locale::classic ()); });
        str.erase (str.begin (), it2);
        return str;
    }

    std::string& RightTrimString (std::string& str)
    {
        auto it1 = std::find_if (str.rbegin (), str.rend (), [](char ch) { return !std::isspace<char> (ch, std::locale::classic ()); });
        str.erase (it1.base (), str.end ());
        return str;
    }

    std::string& TrimString (std::string& str) { return LeftTrimString (RightTrimString (str)); }

    std::string GetDirectory (const std::string& fullPath)
    {
        std::string str = fullPath;
        std::replace (str.begin (), str.end (), '\\', '/');
        unsigned found = str.find_last_of ("/");

        return str.substr (0, found);
    }

    std::string GetBaseName (const std::string& fullPath)
    {
        std::string str = fullPath;
        std::replace (str.begin (), str.end (), '\\', '/');
        unsigned found = str.find_last_of ("/");

        // Remove extension if present.
        str = str.substr (found + 1);

        const size_t period_idx = str.rfind ('.');
        if (std::string::npos != period_idx)
        {
            str.erase (period_idx);
        }

        return str;
    }

    bool IsFileExists (const std::string& filePath)
    {
        std::ifstream f (filePath.c_str ());
        return f.good ();
    }

    std::string GetCurrentDir ()
    {
        char buf[FILENAME_MAX];
        char* succ = getcwd (buf, FILENAME_MAX);

        if (succ)
        {
            std::string str = std::string (succ);
            std::replace (str.begin (), str.end (), '\\', '/');
            return str;
        }

        return ""; // raise a flag, throw an exception, ...
    }
}
