// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COMMON_H__
#define __COMMON_H__

#define UI_EDITOR 0

#include "Lifecycle.h"
#include "Point.h"
#include "Polygon.h"
#include "Rect.h"
#include "Resources.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

extern "C" {
#include "addons/nine-patch/nine_patch.h"
}

#include <angelscript.h>
#include <scriptbuilder/scriptbuilder.h>
#include <scriptstdstring/scriptstdstring.h>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <cmath>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "addons/tweeny/tweeny.h"

#define SAFE_DELETE(x)                                                                                                 \
    {                                                                                                                  \
        if (x != nullptr)                                                                                              \
        {                                                                                                              \
            delete x;                                                                                                  \
            x = nullptr;                                                                                               \
        }                                                                                                              \
    }

#define ARRAY_SIZE(_ARR) ((int)(sizeof (_ARR) / sizeof (*_ARR)))

#define EDITOR_ENABLED

namespace aga
{
    const ALLEGRO_COLOR COLOR_BLACK{ 0.0f, 0.0f, 0.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_WHITE{ 1.0f, 1.0f, 1.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_RED{ 1.0f, 0.0f, 0.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_GREEN{ 0.0f, 1.0f, 0.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_BLUE{ 0.0f, 0.0f, 1.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_LIGHTBLUE{ 51 / 255.f, 153 / 255.f, 255 / 255.f };
    const ALLEGRO_COLOR COLOR_YELLOW{ 1.0f, 1.0f, 0.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_GRAY{ 0.3f, 0.3f, 0.3f, 1.0f };
    const ALLEGRO_COLOR COLOR_VIOLET{ 138 / 255.f, 43 / 255.f, 226 / 255.f, 1.0f };
    const ALLEGRO_COLOR COLOR_INDIGO{ 75 / 255.f, 0 / 255.f, 130 / 255.f, 1.0f };
    const ALLEGRO_COLOR COLOR_DARKBLUE{ 55 / 255.f, 51 / 255.f, 153 / 255.f };
    const ALLEGRO_COLOR COLOR_ORANGE{ 255 / 255.f, 165 / 255.f, 0 / 255.f };
    const ALLEGRO_COLOR COLOR_PINK{ 191 / 255.f, 63 / 255.f, 191 / 255.f };

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
        auto it2 = std::find_if (
            str.begin (), str.end (), [](char ch) { return !std::isspace<char> (ch, std::locale::classic ()); });
        str.erase (str.begin (), it2);
        return str;
    }

    static std::string& RightTrimString (std::string& str)
    {
        auto it1 = std::find_if (
            str.rbegin (), str.rend (), [](char ch) { return !std::isspace<char> (ch, std::locale::classic ()); });
        str.erase (it1.base (), str.end ());
        return str;
    }

    static std::string& TrimString (std::string& str) { return LeftTrimString (RightTrimString (str)); }

    template <typename T> static std::string ToString (T t)
    {
        std::stringstream strStream;
        strStream << t;

        return strStream.str ();
    }

    static std::string GetDirectory (const std::string& fullPath)
    {
        boost::filesystem::path p{ fullPath };
        return p.parent_path ().string ();
    }

    static std::string GetBaseName (const std::string& fullPath)
    {
        boost::filesystem::path p{ fullPath };
        return p.stem ().string ();
    }
}

#endif //   __COMMON_H__
