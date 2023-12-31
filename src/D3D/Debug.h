/*===========================================================================*/
/*                                                                           */
/* Mesa-3.0 DirectX 6 Driver                                       Build 0.7 */
/*                                                                           */
/* By Leigh McRae                                                            */
/*                                                                           */
/* http://www.altsoftware.com/                                               */
/*                                                                           */
/* Copyright (c) 1999-1998  alt.software inc.  All Rights Reserved           */
/*===========================================================================*/
#ifndef _DEBUG_H
#define _DEBUG_H

/*===========================================================================*/
/* Includes.                                                                 */
/*===========================================================================*/
#include <stdio.h>
#include <string.h>
#include <ddraw.h>
#include <d3d.h>
#include "D3DMesa.h"
/*===========================================================================*/
/* Magic numbers.                                                            */
/*===========================================================================*/
/*===========================================================================*/
/* Macros defines.                                                           */
/*===========================================================================*/
#define	DBG_FUNC		0x00000001
#define	DBG_STATES         	0x00000002
#define	DBG_LOGFILE        	0x00000004

#define	DBG_CNTX_INFO		0x00000010
#define	DBG_CNTX_WARN		0x00000020
#define	DBG_CNTX_PROFILE	0x00000040
#define	DBG_CNTX_ERROR		0x00000080
#define	DBG_CNTX_ALL		0x000000F0

#define	DBG_PRIM_INFO		0x00000100
#define	DBG_PRIM_WARN		0x00000200
#define	DBG_PRIM_PROFILE	0x00000400
#define	DBG_PRIM_ERROR		0x00000800
#define	DBG_PRIM_ALL		0x00000F00

#define	DBG_TXT_INFO		0x00001000
#define	DBG_TXT_WARN		0x00002000
#define	DBG_TXT_PROFILE		0x00004000
#define	DBG_TXT_ERROR		0x00008000
#define	DBG_TXT_ALL		0x0000F000

#define	DBG_ALL_INFO		0x11111110
#define	DBG_ALL_WARN		0x22222220
#define	DBG_ALL_PROFILE		0x44444440
#define	DBG_ALL_ERROR		0x88888880
#define	DBG_ALL			0xFFFFFFFF

#ifdef D3D_DEBUG
#	define	DPF(arg)		DebugPrint arg 
#	define  DEBUG_BREAK		_asm int 3
#	define 	PROFILE_START 		 QueryPerformanceCounter( &g_ProfStart );
#	define 	PROFILE_STOP		 QueryPerformanceCounter( &g_ProfFinish ); \
                                         g_dwProfTime = g_ProfFinish.u.LowPart - g_ProfStart.u.LowPart;
#else
#	define	DPF(arg)
#	define  DEBUG_BREAK
#	define	PROFILE_START
#	define 	PROFILE_STOP
#endif
/*===========================================================================*/
/* Type defines.                                                             */
/*===========================================================================*/
/*===========================================================================*/
/* Function prototypes.                                                      */
/*===========================================================================*/
extern void ReadDBGEnv( void );
extern void _cdecl DebugPrint( int mask, char *pszFormat, ... );
extern void DebugPixelFormat( char *pszSurfaceName, DDPIXELFORMAT *pddpf );
/*===========================================================================*/
/* Global variables.                                                         */
/*===========================================================================*/
extern DWORD		g_DBGMask,
                        g_dwProfTime;
extern LARGE_INTEGER 	g_ProfStart,
			g_ProfFinish;

#endif



