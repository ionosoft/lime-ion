#ifndef LIME_SDL_FOREIGN_WINDOW_H
#define LIME_SDL_FOREIGN_WINDOW_H


#include "SDLWindow.h"
#include <SDL.h>
#include <app/Application.h>
#include <graphics/ImageBuffer.h>


namespace lime {


	class SDLForeignWindow : public SDLWindow {

		public:

			SDLForeignWindow (Application* application);
			~SDLForeignWindow ();

			virtual void CreateFrom(const void* foreignHandle, int renderFlags);
			virtual void Create(int width, int height, int flags, const char* title) { }

			virtual void Focus () { }
			virtual void Move (int x, int y) { }
			virtual void Resize (int width, int height) { }
			virtual bool SetBorderless (bool borderless){ return false; }
			virtual void SetDisplayMode (DisplayMode* displayMode) { }
			virtual bool SetFullscreen (bool fullscreen) { return false; }
			virtual void SetIcon (ImageBuffer *imageBuffer) { }
			virtual bool SetMaximized (bool maximized) { return false; }
			virtual bool SetMinimized (bool minimized) { return false; }
			virtual bool SetResizable (bool resizable) { return false; }
			virtual const char* SetTitle (const char* title) { return NULL; }

		private:

	};


}


#endif