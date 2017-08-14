// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COMMON_H__
#define __COMMON_H__

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

#include <angelscript.h>
#include <scriptbuilder/scriptbuilder.h>
#include <scriptstdstring/scriptstdstring.h>

#include <Gwork/Controls.h>
#include <Gwork/Input/Allegro5.h>
#include <Gwork/Platform.h>
#include <Gwork/Renderers/Allegro5.h>
#include <Gwork/Skins/TexturedBase.h>

#include <cmath>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "addons/tweeny/tweeny.h"

#include "Lifecycle.h"
#include "Point.h"
#include "Rect.h"
#include "Resources.h"

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
    const ALLEGRO_COLOR COLOR_BLACK{ 0.0f, 0.0f, 0.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_WHITE{ 1.0f, 1.0f, 1.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_RED{ 1.0f, 0.0f, 0.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_GREEN{ 0.0f, 1.0f, 0.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_BLUE{ 0.0f, 0.0f, 1.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_YELLOW{ 1.0f, 1.0f, 0.0f, 1.0f };
    const ALLEGRO_COLOR COLOR_GRAY{ 0.3f, 0.3f, 0.3f, 1.0f };

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
}

#endif //   __COMMON_H__
