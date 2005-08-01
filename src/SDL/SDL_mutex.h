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

#ifndef _SDL_mutex_h
#define _SDL_mutex_h

/* Functions to manipulate the semaphore locks 

	These are independent of the other SDL routines.
*/

#include "SDL_main.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* The SDL mutex structure, defined in SDL_mutex.c */
struct SDL_mutex;
typedef struct SDL_mutex SDL_mutex;

/* Create a mutex, initialized unlocked */
extern DECLSPEC SDL_mutex * SDL_CreateMutex(void);

/* Lock the mutex  (Returns 0, or -1 on error) */
#define SDL_LockMutex(m)	SDL_mutexP(m)
extern DECLSPEC int SDL_mutexP(SDL_mutex *mutex);

/* Unlock the mutex  (Returns 0, or -1 on error) */
#define SDL_UnlockMutex(m)	SDL_mutexV(m)
extern DECLSPEC int SDL_mutexV(SDL_mutex *mutex);

/* Destroy a mutex */
extern DECLSPEC void SDL_DestroyMutex(SDL_mutex *mutex);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
};
#endif
#include "close_code.h"

#endif /* _SDL_mutex_h */
