// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Font.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    LTexture::LTexture (SDL_Renderer* renderer)
      : m_Renderer (renderer)
      , m_Texture (nullptr)
      , m_Width (0)
      , m_Height (0)
    {
    }

    //--------------------------------------------------------------------------------------------------

    LTexture::~LTexture () { Free (); }

    //--------------------------------------------------------------------------------------------------

    bool LTexture::LoadFromFile (std::string path)
    {
        // Get rid of preexisting texture
        Free ();

        // The final texture
        SDL_Texture* newTexture = NULL;

        // Load image at specified path
        SDL_Surface* loadedSurface = IMG_Load (path.c_str ());

        if (loadedSurface == NULL)
        {
            printf ("Unable to load image %s! SDL_image Error: %s\n", path.c_str (), IMG_GetError ());
        }
        else
        {
            // Color key image
            SDL_SetColorKey (loadedSurface, SDL_TRUE, SDL_MapRGB (loadedSurface->format, 0, 0xFF, 0xFF));

            // Create texture from surface pixels
            newTexture = SDL_CreateTextureFromSurface (m_Renderer, loadedSurface);

            if (newTexture == NULL)
            {
                printf ("Unable to create texture from %s! SDL Error: %s\n", path.c_str (), SDL_GetError ());
            }
            else
            {
                // Get image dimensions
                m_Width = loadedSurface->w;
                m_Height = loadedSurface->h;
            }

            // Get rid of old loaded surface
            SDL_FreeSurface (loadedSurface);
        }

        m_Texture = newTexture;

        return m_Texture != NULL;
    }

    //--------------------------------------------------------------------------------------------------

    bool LTexture::LoadFromRenderedText (std::string textureText, TTF_Font* font, SDL_Color textColor)
    {
        // Get rid of preexisting texture
        Free ();

        // Render text surface
        SDL_Surface* textSurface = TTF_RenderText_Solid (font, textureText.c_str (), textColor);
        if (textSurface == NULL)
        {
            printf ("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError ());
        }
        else
        {
            // Create texture from surface pixels
            m_Texture = SDL_CreateTextureFromSurface (m_Renderer, textSurface);

            if (m_Texture == NULL)
            {
                printf ("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError ());
            }
            else
            {
                // Get image dimensions
                m_Width = textSurface->w;
                m_Height = textSurface->h;
            }

            // Get rid of old surface
            SDL_FreeSurface (textSurface);
        }

        return m_Texture != NULL;
    }

    //--------------------------------------------------------------------------------------------------

    void LTexture::Free ()
    {
        if (m_Texture != NULL)
        {
            SDL_DestroyTexture (m_Texture);

            m_Texture = NULL;
            m_Width = 0;
            m_Height = 0;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void LTexture::SetColor (Uint8 red, Uint8 green, Uint8 blue)
    {
        // Modulate texture rgb
        SDL_SetTextureColorMod (m_Texture, red, green, blue);
    }

    //--------------------------------------------------------------------------------------------------

    void LTexture::SetBlendMode (SDL_BlendMode blending)
    {
        // Set blending function
        SDL_SetTextureBlendMode (m_Texture, blending);
    }

    //--------------------------------------------------------------------------------------------------

    void LTexture::SetAlpha (Uint8 alpha)
    {
        // Modulate texture alpha
        SDL_SetTextureAlphaMod (m_Texture, alpha);
    }

    //--------------------------------------------------------------------------------------------------

    void LTexture::Render (int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
    {
        // Set rendering space and render to screen
        SDL_Rect renderQuad = { x, y, m_Width, m_Height };

        // Set clip rendering dimensions
        if (clip != NULL)
        {
            renderQuad.w = clip->w;
            renderQuad.h = clip->h;
        }

        // Render to screen
        SDL_RenderCopyEx (m_Renderer, m_Texture, clip, &renderQuad, angle, center, flip);
    }

    //--------------------------------------------------------------------------------------------------

    int LTexture::GetWidth () { return m_Width; }

    //--------------------------------------------------------------------------------------------------

    int LTexture::GetHeight () { return m_Height; }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Font::Font (Screen* screen)
      : m_Screen (screen)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Font::~Font ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Font::Initialize ()
    {
        Lifecycle::Initialize ();

        TTF_Font* mainFont = TTF_OpenFont (GetResourcePath (FONT_MAIN).c_str (), 28);

        if (!mainFont)
        {
            SDL_Log ("Could not load %s.\n", GetResourcePath (FONT_MAIN).c_str ());
            SDL_Log ("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError ());
            return false;
        }

        m_Fonts.insert (std::make_pair (FONT_NAME_MAIN, mainFont));

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Font::Destroy ()
    {
        for (std::map<std::string, TTF_Font*>::iterator it = m_Fonts.begin (); it != m_Fonts.end (); ++it)
        {
            TTF_CloseFont (it->second);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Font::DrawText (const std::string& fontName, SDL_Color color, int x, int y, const std::string& text)
    {
        TTF_Font* font = m_Fonts[fontName];

        LTexture fontTexture (m_Screen->GetRenderer ());
        if (!fontTexture.LoadFromRenderedText (text, font, color))
        {
            SDL_Log ("Failed to render text texture!\n");
            return;
        }

        fontTexture.Render (x, y);
        fontTexture.Free ();
    }

    //--------------------------------------------------------------------------------------------------

    Point Font::GetTextDimensions (const std::string& fontName, const std::string& text)
    {
        //        int x, y, width, height;
        //        ALLEGRO_FONT* font = m_Fonts[fontName];

        //        al_get_text_dimensions (font, text.c_str (), &x, &y, &width, &height);

        //        return Point{ width, height };
        return Point{ 0, 0 };
    }

    //--------------------------------------------------------------------------------------------------

    unsigned Font::GetFontAscent (const std::string& fontName)
    {
        // ALLEGRO_FONT* font = m_Fonts[fontName];

        //  return al_get_font_ascent (font);
        return 0;
    }

    //--------------------------------------------------------------------------------------------------
}
