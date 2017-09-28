// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __FONT_H__
#define __FONT_H__

#include "Common.h"

namespace aga
{
    class Screen;

    const std::string FONT_NAME_MAIN = "FONT_MAIN";

    // Texture wrapper class
    class LTexture
    {
    public:
        LTexture (SDL_Renderer* renderer);
        ~LTexture ();

        // Loads image at specified path
        bool LoadFromFile (std::string path);

        // Creates image from font string
        bool LoadFromRenderedText (std::string textureText, TTF_Font* font, SDL_Color textColor);

        // Deallocates texture
        void Free ();

        // Set color modulation
        void SetColor (Uint8 red, Uint8 green, Uint8 blue);

        // Set blending
        void SetBlendMode (SDL_BlendMode blending);

        // Set alpha modulation
        void SetAlpha (Uint8 alpha);

        // Renders texture at given point
        void Render (int x,
                     int y,
                     SDL_Rect* clip = NULL,
                     double angle = 0.0,
                     SDL_Point* center = NULL,
                     SDL_RendererFlip flip = SDL_FLIP_NONE);

        // Gets image dimensions
        int GetWidth ();
        int GetHeight ();

    private:
        SDL_Renderer* m_Renderer;

        // The actual hardware texture
        SDL_Texture* m_Texture;

        // Image dimensions
        int m_Width;
        int m_Height;
    };

    class Font : public Lifecycle
    {
    public:
        Font (Screen* screen);
        virtual ~Font ();
        bool Initialize ();
        bool Destroy ();

        void DrawText (const std::string& fontName, SDL_Color color, int x, int y, const std::string& text);
        Point GetTextDimensions (const std::string& fontName, const std::string& text);
        unsigned GetFontAscent (const std::string& fontName);

    private:
        Screen* m_Screen;
        std::map<std::string, TTF_Font*> m_Fonts;
    };
}

#endif //   __FONT_H__
