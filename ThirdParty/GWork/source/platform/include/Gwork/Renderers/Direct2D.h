/*
 *  Gwork
 *  Copyright (c) 2010 Facepunch Studios
 *  Copyright (c) 2013-17 Nick Trout
 *  See license in Gwork.h
 */

#ifndef GWK_RENDERERS_DIRECT2D_H
#define GWK_RENDERERS_DIRECT2D_H

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <Gwork/BaseRender.h>

namespace Gwk
{
    namespace Renderer
    {
        class GWK_EXPORT Direct2D : public Gwk::Renderer::Base
        {
        public:

            Direct2D();
            Direct2D(ID2D1RenderTarget* device, IDWriteFactory* dWriteFactory,
                     IWICImagingFactory* wICFactory);
            ~Direct2D();

            void Begin() override;
            void End() override;
            virtual void Release();

            void SetDrawColor(Gwk::Color color) override;

            void DrawFilledRect(Gwk::Rect rect) override;

            void LoadFont(Gwk::Font* font) override;
            void FreeFont(Gwk::Font* font) override;
            void RenderText(Gwk::Font* font, Gwk::Point pos, const Gwk::String& text) override;
            Gwk::Point MeasureText(Gwk::Font* font, const Gwk::String& text) override;

            virtual void DeviceLost();
            virtual void DeviceAcquired(ID2D1RenderTarget* rT);

            void StartClip();
            void EndClip();

            void DrawTexturedRect(Gwk::Texture* texture, Gwk::Rect targetRect, float u1 = 0.0f,
                                  float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f) override;
            void        LoadTexture(Gwk::Texture* texture) override;
            void        FreeTexture(Gwk::Texture* texture) override;
            Gwk::Color  PixelColor(Gwk::Texture* texture, unsigned int x, unsigned int y,
                                   const Gwk::Color& col_default) override;

            void DrawLinedRect(Gwk::Rect rect) override;
            void DrawShavedCornerRect(Gwk::Rect rect, bool bSlight = false) override;

        public:

            bool InitializeContext(Gwk::WindowProvider* window);
            bool ShutdownContext(Gwk::WindowProvider* window);
            bool PresentContext(Gwk::WindowProvider* window);
            bool ResizedContext(Gwk::WindowProvider* window, int w, int h);
            bool BeginContext(Gwk::WindowProvider* window);
            bool EndContext(Gwk::WindowProvider* window);

        private:

            bool InternalCreateDeviceResources();
            void InternalReleaseDeviceResources();

            ID2D1Factory*   m_d2DFactory;
            HWND m_hWND;

        private:

            bool InternalLoadTexture(Gwk::Texture* texture);
            bool InternalLoadFont(Gwk::Font* font);

            void InternalFreeFont(Gwk::Font* font, bool bRemove = true);
            void InternalFreeTexture(Gwk::Texture* texture, bool bRemove = true);

        private:

            IDWriteFactory*     m_dWriteFactory;
            IWICImagingFactory* m_wICFactory;
            ID2D1RenderTarget*  m_rT;

            ID2D1SolidColorBrush* m_solidColorBrush;

            D2D1::ColorF m_color;

            Gwk::Texture::List m_textureList;
            Gwk::Font::List m_fontList;
        };

    }
}
#endif // ifndef GWK_RENDERERS_DIRECT2D_H
