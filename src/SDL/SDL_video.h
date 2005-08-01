/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@devolution.com
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif

/* Header file for access to the SDL raw framebuffer window */

#ifndef _SDL_video_h
#define _SDL_video_h

#include <stdio.h>

#include "SDL_types.h"
#include "SDL_mutex.h"
#include "SDL_rwops.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* Useful data types */
typedef struct {
	Sint16 x, y;
	Uint16 w, h;
} SDL_Rect;

typedef struct {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 unused;
} SDL_Color;

typedef struct {
	int       ncolors;
	SDL_Color *colors;
} SDL_Palette;

/* Everything in the pixel format structure is read-only */
typedef struct SDL_PixelFormat {
	SDL_Palette *palette;
	Uint8  BitsPerPixel;
	Uint8  BytesPerPixel;
	Uint8  Rloss;
	Uint8  Gloss;
	Uint8  Bloss;
	Uint8  Aloss;
	Uint8  Rshift;
	Uint8  Gshift;
	Uint8  Bshift;
	Uint8  Ashift;
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	Uint32 Amask;

	/* RGB color key information */
	Uint32 colorkey;
	/* Alpha value information (per-surface alpha) */
	Uint8  alpha;
} SDL_PixelFormat;

/* typedef for private surface blitting functions */
struct SDL_Surface;
typedef int (*SDL_blit)(struct SDL_Surface *src, SDL_Rect *srcrect,
			struct SDL_Surface *dst, SDL_Rect *dstrect);

/* This structure should be treated as read-only, except for 'pixels',
   which, if not NULL, contains the raw pixel data for the surface.
*/
typedef struct SDL_Surface {
	Uint32 flags;				/* Read-only */
	SDL_PixelFormat *format;		/* Read-only */
	int w, h;				/* Read-only */
	Uint16 pitch;				/* Read-only */
	void *pixels;				/* Read-write */
	int offset;				/* Private */

	/* Hardware-specific surface info */
	struct private_hwdata *hwdata;

	/* clipping information */
	int clip_minx;				/* Read-only */
	int clip_maxx;				/* Read-only */
	int clip_miny;				/* Read-only */
	int clip_maxy;				/* Read-only */

	/* info for fast blit mapping to other surfaces */
	struct SDL_BlitMap *map;		/* Private */

	/* List of surfaces mapped */
	struct map_list *mapped;		/* Private */

	/* Reference count -- used when freeing surface */
	int refcount;				/* Read-mostly */
} SDL_Surface;

/* These are the currently supported flags for the SDL_surface */
/* Available for SDL_CreateRGBSurface() or SDL_SetVideoMode() */
#define SDL_SWSURFACE	0x00000000	/* Surface is in system memory */
#define SDL_HWSURFACE	0x00000001	/* Surface is in video memory */
/* Available for SDL_SetVideoMode() */
#define SDL_ANYFORMAT	0x10000000	/* Allow any video pixel format */
#define SDL_HWPALETTE	0x00000002	/* Surface has exclusive palette */
#define SDL_DOUBLEBUF	0x40000000	/* Set up double-buffered video mode */
#define SDL_FULLSCREEN	0x80000000	/* Surface is a full screen display */
/* Used internally (read-only) */
#define SDL_HWACCEL	0x00000100	/* Blit uses hardware acceleration */
#define SDL_SRCCOLORKEY	0x00001000	/* Blit uses a source color key */
#define SDL_RLEACCELOK	0x00002000	/* Private flag */
#define SDL_RLEACCEL	0x00004000	/* Colorkey blit is RLE accelerated */
#define SDL_SRCALPHA	0x00010000	/* Blit uses source alpha blending */
#define SDL_SRCCLIPPING	0x00100000	/* Blit uses source clipping */
#define SDL_PREALLOC	0x01000000	/* Surface uses preallocated memory */

/* Evaluates to true if the surface needs to be locked before access */
#define SDL_MUSTLOCK(surface) \
  (surface->offset || ((surface->flags & (SDL_HWSURFACE|SDL_RLEACCEL)) != 0))


/* Useful for determining the video hardware capabilities */
typedef struct {
	Uint32 hw_available :1;	/* Flag: Can you create hardware surfaces? */
	Uint32 wm_available :1;	/* Flag: Can you talk to a window manager? */
	Uint32 UnusedBits1  :6;
	Uint32 UnusedBits2  :1;
	Uint32 blit_hw      :1;	/* Flag: Accelerated blits HW --> HW */
	Uint32 blit_hw_CC   :1;	/* Flag: Accelerated blits with Colorkey */
	Uint32 blit_hw_A    :1;	/* Flag: Accelerated blits with Alpha */
	Uint32 blit_sw      :1;	/* Flag: Accelerated blits SW --> HW */
	Uint32 blit_sw_CC   :1;	/* Flag: Accelerated blits with Colorkey */
	Uint32 blit_sw_A    :1;	/* Flag: Accelerated blits with Alpha */
	Uint32 blit_fill    :1;	/* Flag: Accelerated color fill */
	Uint32 UnusedBits3  :16;
	Uint32 video_mem;	/* The total amount of video memory (in K) */
	SDL_PixelFormat *vfmt;	/* Value: The format of the video surface */
} SDL_VideoInfo;


/* Function prototypes */

/* These functions are used internally, and should not be used unless you
 * have a specific need to specify the video driver you want to use.
 *
 * SDL_VideoInit() initializes the video subsystem -- sets up a connection
 * to the window manager, etc, and determines the current video mode and
 * pixel format, but does not initialize a window or graphics mode.
 * Note that event handling is activated by this routine.
 *
 * If you use both sound and video in your application, you need to call
 * SDL_Init() before opening the sound device, otherwise under Win32 DirectX,
 * you won't be able to set full-screen display modes.
 */
extern DECLSPEC int SDL_VideoInit(const char *driver_name, Uint32 flags);
extern DECLSPEC void SDL_VideoQuit(void);

/* This function fills the given character buffer with the name of the
 * video driver, and returns a pointer to it if the video driver has
 * been initialized.  It returns NULL if no driver has been initialized.
 */
extern DECLSPEC char *SDL_VideoDriverName(char *namebuf, int maxlen);

/*
 * This function returns a pointer to the current display surface.
 * If SDL is doing format conversion on the display surface, this
 * function returns the publicly visible surface, not the real video
 * surface.
 */
extern DECLSPEC SDL_Surface * SDL_GetVideoSurface(void);

/*
 * This function returns a read-only pointer to information about the
 * video hardware.  If this is called before SDL_SetVideoMode(), the 'vfmt'
 * member of the returned structure will contain the pixel format of the
 * "best" video mode.
 */
extern DECLSPEC const SDL_VideoInfo * SDL_GetVideoInfo(void);

/* 
 * Check to see if a particular video mode is supported.
 * It returns 0 if the requested mode is not supported under any bit depth,
 * or returns the bits-per-pixel of the closest available mode with the
 * given width and height.  If this bits-per-pixel is different from the
 * one used when setting the video mode, SDL_SetVideoMode() will succeed,
 * but will emulate the requested bits-per-pixel with a shadow surface.
 *
 * The arguments to SDL_VideoModeOK() are the same ones you would pass to
 * SDL_SetVideoMode()
 */
extern DECLSPEC int SDL_VideoModeOK(int width, int height, int bpp, Uint32 flags);

/*
 * Return a pointer to an array of available screen dimensions for the
 * given format and video flags, sorted largest to smallest.  Returns 
 * NULL if there are no dimensions available for a particular format, 
 * or (SDL_Rect **)-1 if any dimension is okay for the given format.
 *
 * If 'format' is NULL, the mode list will be for the format given 
 * by SDL_GetVideoInfo()->vfmt
 */
extern DECLSPEC SDL_Rect ** SDL_ListModes(SDL_PixelFormat *format, Uint32 flags);

/*
 * Set up a video mode with the specified width, height and bits-per-pixel.
 *
 * If 'bpp' is 0, it is treated as the current display bits per pixel.
 *
 * If SDL_ANYFORMAT is set in 'flags', the SDL library will try to set the
 * requested bits-per-pixel, but will return whatever video pixel format is
 * available.  The default is to emulate the requested pixel format if it
 * is not natively available.
 *
 * If SDL_HWSURFACE is set in 'flags', the video surface will be placed in
 * video memory, if possible, and you may have to call SDL_LockSurface()
 * in order to access the raw framebuffer.  Otherwise, the video surface
 * will be created in system memory.
 *
 * If SDL_HWPALETTE is set in 'flags', the SDL library will guarantee
 * that the colors set by SDL_SetColors() will be the colors you get.
 * Otherwise, in 8-bit mode, SDL_SetColors() may not be able to set all
 * of the colors exactly the way they are requested, and you should look
 * at the video surface structure to determine the actual palette.
 * If SDL cannot guarantee that the colors you request can be set, 
 * i.e. if the colormap is shared, then the video surface may be created
 * under emulation in system memory, overriding the SDL_HWSURFACE flag.
 *
 * If SDL_FULLSCREEN is set in 'flags', the SDL library will try to set
 * a fullscreen video mode.  The default is to create a windowed mode
 * if the current graphics system has a window manager.
 * If the SDL library is able to set a fullscreen video mode, this flag 
 * will be set in the surface that is returned.
 *
 * If SDL_DOUBLEBUF is set in 'flags', the SDL library will try to set up
 * two surfaces in video memory and swap between them when you call 
 * SDL_Flip().  This is usually slower than the normal single-buffering
 * scheme, but prevents "tearing" artifacts caused by modifying video 
 * memory while the monitor is refreshing.  It should only be used by 
 * applications that redraw the entire screen on every update.
 *
 * This function returns the video framebuffer surface, or NULL if it fails.
 */
extern DECLSPEC SDL_Surface *SDL_SetVideoMode
			(int width, int height, int bpp, Uint32 flags);

/*
 * Makes sure the given list of rectangles is updated on the given screen.
 * If 'x', 'y', 'w' and 'h' are all 0, SDL_UpdateRect will update the entire
 * screen.
 * These functions should not be called while 'screen' is locked.
 */
extern DECLSPEC void SDL_UpdateRects
		(SDL_Surface *screen, int numrects, SDL_Rect *rects);
extern DECLSPEC void SDL_UpdateRect
		(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h);

/*
 * On hardware that supports double-buffering, this function sets up a flip
 * and returns.  The hardware will wait for vertical retrace, and then swap
 * video buffers before the next video surface blit or lock will return.
 * On hardware that doesn not support double-buffering, this is equivalent
 * to calling SDL_UpdateRect(screen, 0, 0, 0, 0);
 * The SDL_DOUBLEBUF flag must have been passed to SDL_SetVideoMode() when
 * setting the video mode for this function to perform hardware flipping.
 * This function returns 0 if successful, or -1 if there was an error.
 */
extern DECLSPEC int SDL_Flip(SDL_Surface *screen);

/*
 * Sets a portion of the colormap for the given 8-bit surface.  If 'surface'
 * is not a palettized surface, this function does nothing, returning 0.
 * If all of the colors were set as passed to SDL_SetColors(), it will
 * return 1.  If not all the color entries were set exactly as given,
 * it will return 0, and you should look at the surface palette to
 * determine the actual color palette.
 *
 * When 'surface' is the surface associated with the current display, the
 * display colormap will be updated with the requested colors.  If 
 * SDL_HWPALETTE was set in SDL_SetVideoMode() flags, SDL_SetColors()
 * will always return 1, and the palette is guaranteed to be set the way
 * you desire, even if the window colormap has to be warped or run under
 * emulation.
 */
extern DECLSPEC int SDL_SetColors(SDL_Surface *surface, 
			SDL_Color *colors, int firstcolor, int ncolors);

/*
 * Maps an RGB value to a 32-bit pixel for a given pixel format
 */
extern DECLSPEC Uint32 SDL_MapRGB
			(SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b);

/*
 * Maps a 32-bit pixel into the RGB components for a given pixel format
 */
extern DECLSPEC void SDL_GetRGB(Uint32 pixel,
                         SDL_PixelFormat *fmt, Uint8 *r, Uint8 *g, Uint8 *b);

/*
 * Allocate and free an RGB surface (must be called after SDL_SetVideoMode)
 * If the depth is 4 or 8 bits, an empty palette is allocated for the surface.
 * If the depth is greater than 8 bits, the pixel format is set using the
 * flags '[RGB]mask'.
 * If the function runs out of memory, it will return NULL.
 *
 * The 'flags' tell what kind of surface to create.
 * SDL_SWSURFACE means that the surface should be created in system memory.
 * SDL_HWSURFACE means that the surface should be created in video memory,
 * with the same format as the display surface.  This is useful for surfaces
 * that will not change much, to take advantage of hardware acceleration
 * when being blitted to the display surface.
 * SDL_SRCCOLORKEY means that the surface will be used for colorkey blits
 * and if the hardware supports hardware acceleration of colorkey blits
 * between two surfaces in video memory, to place the surface in video 
 * memory if possible, otherwise it will be placed in system memory.
 * SDL_SRCALPHA means that the surface will be used for alpha blits and 
 * if the hardware supports hardware acceleration of alpha blits between
 * two surfaces in video memory, to place the surface in video memory
 * if possible, otherwise it will be placed in system memory.
 * SDL_SRCCOLORKEY and SDL_SRCALPHA will only be honored if the display 
 * surface is in video memory (the surface returned by SDL_SetVideoMode()
 * has the SDL_HWSURFACE flag set.)
 * If the surface is created in video memory, blits will be _much_ faster,
 * but the surface format must be identical to the video surface format,
 * and the only way to access the pixels member of the surface is to use
 * the SDL_LockSurface() and SDL_UnlockSurface() calls.
 * If the requested surface actually resides in video memory, SDL_HWSURFACE
 * will be set in the flags member of the returned surface.  If for some
 * reason the surface could not be placed in video memory, it will not have
 * the SDL_HWSURFACE flag set, but will be created in system memory and 
 * returned anyway.
 */
#define SDL_AllocSurface    SDL_CreateRGBSurface
extern DECLSPEC SDL_Surface *SDL_CreateRGBSurface
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
extern DECLSPEC SDL_Surface *SDL_CreateRGBSurfaceFrom(void *pixels,
			int width, int height, int depth, int pitch,
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
extern DECLSPEC void SDL_FreeSurface(SDL_Surface *surface);

/*
 * SDL_LockSurface() sets up a surface for directly accessing the pixels.
 * Between calls to SDL_LockSurface()/SDL_UnlockSurface(), you can write
 * to and read from 'surface->pixels', using the pixel format stored in 
 * 'surface->format'.  Once you are done accessing the surface, you should 
 * use SDL_UnlockSurface() to release it.
 *
 * Not all surfaces require locking.  If SDL_MUSTLOCK(surface) evaluates
 * to 0, then you can read and write to the surface at any time, and the
 * pixel format of the surface will not change.  In particular, if the
 * SDL_HWSURFACE flag is not given when calling SDL_SetVideoMode(), you
 * will not need to lock the display surface before accessing it.
 * 
 * No operating system or library calls should be made between lock/unlock
 * pairs, as critical system locks may be held during this time.
 *
 * SDL_LockSurface() returns 0, or -1 if the surface couldn't be locked.
 */
extern DECLSPEC int SDL_LockSurface(SDL_Surface *surface);
extern DECLSPEC void SDL_UnlockSurface(SDL_Surface *surface);

/*
 * Load a surface from a seekable SDL data source (memory or file.)
 * If 'freesrc' is non-zero, the source will be closed after being read.
 * Returns the new surface, or NULL if there was an error.
 * The new surface should be freed with SDL_FreeSurface().
 */
extern DECLSPEC SDL_Surface * SDL_LoadBMP_RW(SDL_RWops *src, int freesrc);

/* Convenience macro -- load a surface from a file */
#define SDL_LoadBMP(file)	SDL_LoadBMP_RW(SDL_RWFromFile(file, "rb"), 1)

/*
 * Save a surface to a seekable SDL data source (memory or file.)
 * If 'freedst' is non-zero, the source will be closed after being written.
 * Returns 0 if successful or -1 if there was an error.
 */
extern DECLSPEC int SDL_SaveBMP_RW
		(SDL_Surface *surface, SDL_RWops *dst, int freedst);

/* Convenience macro -- save a surface to a file */
#define SDL_SaveBMP(surface, file) \
		SDL_SaveBMP_RW(surface, SDL_RWFromFile(file, "wb"), 1)

/*
 * Sets the color key (transparent pixel) in a blittable surface.
 * If 'flag' is SDL_SRCCOLORKEY (optionally OR'd with SDL_RLEACCEL), 
 * 'key' will be the transparent pixel in the source image of a blit.
 * If 'flag' is 0, this function clears any current color key.
 * This function returns 0, or -1 if there was an error.
 */
extern DECLSPEC int SDL_SetColorKey
			(SDL_Surface *surface, Uint32 flag, Uint32 key);

/*
 * If 'alpha' is non-zero, this function sets the alpha value for the 
 * entire surface, as opposed to using the alpha component of each pixel.
 * This value measures the range of transparency of the surface, 0 being
 * completely opaque to 255 being completely transparent.
 *
 * If 'flag' is 0, alpha blending is disabled for the surface.
 * If 'flag' is SDL_SRCALPHA, alpha blending is enabled for the surface.
 * If 'flag' is (SDL_SRCALPHA|SDL_MULACCEL), alpha blending is enabled
 * with alpha pre-multiplication enabled.
 *
 * Alpha blend premultiplication is an optimization that cuts the actual
 * alpha blend operation in half.  There are a couple of drawbacks however.
 * It slightly reduces the accuracy of the blending operation.
 * You must lock and unlock the surface to access it, and each lock and
 * unlock operation reduces the color accuracy of the unchanged pixels.
 * The best use of this optimization is on surfaces that will be loaded
 * once and then be static most of the time they are in use.
 */
extern DECLSPEC int SDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha);

/*
 * Sets the clipping rectangle for the source surface in a blit.
 * If the destination rectangle falls outside this rectangle, the
 * source surface will be clipped so the blit doesn't write outside
 * the clipping rectangle.
 * If the clip rectangle is { 0, 0, 0, 0 }, clipping will be disabled.
 *
 * Note that blits are automatically clipped to the edges of the source
 * and destination surfaces.
 */
extern DECLSPEC void SDL_SetClipping(SDL_Surface *surface,
				int top, int left, int bottom, int right);

/*
 * Creates a new surface of the specified format, and then copies and maps 
 * the given surface to it so the blit of the converted surface will be as 
 * fast as possible.  If this function fails, it returns NULL.
 *
 * The 'flags' parameter is passed to SDL_CreateRGBSurface() and has those 
 * semantics.
 *
 * This function is used internally by SDL_DisplayFormat().
 */
extern DECLSPEC SDL_Surface *SDL_ConvertSurface
			(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags);

/*
 * This performs a fast blit from the source surface to the destination
 * surface.  It assumes that the source and destination rectangles are
 * the same size.  If either 'srcrect' or 'dstrect' are NULL, the entire
 * surface (src or dst) is copied.  The final blit rectangles are saved
 * in 'srcrect' and 'dstrect' after all clipping is performed.
 * If the blit is successful, it returns 0, otherwise it returns -1.
 *
 * The blit function should not be called on a locked surface.
 *
 * If either of the surfaces were in video memory, and the blit returns -2,
 * the video memory was lost, so it should be reloaded with artwork and 
 * re-blitted:
	while ( SDL_BlitSurface(image, imgrect, screen, dstrect) == -2 ) {
		while ( SDL_LockSurface(image) < 0 )
			Sleep(10);
		-- Write image pixels to image->pixels --
		SDL_UnlockSurface(image);
	}
 * This happens under DirectX 5.0 when the system switches away from your
 * fullscreen application.  The lock will also fail until you have access
 * to the video memory again.
 */
/* You should call SDL_BlitSurface() unless you know exactly how SDL
   blitting works internally and how to use the other blit functions.
*/
#define SDL_BlitSurface SDL_UpperBlit

/* This is the public blit function, SDL_BlitSurface(), and it performs
   rectangle validation and clipping before passing it to SDL_LowerBlit()
*/
extern DECLSPEC int SDL_UpperBlit
			(SDL_Surface *src, SDL_Rect *srcrect,
			 SDL_Surface *dst, SDL_Rect *dstrect);
/* This is a semi-private blit function and it performs low-level surface
   blitting only.
*/
extern DECLSPEC int SDL_LowerBlit
			(SDL_Surface *src, SDL_Rect *srcrect,
			 SDL_Surface *dst, SDL_Rect *dstrect);

/*
 * This function performs a fast fill of the given rectangle with 'color'
 * If 'dstrect' is NULL, the whole surface will be filled with 'color'
 * The color should be a pixel of the format used by the surface, and 
 * can be generated by the SDL_MapRGB() function.
 * This function returns 0 on success, or -1 on error.
 */
extern DECLSPEC int SDL_FillRect
		(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);

/* 
 * This function takes a surface and copies it to a new surface of the
 * pixel format and colors of the video framebuffer, suitable for fast
 * blitting onto the display surface.  It calls SDL_ConvertSurface()
 *
 * If you want to take advantage of hardware colorkey or alpha blit
 * acceleration, you should set the colorkey and alpha value before
 * calling this function.
 *
 * If the conversion fails or runs out of memory, it returns NULL
 */
extern DECLSPEC SDL_Surface * SDL_DisplayFormat(SDL_Surface *surface);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* These functions allow interaction with the window manager, if any.        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Sets/Gets the title and icon text of the display window
 */
extern DECLSPEC void SDL_WM_SetCaption(const char *title, const char *icon);
extern DECLSPEC void SDL_WM_GetCaption(char **title, char **icon);

/*
 * Sets the icon for the display window.
 * This function must be called before the first call to SDL_SetVideoMode().
 * It takes an icon surface, and a mask in MSB format.
 * If 'mask' is NULL, the entire icon surface will be used as the icon.
 */
extern DECLSPEC void SDL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask);

/*
 * This function iconifies the window, and returns 1 if it succeeded.
 * If the function succeeds, it generates an SDL_APPACTIVE loss event.
 * This function is a noop and returns 0 in non-windowed environments.
 */
extern DECLSPEC int SDL_WM_IconifyWindow(void);

/*
 * Toggle fullscreen mode without changing the contents of the screen.
 * If the display surface does not require locking before accessing
 * the pixel information, then the memory pointers will not change.
 *
 * If this function was able to toggle fullscreen mode (change from 
 * running in a window to fullscreen, or vice-versa), it will return 1.
 * If it is not implemented, or fails, it returns 0.
 *
 * The next call to SDL_SetVideoMode() will set the mode fullscreen
 * attribute based on the flags parameter - if SDL_FULLSCREEN is not
 * set, then the display will be windowed by default where supported.
 *
 * This is currently only implemented in the X11 video driver.
 */
extern DECLSPEC int SDL_WM_ToggleFullScreen(SDL_Surface *surface);

/*
 * This function allows you to set and query the input grab state of
 * the application.  It returns the new input grab state.
 */
typedef enum {
	SDL_GRAB_QUERY = -1,
	SDL_GRAB_OFF = 0,
	SDL_GRAB_ON = 1,
	SDL_GRAB_FULLSCREEN	/* Used internally */
} SDL_GrabMode;
/*
 * Grabbing means that the mouse is confined to the application window,
 * and nearly all keyboard input is passed directly to the application,
 * and not interpreted by a window manager, if any.
 *
 * The input grab state is reset after each call to SDL_SetVideoMode().
 * The input is grabbed by default in fullscreen mode, and ungrabbed in
 * windowed mode.  If you want to set input grab to a particular value,
 * you should set it after each call to SDL_SetVideoMode().
 */
extern DECLSPEC SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
};
#endif
#include "close_code.h"

#endif /* _SDL_video_h */
