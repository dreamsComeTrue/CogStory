// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Common.h"
#include "Font.h"
#include "Screen.h"

#include <chrono>

namespace aga
{

    //--------------------------------------------------------------------------------------------------

    Point Point::MIN_POINT = {
        -10000000.f,
        -10000000.f,
    };

    Point Point::ZERO_POINT = { 0.f, 0.f };

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    float RadiansToDegrees (float radians) { return radians * 180.0 / M_PI; }

    //--------------------------------------------------------------------------------------------------

    float DegressToRadians (float degrees) { return degrees * M_PI / 180.0; }

    //--------------------------------------------------------------------------------------------------

    float ToPositiveAngle (float degrees)
    {
        degrees = fmod (degrees, 360);

        if (degrees < 0)
        {
            degrees += 360;
        }

        degrees = 360 - degrees;

        return degrees;
    }

    //--------------------------------------------------------------------------------------------------

    bool AreSame (float a, float b, float epsilon)
    {
        return std::fabs (a - b) < epsilon;
    }

    //--------------------------------------------------------------------------------------------------

    bool AreSame (Point a, Point b, Point epsilonPoint) { return AreSame (a.X, b.X, epsilonPoint.X) 
        && AreSame (a.Y, b.Y, epsilonPoint.Y); }

    //--------------------------------------------------------------------------------------------------

    Point RotatePoint (float x, float y, const Point& origin, float angle)
    {
        float s = std::sin (DegressToRadians (angle));
        float c = std::cos (DegressToRadians (angle));

        // translate point back to origin:
        x -= origin.X;
        y -= origin.Y;

        float nx = (x * c) - (y * s);
        float ny = (x * s) + (y * c);

        // translate point back:
        return { nx + origin.X, ny + origin.Y };
    }

    //--------------------------------------------------------------------------------------------------

    float Lerp (float a, float b, float percentage) { return a + percentage * (b - a); }

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_COLOR Lerp (ALLEGRO_COLOR a, ALLEGRO_COLOR b, float percentage)
    {
        ALLEGRO_COLOR result;

        result.a = Lerp (a.a, b.a, percentage);
        result.r = Lerp (a.r, b.r, percentage);
        result.g = Lerp (a.g, b.g, percentage);
        result.b = Lerp (a.b, b.b, percentage);

        return result;
    }

    //--------------------------------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------------------------------

    std::string& LeftTrimString (std::string& str)
    {
        auto it2 = std::find_if (str.begin (), str.end (),
                                 [](char ch) { return !std::isspace<char> (ch, std::locale::classic ()); });
        str.erase (str.begin (), it2);
        return str;
    }

    //--------------------------------------------------------------------------------------------------

    std::string& RightTrimString (std::string& str)
    {
        auto it1 = std::find_if (str.rbegin (), str.rend (),
                                 [](char ch) { return !std::isspace<char> (ch, std::locale::classic ()); });
        str.erase (it1.base (), str.end ());
        return str;
    }

    //--------------------------------------------------------------------------------------------------

    std::string& TrimString (std::string& str) { return LeftTrimString (RightTrimString (str)); }

    //--------------------------------------------------------------------------------------------------

    bool EndsWith (const std::string& str, const std::string& suffix)
    {
        return str.size () >= suffix.size () && 0 == str.compare (str.size () - suffix.size (), suffix.size (), suffix);
    }

    //--------------------------------------------------------------------------------------------------

    bool StartsWith (const std::string& str, const std::string& prefix)
    {
        return str.size () >= prefix.size () && 0 == str.compare (0, prefix.size (), prefix);
    }

    //--------------------------------------------------------------------------------------------------

    float RandZeroToOne () { return rand () / (RAND_MAX + 1.f); }

    //--------------------------------------------------------------------------------------------------

    bool RandBool () { return RandZeroToOne () > 0.5f ? true : false; }

    //--------------------------------------------------------------------------------------------------

    float RandInRange (float min, float max) { return ((float(rand ()) / float(RAND_MAX)) * (max - min)) + min; }

    //--------------------------------------------------------------------------------------------------

    int ToInteger (const std::string& str) { return std::stoi (str); }

    //--------------------------------------------------------------------------------------------------

    std::string GetDirectory (const std::string& fullPath)
    {
        std::string str = fullPath;
        std::replace (str.begin (), str.end (), '\\', '/');
        size_t found = str.find_last_of ("/");

        return str.substr (0, found);
    }

    //--------------------------------------------------------------------------------------------------

    std::string GetBaseName (const std::string& fullPath)
    {
        std::string str = fullPath;
        std::replace (str.begin (), str.end (), '\\', '/');
        size_t found = str.find_last_of ("/");

        // Remove extension if present.
        str = str.substr (found + 1);

        const size_t period_idx = str.rfind ('.');
        if (std::string::npos != period_idx)
        {
            str.erase (period_idx);
        }

        return str;
    }

    //--------------------------------------------------------------------------------------------------

    bool IsFileExists (const std::string& filePath)
    {
        std::ifstream f (filePath.c_str ());
        return f.good ();
    }

    //--------------------------------------------------------------------------------------------------

    std::string GetCurrentDir ()
    {
        char buf[FILENAME_MAX];

#ifdef _MSC_VER
        char* succ = _getcwd (buf, FILENAME_MAX);
#else
        char* succ = getcwd (buf, FILENAME_MAX);
#endif

        if (succ)
        {
            std::string str = std::string (succ);
            std::replace (str.begin (), str.end (), '\\', '/');
            return str;
        }

        return ""; // raise a flag, throw an exception, ...
    }

    //--------------------------------------------------------------------------------------------------

    long GetCurrentTime ()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds> (
                   std::chrono::steady_clock::now ().time_since_epoch ())
            .count ();
    }

    //--------------------------------------------------------------------------------------------------

    void Log (const char* str, ...)
    {
        const char* const zcFormat = str;

        // initialize use of the variable argument array
        va_list vaArgs;
        va_start (vaArgs, str);

        // reliably acquire the size from a copy of
        // the variable argument array
        // and a functionally reliable call
        // to mock the formatting
        va_list vaCopy;
        va_copy (vaCopy, vaArgs);
        const int iLen = std::vsnprintf (NULL, 0, zcFormat, vaCopy);
        va_end (vaCopy);

        // return a formatted string without
        // risking memory mismanagement
        // and without assuming any compiler
        // or platform specific behavior
        std::vector<char> zc (iLen + 1);
        std::vsnprintf (zc.data (), zc.size (), zcFormat, vaArgs);
        va_end (vaArgs);

        std::string result = std::string (zc.data (), zc.size ());

        std::cout << result;

        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        Screen::GetSingleton ()->AddDebugMessage (result, 5000);
    }

    //--------------------------------------------------------------------------------------------------

    void Log (float timeout, ALLEGRO_COLOR color, const char* str, ...)
    {
        const char* const zcFormat = str;

        // initialize use of the variable argument array
        va_list vaArgs;
        va_start (vaArgs, str);

        // reliably acquire the size from a copy of
        // the variable argument array
        // and a functionally reliable call
        // to mock the formatting
        va_list vaCopy;
        va_copy (vaCopy, vaArgs);
        const int iLen = std::vsnprintf (NULL, 0, zcFormat, vaCopy);
        va_end (vaCopy);

        // return a formatted string without
        // risking memory mismanagement
        // and without assuming any compiler
        // or platform specific behavior
        std::vector<char> zc (iLen + 1);
        std::vsnprintf (zc.data (), zc.size (), zcFormat, vaArgs);
        va_end (vaArgs);

        std::string result = std::string (zc.data (), zc.size ());

        std::cout << result;

        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        Screen::GetSingleton ()->AddDebugMessage (result, timeout, color);
    }

    //--------------------------------------------------------------------------------------------------
}
