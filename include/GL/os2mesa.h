/* $Id: wmesa.h,v 3.0 1998/02/20 05:06:59 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.0
 * Copyright (C) 1995-1998  Brian Paul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


/*
 * $Log: wmesa.h,v $
 * Revision 3.0  1998/02/20 05:06:59  brianp
 * initial rev
 *
 */


/*
 * OS2 driver by: Leigh McRae (lmcrae@cgocable.net)
 *
 * Septermber 14, 1998
 *
 */


#ifndef OS2MESA_H
#define OS2MESA_H


#ifdef __cplusplus
extern "C" {
#endif


#include "gl\gl.h"


/* This is the OS2 context 'handle': */
typedef struct OS2_context *OS2_context;


/*
 * Create a new OS2Context for rendering into a window.  You must
 * have already created the window of correct visual type and with an
 * appropriate colormap.
 *
 * Input:
 *         hWnd     - Window handle
 *         Pal      - Palette to use
 *         rgb_flag - GL_TRUE = RGB mode,
 *                    GL_FALSE = color index mode
 *         db_flag  - GL_TRUE = double-buffered,
 *                    GL_FALSE = single buffered
 *
 * Note: Indexed mode requires double buffering under Windows.
 *
 * Return:  a OS2_context or NULL if error.
 */
extern OS2_context OS2CreateContext( HWND hWnd, HPALETTE *pPal, GLboolean rgb_flag,GLboolean db_flag );


/* Destroy a rendering context as returned by OS2CreateContext()  */
// extern void OS2DestroyContext( OS2Context ctx );
extern void OS2DestroyContext( void );


/* Make the specified context the current one. */
extern void OS2MakeCurrent( OS2_context ctx );


/* Return a handle to the current context. */
extern OS2_context OS2GetCurrentContext( void );


/* Swap the front and back buffers for the current context.  No action */
/* taken if the context is not double buffered.                        */
extern void OS2SwapBuffers(void);


/* In indexed color mode we need to know when the palette changes.  */
extern void OS2PaletteChange(HPALETTE Pal);

extern void OS2Move(void);



#ifdef __cplusplus
}
#endif


#endif

