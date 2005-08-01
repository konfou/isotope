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

/* Include file for SDL event handling */

#ifndef _SDL_events_h
#define _SDL_events_h

#include "SDL_types.h"
#include "SDL_active.h"
#include "SDL_keyboard.h"
#include "SDL_mouse.h"
#include "SDL_quit.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* Event enumerations */
enum { SDL_NOEVENT,			/* Unused (do not remove) */
       SDL_ACTIVEEVENT,			/* Application loses/gains visibility */
       SDL_KEYDOWN,			/* Keys pressed */
       SDL_KEYUP,			/* Keys released */
       SDL_MOUSEMOTION,			/* Mouse moved */
       SDL_MOUSEBUTTONDOWN,		/* Mouse button pressed */
       SDL_MOUSEBUTTONUP,		/* Mouse button released */
       SDL_JOYMOTION,			/* Joystick axis motion */
       SDL_JOYBUTTONDOWN,		/* Joystick button pressed */
       SDL_JOYBUTTONUP,			/* Joystick button released */
       SDL_QUIT,			/* User-requested quit */
       SDL_SYSWMEVENT,			/* System specific event */
       /* Events SDL_USEREVENT through SDL_MAXEVENTS-1 are for your use */
       SDL_USEREVENT = 24,
       /* This last event is only for bounding internal arrays
	  It is the number of bits in the event mask datatype -- Uint32
        */
       SDL_NUMEVENTS = 32
};

/* Predefined event masks */
#define SDL_EVENTMASK(X)	(1<<(X))
enum {
	SDL_ACTIVEEVENTMASK	= SDL_EVENTMASK(SDL_ACTIVEEVENT),
	SDL_KEYDOWNMASK		= SDL_EVENTMASK(SDL_KEYDOWN),
	SDL_KEYUPMASK		= SDL_EVENTMASK(SDL_KEYUP),
	SDL_MOUSEMOTIONMASK	= SDL_EVENTMASK(SDL_MOUSEMOTION),
	SDL_MOUSEBUTTONDOWNMASK	= SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN),
	SDL_MOUSEBUTTONUPMASK	= SDL_EVENTMASK(SDL_MOUSEBUTTONUP),
	SDL_JOYMOTIONMASK	= SDL_EVENTMASK(SDL_JOYMOTION),
	SDL_JOYBUTTONDOWNMASK	= SDL_EVENTMASK(SDL_JOYBUTTONDOWN),
	SDL_JOYBUTTONUPMASK	= SDL_EVENTMASK(SDL_JOYBUTTONUP),
	SDL_QUITMASK		= SDL_EVENTMASK(SDL_QUIT),
	SDL_SYSWMEVENTMASK	= SDL_EVENTMASK(SDL_SYSWMEVENT)
};
#define SDL_ALLEVENTS		0xFFFFFFFF

/* Application visibility event structure */
typedef struct {
	Uint8 type;
	Uint8 gain;
	Uint8 state;
} SDL_ActiveEvent;

/* Keyboard event structure */
typedef struct {
	Uint8 type;
	Uint8 state;
	SDL_keysym keysym;
} SDL_KeyboardEvent;

/* Mouse motion event structure */
typedef struct {
	Uint8 type;
	Uint8 state;
	Uint16 x, y;
	Sint16 xrel;
	Sint16 yrel;
} SDL_MouseMotionEvent;

/* Mouse button event structure */
typedef struct {
	Uint8 type;
	Uint8 state;
	Uint8 button;
	Uint16 x, y;
} SDL_MouseButtonEvent;

typedef struct {
	Uint8 type;
} SDL_QuitEvent;

/* If you want to use this event, you should include SDL_syswm.h */
struct SDL_SysWMmsg;
typedef struct SDL_SysWMmsg SDL_SysWMmsg;
typedef struct {
	Uint8 type;
	SDL_SysWMmsg *msg;
} SDL_SysWMEvent;

/* General event structure */
typedef union {
	Uint8 type;
	SDL_ActiveEvent active;
	SDL_KeyboardEvent key;
	SDL_MouseMotionEvent motion;
	SDL_MouseButtonEvent button;
	SDL_QuitEvent quit;
	SDL_SysWMEvent syswm;
} SDL_Event;


/* Function prototypes */

/* Pumps the event loop, gathering events from the input devices.
   This function updates the event queue and internal input device state.
   This should only be run in the thread that sets the video mode.
*/
extern DECLSPEC void SDL_PumpEvents(void);

/* Checks the event queue for messages and optionally returns them.
   If 'action' is SDL_ADDEVENT, up to 'numevents' events will be added to
   the back of the event queue.
   If 'action' is SDL_PEEKEVENT, up to 'numevents' events at the front
   of the event queue, matching 'mask', will be returned and will not
   be removed from the queue.
   If 'action' is SDL_GETEVENT, up to 'numevents' events at the front 
   of the event queue, matching 'mask', will be returned and will be
   removed from the queue.
   This function returns the number of events actually stored, or -1
   if there was an error.  This function is thread-safe.
*/
typedef enum {
	SDL_ADDEVENT,
	SDL_PEEKEVENT,
	SDL_GETEVENT
} SDL_eventaction;
/* */
extern DECLSPEC int SDL_PeepEvents(SDL_Event *events, int numevents,
				SDL_eventaction action, Uint32 mask);

/* Polls for currently pending events, and returns 1 if there are any pending
   events, or 0 if there are none available.  If 'event' is not NULL, the next
   event is removed from the queue and stored in that area.
 */
extern DECLSPEC int SDL_PollEvent(SDL_Event *event);

/* Waits indefinitely for the next available event, returning 1, or 0 if there
   was an error while waiting for events.  If 'event' is not NULL, the next
   event is removed from the queue and stored in that area.
 */
extern DECLSPEC int SDL_WaitEvent(SDL_Event *event);

/*
  This function sets up a filter to process all events before they
  change internal state and are posted to the internal event queue.

  The filter is protypted as:
*/
typedef int (*SDL_EventFilter)(const SDL_Event *event);
/*
  If the filter returns 1, then the event will be added to the internal queue.
  If it returns 0, then the event will be dropped from the queue, but the 
  internal state will still be updated.  This allows selective filtering of
  dynamically arriving events.

  WARNING:  Be very careful of what you do in the event filter function, as 
            it may run in a different thread!

  There is one caveat when dealing with the SDL_QUITEVENT event type.  The
  event filter is only called when the window manager desires to close the
  application window.  If the event filter returns 1, then the window will
  be closed, otherwise the window will remain open if possible.
  If the quit event is generated by an interrupt signal, it will bypass the
  internal queue and be delivered to the application at the next event poll.
*/
extern DECLSPEC void SDL_SetEventFilter(SDL_EventFilter filter);

/*
  This function allows you to set the state of processing certain events.
  If 'state' is set to SDL_IGNORE, that event will be automatically dropped
  from the event queue and will not event be filtered.
  If 'state' is set to SDL_ENABLE, that event will be processed normally.
  If 'state' is set to SDL_QUERY, SDL_EventState() will return the 
  current processing state of the specified event.
*/
#define SDL_QUERY	-1
#define SDL_IGNORE	 0
#define SDL_ENABLE	 1
extern DECLSPEC Uint8 SDL_EventState(Uint8 type, int state);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
};
#endif
#include "close_code.h"

#endif /* _SDL_events_h */
