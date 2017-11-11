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

#include <experimental/optional>
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

    float DegressToRadians (float degrees); 

    bool AreSame (double a, double b);

    std::vector<std::string> SplitString (const std::string& s, char seperator);

    std::string& LeftTrimString (std::string& str);

    std::string& RightTrimString (std::string& str);

    std::string& TrimString (std::string& str); 

    template <typename T>  std::string ToString (T t)
    {
        std::stringstream strStream;
        strStream << t;

        return strStream.str ();
    }

    std::string GetDirectory (const std::string& fullPath);

    std::string GetBaseName (const std::string& fullPath);

    bool IsFileExists (const std::string& filePath);

    std::string GetCurrentDir ();
}

#endif //   __COMMON_H__
