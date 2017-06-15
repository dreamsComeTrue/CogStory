// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COMMON_H__
#define __COMMON_H__

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "Lifecycle.h"
#include "Point.h"
#include "Rect.h"
#include "Resources.h"

#define SAFE_DELETE(x)    \
    {                     \
        if (x != nullptr) \
        {                 \
            delete x;     \
            x = nullptr;  \
        }                 \
    }

namespace aga
{
    static bool AreSame (double a, double b)
    {
        double epsilon = 1.0E-8;
        return std::fabs (a - b) < epsilon;
    }
}

#endif //   __COMMON_H__
