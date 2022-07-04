#include "SDLForeignWindow.h"
#include "SDLApplication.h"
#include "../../graphics/opengl/OpenGL.h"
#include "../../graphics/opengl/OpenGLBindings.h"

namespace lime {


	static int maxAttempts = 5;


	SDLForeignWindow::SDLForeignWindow (Application* application) : SDLWindow (application) { }

	void SDLForeignWindow::CreateFrom (const void* foreignHandle, int renderFlags) {

		//printf ("Any lingering SDL errors pre-window creation? %s.\n", SDL_GetError ());

		renderFlags &= (WINDOW_FLAG_HARDWARE
					  | WINDOW_FLAG_VSYNC);

		this->flags = renderFlags;

		#if defined (HX_WINDOWS) && defined (NATIVE_TOOLKIT_SDL_ANGLE) && !defined (HX_WINRT)
		OSVERSIONINFOEXW osvi = { sizeof (osvi), 0, 0, 0, 0, {0}, 0, 0 };
		DWORDLONG const dwlConditionMask = VerSetConditionMask (VerSetConditionMask (VerSetConditionMask (0, VER_MAJORVERSION, VER_GREATER_EQUAL), VER_MINORVERSION, VER_GREATER_EQUAL), VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
		osvi.dwMajorVersion = HIBYTE (_WIN32_WINNT_VISTA);
		osvi.dwMinorVersion = LOBYTE (_WIN32_WINNT_VISTA);
		osvi.wServicePackMajor = 0;

		if (VerifyVersionInfoW (&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) == FALSE) {

			flags &= ~WINDOW_FLAG_HARDWARE;

		}
		#endif

		if (renderFlags & WINDOW_FLAG_HARDWARE) {
			// TODO: Set state for SDL to enable SDL_WINDOW_OPENGL on the foreign window
		}

		#ifndef EMSCRIPTEN
		SDL_SetHint (SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "0");
		SDL_SetHint (SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
		SDL_SetHint (SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
		SDL_SetHint (SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
		#endif

		sdlWindow = SDL_CreateWindowFrom (foreignHandle);

		// If SDL_CreateWindowFrom fails to return a window, keep trying
		if (!sdlWindow) {

			int tries = 0;
			while (tries < maxAttempts) {
				sdlWindow = SDL_CreateWindowFrom (foreignHandle);

				if (sdlWindow) break;

				// Initialize the video subsystem in case it isn't already initialized, and try again
				SDL_Init (SDL_INIT_VIDEO|SDL_INIT_EVENTS);
				
				tries++;
			}

		}

		if (!sdlWindow) {

			printf ("Could not create SDL window: %s.\n", SDL_GetError ());
			return;

		}

		int sdlRendererFlags = 0;

		if (renderFlags & WINDOW_FLAG_HARDWARE) {

			if (0 == SDL_GL_LoadLibrary (NULL)) {

				context = SDL_GL_CreateContext (sdlWindow);

			}

			if (NULL != context) {

				sdlRendererFlags |= SDL_RENDERER_ACCELERATED;

				// if (window->flags & WINDOW_FLAG_VSYNC) {

				// 	sdlRendererFlags |= SDL_RENDERER_PRESENTVSYNC;

				// }

				// sdlRenderer = SDL_CreateRenderer (sdlWindow, -1, sdlRendererFlags);

				// if (sdlRenderer) {

				// 	context = SDL_GL_GetCurrentContext ();

				// }

				if (context && SDL_GL_MakeCurrent (sdlWindow, context) == 0) {

					if (renderFlags & WINDOW_FLAG_VSYNC) {

						SDL_GL_SetSwapInterval (1);

					} else {

						SDL_GL_SetSwapInterval (0);

					}

					OpenGLBindings::Init ();

					#ifndef LIME_GLES

					int version = 0;
					glGetIntegerv (GL_MAJOR_VERSION, &version);

					if (version == 0) {

						float versionScan = 0;
						sscanf ((const char*)glGetString (GL_VERSION), "%f", &versionScan);
						version = versionScan;

					}

					if (version < 2 && !strstr ((const char*)glGetString (GL_VERSION), "OpenGL ES")) {

						SDL_GL_DeleteContext (context);
						context = 0;

					}

					#elif defined(IPHONE) || defined(APPLETV)

					// SDL_SysWMinfo windowInfo;
					// SDL_GetWindowWMInfo (SDLForeignWindow, &windowInfo);
					// OpenGLBindings::defaultFramebuffer = windowInfo.info.uikit.framebuffer;
					// OpenGLBindings::defaultRenderbuffer = windowInfo.info.uikit.colorbuffer;
					glGetIntegerv (GL_FRAMEBUFFER_BINDING, &OpenGLBindings::defaultFramebuffer);
					glGetIntegerv (GL_RENDERBUFFER_BINDING, &OpenGLBindings::defaultRenderbuffer);

					#endif

				} else {

					SDL_GL_DeleteContext (context);
					context = NULL;

				}

			}
		}

		if (!context) {

			sdlRendererFlags &= ~SDL_RENDERER_ACCELERATED;
			sdlRendererFlags &= ~SDL_RENDERER_PRESENTVSYNC;

			sdlRendererFlags |= SDL_RENDERER_SOFTWARE;

			sdlRenderer = SDL_CreateRenderer (sdlWindow, -1, sdlRendererFlags);

		}

		if (context || sdlRenderer) {

			((SDLApplication*)currentApplication)->RegisterWindow (this);

		} else {

			printf("Could not create SDL renderer: %s.\n", SDL_GetError ());

		}
		
		//printf ("Any lingering SDL errors post-window creation? %s.\n", SDL_GetError ());

	}


	SDLForeignWindow::~SDLForeignWindow () {

		if (sdlWindow) {

			SDL_DestroyWindow (sdlWindow);
			sdlWindow = 0;

		}

		if (sdlRenderer) {

			SDL_DestroyRenderer (sdlRenderer);

		} else if (context) {

			SDL_GL_DeleteContext (context);

		}

	}


	Window* CreateWindowFrom (Application* application, const void* foreignHandle, int renderFlags) {

		SDLForeignWindow* window = new SDLForeignWindow (application);
		window->CreateFrom (foreignHandle, renderFlags);
		return window;

	}
}