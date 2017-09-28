// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COMMON_H__
#define __COMMON_H__

#define UI_EDITOR 0

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <angelscript.h>
#include <scriptbuilder/scriptbuilder.h>
#include <scriptstdstring/scriptstdstring.h>

#include <cmath>
#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "addons/tweeny/tweeny.h"

#include "Lifecycle.h"
#include "Point.h"
#include "Rect.h"
#include "Resources.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#define SAFE_DELETE(x)                                                                                                                     \
    {                                                                                                                                      \
        if (x != nullptr)                                                                                                                  \
        {                                                                                                                                  \
            delete x;                                                                                                                      \
            x = nullptr;                                                                                                                   \
        }                                                                                                                                  \
    }

namespace aga
{
    const SDL_Color COLOR_BLACK{ 0, 0, 0, 255 };
    const SDL_Color COLOR_WHITE{ 255, 255, 255, 255 };
    const SDL_Color COLOR_RED{ 255, 0, 0, 255 };
    const SDL_Color COLOR_GREEN{ 0, 255, 0, 255 };
    const SDL_Color COLOR_BLUE{ 0, 0, 255, 255 };
    const SDL_Color COLOR_YELLOW{ 255, 255, 0, 255 };
    const SDL_Color COLOR_GRAY{ 100, 100, 100, 255 };

    static float DegressToRadians (float degrees) { return degrees * M_PI / 180.0; }

    static bool AreSame (double a, double b)
    {
        double epsilon = 1.0E-8;
        return std::fabs (a - b) < epsilon;
    }

    static std::vector<std::string> SplitString (const std::string& s, char seperator)
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

    static std::string& LeftTrimString (std::string& str)
    {
        auto it2 = std::find_if (str.begin (), str.end (), [](char ch) { return !std::isspace<char> (ch, std::locale::classic ()); });
        str.erase (str.begin (), it2);
        return str;
    }

    static std::string& RightTrimString (std::string& str)
    {
        auto it1 = std::find_if (str.rbegin (), str.rend (), [](char ch) { return !std::isspace<char> (ch, std::locale::classic ()); });
        str.erase (it1.base (), str.end ());
        return str;
    }

    static std::string& TrimString (std::string& str) { return LeftTrimString (RightTrimString (str)); }

    template<typename T>
    static std::string ToString (T t)
    {
        std::stringstream strStream;
        strStream << t;

        return strStream.str ();
    }

    static std::string GetDirectory (const std::string& fullPath)
    {
        std::experimental::filesystem::path p{ fullPath };
        return p.parent_path ().string ();
    }

    static std::string GetBaseName (const std::string& fullPath)
    {
        std::experimental::filesystem::path p{ fullPath };
        return p.stem ().string ();
    }
}

#endif //   __COMMON_H__
