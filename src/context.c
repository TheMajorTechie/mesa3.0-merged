/* $Id: context.c,v 3.21 1998/08/23 22:19:30 brianp Exp $ */

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
 */


/*
 * $Log: context.c,v $
 * Revision 3.21  1998/08/23 22:19:30  brianp
 * added DoViewportMapping to context struct
 *
 * Revision 3.20  1998/08/21 01:49:46  brianp
 * initialize vertex array state
 *
 * Revision 3.19  1998/07/17 03:23:32  brianp
 * added Pixel.ScaleOrBiasRGBA field
 *
 * Revision 3.18  1998/06/21 02:00:55  brianp
 * cleaned up clip interpolation function code
 *
 * Revision 3.17  1998/06/19 03:13:10  brianp
 * evaluator state wasn't fully initialized
 *
 * Revision 3.16  1998/06/07 22:18:52  brianp
 * implemented GL_EXT_multitexture extension
 *
 * Revision 3.15  1998/04/01 02:58:28  brianp
 * updated for v0.24 of 3Dfx/Glide driver
 *
 * Revision 3.14  1998/03/27 03:30:36  brianp
 * fixed G++ warnings
 *
 * Revision 3.13  1998/03/22 16:42:22  brianp
 * added 8-bit CI->RGBA pixel mapping tables
 *
 * Revision 3.12  1998/03/15 17:55:54  brianp
 * added FogMode to context struct
 *
 * Revision 3.11  1998/03/05 02:49:48  brianp
 * added an assertion and added better comments in gl_update_state()
 *
 * Revision 3.10  1998/03/01 20:17:03  brianp
 * removed dead code
 *
 * Revision 3.9  1998/02/20 04:50:09  brianp
 * implemented GL_SGIS_multitexture
 *
 * Revision 3.8  1998/02/13 04:38:05  brianp
 * optimized blending functions called via BlendFunc function pointer
 *
 * Revision 3.7  1998/02/13 03:23:04  brianp
 * AlphaRef is now a GLubyte
 *
 * Revision 3.6  1998/02/13 03:17:02  brianp
 * added basic stereo support
 *
 * Revision 3.5  1998/02/08 20:20:34  brianp
 * ColorMask is now GLubyte[4] instead of GLuint
 *
 * Revision 3.4  1998/02/05 01:10:25  brianp
 * added John Stone's thread modifications
 *
 * Revision 3.3  1998/02/02 03:09:34  brianp
 * added GL_LIGHT_MODEL_COLOR_CONTROL (separate specular color interpolation)
 *
 * Revision 3.2  1998/02/01 16:37:19  brianp
 * added GL_EXT_rescale_normal extension
 *
 * Revision 3.1  1998/01/31 23:56:55  brianp
 * removed Driver.ClearDepthBuffer function
 *
 * Revision 3.0  1998/01/31 20:49:07  brianp
 * initial rev
 *
 */


/*
 * If multi-threading is enabled (-DTHREADS) then each thread has it's
 * own rendering context.  A thread obtains the pointer to its GLcontext
 * with the gl_get_thread_context() function.  Otherwise, the global
 * pointer, CC, points to the current context used by all threads in
 * the address space.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "accum.h"
#include "alphabuf.h"
#include "clip.h"
#include "context.h"
#include "depth.h"
#include "eval.h"
#include "hash.h"
#include "light.h"
#include "lines.h"
#include "dlist.h"
#include "macros.h"
#include "mmath.h"
#include "pb.h"
#include "points.h"
#include "pointers.h"
#include "quads.h"
#include "stencil.h"
#include "triangle.h"
#include "teximage.h"
#include "texobj.h"
#include "texstate.h"
#include "types.h"
#include "vb.h"
#include "vbfill.h"
#endif



/**********************************************************************/
/*****                  Context and Thread management             *****/
/**********************************************************************/


#ifdef THREADS

#include "mthreads.h" /* Mesa platform independent threads interface */

static MesaTSD mesa_ctx_tsd;

static void mesa_ctx_thread_init() {
  MesaInitTSD(&mesa_ctx_tsd);
}

GLcontext *gl_get_thread_context( void ) {
  return (GLcontext *) MesaGetTSD(&mesa_ctx_tsd);
}

static void set_thread_context( GLcontext *ctx ) {
  MesaSetTSD(&mesa_ctx_tsd, ctx, mesa_ctx_thread_init);
}


#else

/* One Current Context pointer for all threads in the address space */
GLcontext *CC = NULL;

#endif /*THREADS*/




/**********************************************************************/
/*****                   Profiling functions                      *****/
/**********************************************************************/

#ifdef PROFILE

#include <sys/times.h>
#include <sys/param.h>


/*
 * Return system time in seconds.
 * NOTE:  this implementation may not be very portable!
 */
GLdouble gl_time( void )
{
   static GLdouble prev_time = 0.0;
   static GLdouble time;
   struct tms tm;
   clock_t clk;

   clk = times(&tm);

#ifdef CLK_TCK
   time = (double)clk / (double)CLK_TCK;
#else
   time = (double)clk / (double)HZ;
#endif

   if (time>prev_time) {
      prev_time = time;
      return time;
   }
   else {
      return prev_time;
   }
}



/*
 * Reset the timing/profiling counters
 */
static void init_timings( GLcontext *ctx )
{
   ctx->BeginEndCount = 0;
   ctx->BeginEndTime = 0.0;
   ctx->VertexCount = 0;
   ctx->VertexTime = 0.0;
   ctx->PointCount = 0;
   ctx->PointTime = 0.0;
   ctx->LineCount = 0;
   ctx->LineTime = 0.0;
   ctx->PolygonCount = 0;
   ctx->PolygonTime = 0.0;
   ctx->ClearCount = 0;
   ctx->ClearTime = 0.0;
   ctx->SwapCount = 0;
   ctx->SwapTime = 0.0;
}



/*
 * Print the accumulated timing/profiling data.
 */
static void print_timings( GLcontext *ctx )
{
   GLdouble beginendrate;
   GLdouble vertexrate;
   GLdouble pointrate;
   GLdouble linerate;
   GLdouble polygonrate;
   GLdouble overhead;
   GLdouble clearrate;
   GLdouble swaprate;
   GLdouble avgvertices;

   if (ctx->BeginEndTime>0.0) {
      beginendrate = ctx->BeginEndCount / ctx->BeginEndTime;
   }
   else {
      beginendrate = 0.0;
   }
   if (ctx->VertexTime>0.0) {
      vertexrate = ctx->VertexCount / ctx->VertexTime;
   }
   else {
      vertexrate = 0.0;
   }
   if (ctx->PointTime>0.0) {
      pointrate = ctx->PointCount / ctx->PointTime;
   }
   else {
      pointrate = 0.0;
   }
   if (ctx->LineTime>0.0) {
      linerate = ctx->LineCount / ctx->LineTime;
   }
   else {
      linerate = 0.0;
   }
   if (ctx->PolygonTime>0.0) {
      polygonrate = ctx->PolygonCount / ctx->PolygonTime;
   }
   else {
      polygonrate = 0.0;
   }
   if (ctx->ClearTime>0.0) {
      clearrate = ctx->ClearCount / ctx->ClearTime;
   }
   else {
      clearrate = 0.0;
   }
   if (ctx->SwapTime>0.0) {
      swaprate = ctx->SwapCount / ctx->SwapTime;
   }
   else {
      swaprate = 0.0;
   }

   if (ctx->BeginEndCount>0) {
      avgvertices = (GLdouble) ctx->VertexCount / (GLdouble) ctx->BeginEndCount;
   }
   else {
      avgvertices = 0.0;
   }

   overhead = ctx->BeginEndTime - ctx->VertexTime - ctx->PointTime
              - ctx->LineTime - ctx->PolygonTime;


   printf("                          Count   Time (s)    Rate (/s) \n");
   printf("--------------------------------------------------------\n");
   printf("glBegin/glEnd           %7d  %8.3f   %10.3f\n",
          ctx->BeginEndCount, ctx->BeginEndTime, beginendrate);
   printf("  vertexes transformed  %7d  %8.3f   %10.3f\n",
          ctx->VertexCount, ctx->VertexTime, vertexrate );
   printf("  points rasterized     %7d  %8.3f   %10.3f\n",
          ctx->PointCount, ctx->PointTime, pointrate );
   printf("  lines rasterized      %7d  %8.3f   %10.3f\n",
          ctx->LineCount, ctx->LineTime, linerate );
   printf("  polygons rasterized   %7d  %8.3f   %10.3f\n",
          ctx->PolygonCount, ctx->PolygonTime, polygonrate );
   printf("  overhead                       %8.3f\n", overhead );
   printf("glClear                 %7d  %8.3f   %10.3f\n",
          ctx->ClearCount, ctx->ClearTime, clearrate );
   printf("SwapBuffers             %7d  %8.3f   %10.3f\n",
          ctx->SwapCount, ctx->SwapTime, swaprate );
   printf("\n");

   printf("Average number of vertices per begin/end: %8.3f\n", avgvertices );
}
#endif





/**********************************************************************/
/*****       Context allocation, initialization, destroying       *****/
/**********************************************************************/


/*
 * Allocate and initialize a shared context state structure.
 */
static struct gl_shared_state *alloc_shared_state( void )
{
   GLuint i;
   struct gl_shared_state *ss;
   GLboolean outOfMemory;

   ss = (struct gl_shared_state*) calloc( 1, sizeof(struct gl_shared_state) );
   if (!ss)
      return NULL;

   ss->DisplayList = NewHashTable();

   ss->TexObjects = NewHashTable();

   /* Default Texture objects */
   outOfMemory = GL_FALSE;
   for (i=0;i<MAX_TEX_SETS;i++) {
      ss->Default1D[i] = gl_alloc_texture_object(ss, 0, 1);
      ss->Default2D[i] = gl_alloc_texture_object(ss, 0, 2);
      ss->Default3D[i] = gl_alloc_texture_object(ss, 0, 3);
      if (!ss->Default1D[i] || !ss->Default2D[i] || !ss->Default3D[i]) {
         outOfMemory = GL_TRUE;
         break;
      }
   }

   if (!ss->DisplayList || !ss->TexObjects || outOfMemory) {
      /* Ran out of memory at some point.  Free everything and return NULL */
      if (!ss->DisplayList)
         DeleteHashTable(ss->DisplayList);
      if (!ss->TexObjects)
         DeleteHashTable(ss->TexObjects);
      for (i=0;i<MAX_TEX_SETS;i++) {
         if (!ss->Default1D[i])
            gl_free_texture_object(ss, ss->Default1D[i]);
         if (!ss->Default2D[i])
            gl_free_texture_object(ss, ss->Default2D[i]);
         if (!ss->Default3D[i])
            gl_free_texture_object(ss, ss->Default3D[i]);
      }
      free(ss);
      return NULL;
   }
   else {
      return ss;
   }
}


/*
 * Deallocate a shared state context and all children structures.
 */
static void free_shared_state( GLcontext *ctx, struct gl_shared_state *ss )
{
   /* Free display lists */
   while (1) {
      GLuint list = HashFirstEntry(ss->DisplayList);
      if (list) {
         gl_destroy_list(ctx, list);
      }
      else {
         break;
      }
   }
   DeleteHashTable(ss->DisplayList);

   /* Free texture objects */
   while (ss->TexObjectList)
   {
      /* this function removes from linked list too! */
      gl_free_texture_object(ss, ss->TexObjectList);
   }
   DeleteHashTable(ss->TexObjects);

   free(ss);
}




/*
 * Initialize the nth light.  Note that the defaults for light 0 are
 * different than the other lights.
 */
static void init_light( struct gl_light *l, GLuint n )
{
   ASSIGN_4V( l->Ambient, 0.0, 0.0, 0.0, 1.0 );
   if (n==0) {
      ASSIGN_4V( l->Diffuse, 1.0, 1.0, 1.0, 1.0 );
      ASSIGN_4V( l->Specular, 1.0, 1.0, 1.0, 1.0 );
   }
   else {
      ASSIGN_4V( l->Diffuse, 0.0, 0.0, 0.0, 1.0 );
      ASSIGN_4V( l->Specular, 0.0, 0.0, 0.0, 1.0 );
   }
   ASSIGN_4V( l->Position, 0.0, 0.0, 1.0, 0.0 );
   ASSIGN_3V( l->Direction, 0.0, 0.0, -1.0 );
   l->SpotExponent = 0.0;
   gl_compute_spot_exp_table( l );
   l->SpotCutoff = 180.0;
   l->CosCutoff = -1.0;
   l->ConstantAttenuation = 1.0;
   l->LinearAttenuation = 0.0;
   l->QuadraticAttenuation = 0.0;
   l->Enabled = GL_FALSE;
}



static void init_lightmodel( struct gl_lightmodel *lm )
{
   ASSIGN_4V( lm->Ambient, 0.2, 0.2, 0.2, 1.0 );
   lm->LocalViewer = GL_FALSE;
   lm->TwoSide = GL_FALSE;
   lm->ColorControl = GL_SINGLE_COLOR;
}


static void init_material( struct gl_material *m )
{
   ASSIGN_4V( m->Ambient,  0.2, 0.2, 0.2, 1.0 );
   ASSIGN_4V( m->Diffuse,  0.8, 0.8, 0.8, 1.0 );
   ASSIGN_4V( m->Specular, 0.0, 0.0, 0.0, 1.0 );
   ASSIGN_4V( m->Emission, 0.0, 0.0, 0.0, 1.0 );
   m->Shininess = 0.0;
   m->AmbientIndex = 0;
   m->DiffuseIndex = 1;
   m->SpecularIndex = 1;
   gl_compute_material_shine_table( m );
}


static void init_texture_set( GLcontext *ctx, GLuint setNum )
{
   struct gl_texture_set *s = &ctx->Texture.Set[setNum];

   s->EnvMode = GL_MODULATE;
   ASSIGN_4V( s->EnvColor, 0.0, 0.0, 0.0, 0.0 );
   s->TexGenEnabled = 0;
   s->GenModeS = GL_EYE_LINEAR;
   s->GenModeT = GL_EYE_LINEAR;
   s->GenModeR = GL_EYE_LINEAR;
   s->GenModeQ = GL_EYE_LINEAR;
   /* Yes, these plane coefficients are correct! */
   ASSIGN_4V( s->ObjectPlaneS, 1.0, 0.0, 0.0, 0.0 );
   ASSIGN_4V( s->ObjectPlaneT, 0.0, 1.0, 0.0, 0.0 );
   ASSIGN_4V( s->ObjectPlaneR, 0.0, 0.0, 0.0, 0.0 );
   ASSIGN_4V( s->ObjectPlaneQ, 0.0, 0.0, 0.0, 0.0 );
   ASSIGN_4V( s->EyePlaneS, 1.0, 0.0, 0.0, 0.0 );
   ASSIGN_4V( s->EyePlaneT, 0.0, 1.0, 0.0, 0.0 );
   ASSIGN_4V( s->EyePlaneR, 0.0, 0.0, 0.0, 0.0 );
   ASSIGN_4V( s->EyePlaneQ, 0.0, 0.0, 0.0, 0.0 );

   s->Current1D = ctx->Shared->Default1D[setNum];
   s->Current2D = ctx->Shared->Default2D[setNum];
   s->Current3D = ctx->Shared->Default3D[setNum];

   s->TexCoordSet = setNum;   /* GL_EXT_multitexture */
}


/* Initialize a 1-D evaluator map */
static void init_1d_map( struct gl_1d_map *map, int n, const float *initial )
{
   map->Order = 1;
   map->u1 = 0.0;
   map->u2 = 1.0;
   map->Points = malloc(n * sizeof(GLfloat));
   if (map->Points) {
      GLint i;
      for (i=0;i<n;i++)
         map->Points[i] = initial[i];
   }
   map->Retain = GL_FALSE;
}


/* Initialize a 2-D evaluator map */
static void init_2d_map( struct gl_2d_map *map, int n, const float *initial )
{
   map->Uorder = 1;
   map->Vorder = 1;
   map->u1 = 0.0;
   map->u2 = 1.0;
   map->v1 = 0.0;
   map->v2 = 1.0;
   map->Points = malloc(n * sizeof(GLfloat));
   if (map->Points) {
      GLint i;
      for (i=0;i<n;i++)
         map->Points[i] = initial[i];
   }
   map->Retain = GL_FALSE;
}



/*
 * Initialize a gl_context structure to default values.
 */
static void initialize_context( GLcontext *ctx )
{
   static GLfloat identity[16] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
   };
   GLuint i;

   if (ctx) {
      /* Modelview matrix */
      ctx->NewModelViewMatrix = GL_FALSE;
      ctx->ModelViewMatrixType = MATRIX_IDENTITY;
      MEMCPY( ctx->ModelViewMatrix, identity, 16*sizeof(GLfloat) );
      MEMCPY( ctx->ModelViewInv, identity, 16*sizeof(GLfloat) );
      ctx->ModelViewStackDepth = 0;

      /* Projection matrix */
      ctx->NewProjectionMatrix = GL_FALSE;
      ctx->ProjectionMatrixType = MATRIX_IDENTITY;
      MEMCPY( ctx->ProjectionMatrix, identity, 16*sizeof(GLfloat) );
      ctx->ProjectionStackDepth = 0;
      ctx->NearFarStack[0][0] = 1.0; /* These values seem weird by make */
      ctx->NearFarStack[0][1] = 0.0; /* sense mathematically. */

      /* Texture matrix */
      for (i=0; i<MAX_TEX_SETS; i++) {
         ctx->NewTextureMatrix = GL_FALSE;
         ctx->TextureMatrixType[i] = MATRIX_IDENTITY;
         MEMCPY( ctx->TextureMatrix[i], identity, 16*sizeof(GLfloat) );
         ctx->TextureStackDepth[i] = 0;
      }

      /* Accumulate buffer group */
      ASSIGN_4V( ctx->Accum.ClearColor, 0.0, 0.0, 0.0, 0.0 );

      /* Color buffer group */
      ctx->Color.IndexMask = 0xffffffff;
      ctx->Color.ColorMask[0] = 0xff;
      ctx->Color.ColorMask[1] = 0xff;
      ctx->Color.ColorMask[2] = 0xff;
      ctx->Color.ColorMask[3] = 0xff;
      ctx->Color.SWmasking = GL_FALSE;
      ctx->Color.ClearIndex = 0;
      ASSIGN_4V( ctx->Color.ClearColor, 0.0, 0.0, 0.0, 0.0 );
      ctx->Color.DrawBuffer = GL_FRONT;
      ctx->Color.AlphaEnabled = GL_FALSE;
      ctx->Color.AlphaFunc = GL_ALWAYS;
      ctx->Color.AlphaRef = 0;
      ctx->Color.BlendEnabled = GL_FALSE;
      ctx->Color.BlendSrc = GL_ONE;
      ctx->Color.BlendDst = GL_ZERO;
      ctx->Color.BlendEquation = GL_FUNC_ADD_EXT;
      ctx->Color.BlendFunc = NULL;  /* this pointer set only when needed */
      ASSIGN_4V( ctx->Color.BlendColor, 0.0, 0.0, 0.0, 0.0 );
      ctx->Color.IndexLogicOpEnabled = GL_FALSE;
      ctx->Color.ColorLogicOpEnabled = GL_FALSE;
      ctx->Color.SWLogicOpEnabled = GL_FALSE;
      ctx->Color.LogicOp = GL_COPY;
      ctx->Color.DitherFlag = GL_TRUE;

      /* Current group */
      ASSIGN_4V( ctx->Current.ByteColor, 255, 255, 255, 255 );
      ctx->Current.Index = 1;
      ASSIGN_3V( ctx->Current.Normal, 0.0, 0.0, 1.0 );
      for (i=0; i<MAX_TEX_SETS; i++)
         ASSIGN_4V( ctx->Current.MultiTexCoord[i], 0.0, 0.0, 0.0, 1.0 );
      ctx->Current.TexCoord = ctx->Current.MultiTexCoord[0];
      ASSIGN_4V( ctx->Current.RasterPos, 0.0, 0.0, 0.0, 1.0 );
      ctx->Current.RasterDistance = 0.0;
      ASSIGN_4V( ctx->Current.RasterColor, 1.0, 1.0, 1.0, 1.0 );
      ctx->Current.RasterIndex = 1;
      for (i=0; i<MAX_TEX_SETS; i++)
         ASSIGN_4V( ctx->Current.RasterMultiTexCoord[i], 0.0, 0.0, 0.0, 1.0 );
      ctx->Current.RasterTexCoord = ctx->Current.RasterMultiTexCoord[0];
      ctx->Current.RasterPosValid = GL_TRUE;
      ctx->Current.EdgeFlag = GL_TRUE;

      /* Depth buffer group */
      ctx->Depth.Test = GL_FALSE;
      ctx->Depth.Clear = 1.0;
      ctx->Depth.Func = GL_LESS;
      ctx->Depth.Mask = GL_TRUE;

      /* Evaluators group */
      ctx->Eval.Map1Color4 = GL_FALSE;
      ctx->Eval.Map1Index = GL_FALSE;
      ctx->Eval.Map1Normal = GL_FALSE;
      ctx->Eval.Map1TextureCoord1 = GL_FALSE;
      ctx->Eval.Map1TextureCoord2 = GL_FALSE;
      ctx->Eval.Map1TextureCoord3 = GL_FALSE;
      ctx->Eval.Map1TextureCoord4 = GL_FALSE;
      ctx->Eval.Map1Vertex3 = GL_FALSE;
      ctx->Eval.Map1Vertex4 = GL_FALSE;
      ctx->Eval.Map2Color4 = GL_FALSE;
      ctx->Eval.Map2Index = GL_FALSE;
      ctx->Eval.Map2Normal = GL_FALSE;
      ctx->Eval.Map2TextureCoord1 = GL_FALSE;
      ctx->Eval.Map2TextureCoord2 = GL_FALSE;
      ctx->Eval.Map2TextureCoord3 = GL_FALSE;
      ctx->Eval.Map2TextureCoord4 = GL_FALSE;
      ctx->Eval.Map2Vertex3 = GL_FALSE;
      ctx->Eval.Map2Vertex4 = GL_FALSE;
      ctx->Eval.AutoNormal = GL_FALSE;
      ctx->Eval.MapGrid1un = 1;
      ctx->Eval.MapGrid1u1 = 0.0;
      ctx->Eval.MapGrid1u2 = 1.0;
      ctx->Eval.MapGrid2un = 1;
      ctx->Eval.MapGrid2vn = 1;
      ctx->Eval.MapGrid2u1 = 0.0;
      ctx->Eval.MapGrid2u2 = 1.0;
      ctx->Eval.MapGrid2v1 = 0.0;
      ctx->Eval.MapGrid2v2 = 1.0;

      /* Evaluator data */
      {
         static GLfloat vertex[4] = { 0.0, 0.0, 0.0, 1.0 };
         static GLfloat normal[3] = { 0.0, 0.0, 1.0 };
         static GLfloat index[1] = { 1.0 };
         static GLfloat color[4] = { 1.0, 1.0, 1.0, 1.0 };
         static GLfloat texcoord[4] = { 0.0, 0.0, 0.0, 1.0 };

         init_1d_map( &ctx->EvalMap.Map1Vertex3, 3, vertex );
         init_1d_map( &ctx->EvalMap.Map1Vertex4, 4, vertex );
         init_1d_map( &ctx->EvalMap.Map1Index, 1, index );
         init_1d_map( &ctx->EvalMap.Map1Color4, 4, color );
         init_1d_map( &ctx->EvalMap.Map1Normal, 3, normal );
         init_1d_map( &ctx->EvalMap.Map1Texture1, 1, texcoord );
         init_1d_map( &ctx->EvalMap.Map1Texture2, 2, texcoord );
         init_1d_map( &ctx->EvalMap.Map1Texture3, 3, texcoord );
         init_1d_map( &ctx->EvalMap.Map1Texture4, 4, texcoord );

         init_2d_map( &ctx->EvalMap.Map2Vertex3, 3, vertex );
         init_2d_map( &ctx->EvalMap.Map2Vertex4, 4, vertex );
         init_2d_map( &ctx->EvalMap.Map2Index, 1, index );
         init_2d_map( &ctx->EvalMap.Map2Color4, 4, color );
         init_2d_map( &ctx->EvalMap.Map2Normal, 3, normal );
         init_2d_map( &ctx->EvalMap.Map2Texture1, 1, texcoord );
         init_2d_map( &ctx->EvalMap.Map2Texture2, 2, texcoord );
         init_2d_map( &ctx->EvalMap.Map2Texture3, 3, texcoord );
         init_2d_map( &ctx->EvalMap.Map2Texture4, 4, texcoord );
      }

      /* Fog group */
      ctx->Fog.Enabled = GL_FALSE;
      ctx->Fog.Mode = GL_EXP;
      ASSIGN_4V( ctx->Fog.Color, 0.0, 0.0, 0.0, 0.0 );
      ctx->Fog.Index = 0.0;
      ctx->Fog.Density = 1.0;
      ctx->Fog.Start = 0.0;
      ctx->Fog.End = 1.0;

      /* Hint group */
      ctx->Hint.PerspectiveCorrection = GL_DONT_CARE;
      ctx->Hint.PointSmooth = GL_DONT_CARE;
      ctx->Hint.LineSmooth = GL_DONT_CARE;
      ctx->Hint.PolygonSmooth = GL_DONT_CARE;
      ctx->Hint.Fog = GL_DONT_CARE;

      /* Lighting group */
      for (i=0;i<MAX_LIGHTS;i++) {
	 init_light( &ctx->Light.Light[i], i );
      }
      init_lightmodel( &ctx->Light.Model );
      init_material( &ctx->Light.Material[0] );
      init_material( &ctx->Light.Material[1] );
      ctx->Light.ShadeModel = GL_SMOOTH;
      ctx->Light.Enabled = GL_FALSE;
      ctx->Light.ColorMaterialFace = GL_FRONT_AND_BACK;
      ctx->Light.ColorMaterialMode = GL_AMBIENT_AND_DIFFUSE;
      ctx->Light.ColorMaterialBitmask
         = gl_material_bitmask( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );

      ctx->Light.ColorMaterialEnabled = GL_FALSE;

      /* Line group */
      ctx->Line.SmoothFlag = GL_FALSE;
      ctx->Line.StippleFlag = GL_FALSE;
      ctx->Line.Width = 1.0;
      ctx->Line.StipplePattern = 0xffff;
      ctx->Line.StippleFactor = 1;

      /* Display List group */
      ctx->List.ListBase = 0;

      /* Pixel group */
      ctx->Pixel.RedBias = 0.0;
      ctx->Pixel.RedScale = 1.0;
      ctx->Pixel.GreenBias = 0.0;
      ctx->Pixel.GreenScale = 1.0;
      ctx->Pixel.BlueBias = 0.0;
      ctx->Pixel.BlueScale = 1.0;
      ctx->Pixel.AlphaBias = 0.0;
      ctx->Pixel.AlphaScale = 1.0;
      ctx->Pixel.ScaleOrBiasRGBA = GL_FALSE;
      ctx->Pixel.DepthBias = 0.0;
      ctx->Pixel.DepthScale = 1.0;
      ctx->Pixel.IndexOffset = 0;
      ctx->Pixel.IndexShift = 0;
      ctx->Pixel.ZoomX = 1.0;
      ctx->Pixel.ZoomY = 1.0;
      ctx->Pixel.MapColorFlag = GL_FALSE;
      ctx->Pixel.MapStencilFlag = GL_FALSE;
      ctx->Pixel.MapStoSsize = 1;
      ctx->Pixel.MapItoIsize = 1;
      ctx->Pixel.MapItoRsize = 1;
      ctx->Pixel.MapItoGsize = 1;
      ctx->Pixel.MapItoBsize = 1;
      ctx->Pixel.MapItoAsize = 1;
      ctx->Pixel.MapRtoRsize = 1;
      ctx->Pixel.MapGtoGsize = 1;
      ctx->Pixel.MapBtoBsize = 1;
      ctx->Pixel.MapAtoAsize = 1;
      ctx->Pixel.MapStoS[0] = 0;
      ctx->Pixel.MapItoI[0] = 0;
      ctx->Pixel.MapItoR[0] = 0.0;
      ctx->Pixel.MapItoG[0] = 0.0;
      ctx->Pixel.MapItoB[0] = 0.0;
      ctx->Pixel.MapItoA[0] = 0.0;
      ctx->Pixel.MapItoR8[0] = 0;
      ctx->Pixel.MapItoG8[0] = 0;
      ctx->Pixel.MapItoB8[0] = 0;
      ctx->Pixel.MapItoA8[0] = 0;
      ctx->Pixel.MapRtoR[0] = 0.0;
      ctx->Pixel.MapGtoG[0] = 0.0;
      ctx->Pixel.MapBtoB[0] = 0.0;
      ctx->Pixel.MapAtoA[0] = 0.0;

      /* Point group */
      ctx->Point.SmoothFlag = GL_FALSE;
      ctx->Point.Size = 1.0;
      ctx->Point.Params[0] = 1.0;
      ctx->Point.Params[1] = 0.0;
      ctx->Point.Params[2] = 0.0;
      ctx->Point.MinSize = 0.0;
      ctx->Point.MaxSize = (GLfloat) MAX_POINT_SIZE;
      ctx->Point.Threshold = 1.0;

      /* Polygon group */
      ctx->Polygon.CullFlag = GL_FALSE;
      ctx->Polygon.CullFaceMode = GL_BACK;
      ctx->Polygon.FrontFace = GL_CCW;
      ctx->Polygon.FrontMode = GL_FILL;
      ctx->Polygon.BackMode = GL_FILL;
      ctx->Polygon.Unfilled = GL_FALSE;
      ctx->Polygon.SmoothFlag = GL_FALSE;
      ctx->Polygon.StippleFlag = GL_FALSE;
      ctx->Polygon.OffsetFactor = 0.0F;
      ctx->Polygon.OffsetUnits = 0.0F;
      ctx->Polygon.OffsetPoint = GL_FALSE;
      ctx->Polygon.OffsetLine = GL_FALSE;
      ctx->Polygon.OffsetFill = GL_FALSE;
      ctx->Polygon.OffsetAny = GL_FALSE;

      /* Polygon Stipple group */
      MEMSET( ctx->PolygonStipple, 0xff, 32*sizeof(GLuint) );

      /* Scissor group */
      ctx->Scissor.Enabled = GL_FALSE;
      ctx->Scissor.X = 0;
      ctx->Scissor.Y = 0;
      ctx->Scissor.Width = 0;
      ctx->Scissor.Height = 0;

      /* Stencil group */
      ctx->Stencil.Enabled = GL_FALSE;
      ctx->Stencil.Function = GL_ALWAYS;
      ctx->Stencil.FailFunc = GL_KEEP;
      ctx->Stencil.ZPassFunc = GL_KEEP;
      ctx->Stencil.ZFailFunc = GL_KEEP;
      ctx->Stencil.Ref = 0;
      ctx->Stencil.ValueMask = 0xff;
      ctx->Stencil.Clear = 0;
      ctx->Stencil.WriteMask = 0xff;

      /* Texture group */
      ctx->Texture.CurrentSet = 0;      /* GL_SGIS_multitexture */
      ctx->Texture.CurrentTransformSet = 0; /* GL_EXT_multitexture */
      ctx->Texture.Enabled = 0;

      ctx->Texture.AnyDirty = GL_FALSE;
      for (i=0; i<MAX_TEX_SETS; i++)
         init_texture_set( ctx, i );

      ctx->Texture.SharedPalette = GL_FALSE;
      ctx->Texture.Palette[0] = 255;
      ctx->Texture.Palette[1] = 255;
      ctx->Texture.Palette[2] = 255;
      ctx->Texture.Palette[3] = 255;
      ctx->Texture.PaletteSize = 1;
      ctx->Texture.PaletteIntFormat = GL_RGBA;
      ctx->Texture.PaletteFormat = GL_RGBA;

      /* Transformation group */
      ctx->Transform.MatrixMode = GL_MODELVIEW;
      ctx->Transform.Normalize = GL_FALSE;
      ctx->Transform.RescaleNormals = GL_FALSE;
      for (i=0;i<MAX_CLIP_PLANES;i++) {
	 ctx->Transform.ClipEnabled[i] = GL_FALSE;
         ASSIGN_4V( ctx->Transform.ClipEquation[i], 0.0, 0.0, 0.0, 0.0 );
      }
      ctx->Transform.AnyClip = GL_FALSE;

      /* Viewport group */
      ctx->Viewport.X = 0;
      ctx->Viewport.Y = 0;
      ctx->Viewport.Width = 0;
      ctx->Viewport.Height = 0;   
      ctx->Viewport.Near = 0.0;
      ctx->Viewport.Far = 1.0;
      ctx->Viewport.Sx = 0.0;  /* Sx, Tx, Sy, Ty are computed later */
      ctx->Viewport.Tx = 0.0;
      ctx->Viewport.Sy = 0.0;
      ctx->Viewport.Ty = 0.0;
      ctx->Viewport.Sz = 0.5 * DEPTH_SCALE;
      ctx->Viewport.Tz = 0.5 * DEPTH_SCALE;

      /* Vertex arrays */
      ctx->Array.VertexSize = 4;
      ctx->Array.VertexType = GL_FLOAT;
      ctx->Array.VertexStride = 0;
      ctx->Array.VertexStrideB = 0;
      ctx->Array.VertexPtr = NULL;
      ctx->Array.VertexEnabled = GL_FALSE;
      ctx->Array.NormalType = GL_FLOAT;
      ctx->Array.NormalStride = 0;
      ctx->Array.NormalStrideB = 0;
      ctx->Array.NormalPtr = NULL;
      ctx->Array.NormalEnabled = GL_FALSE;
      ctx->Array.ColorSize = 4;
      ctx->Array.ColorType = GL_FLOAT;
      ctx->Array.ColorStride = 0;
      ctx->Array.ColorStrideB = 0;
      ctx->Array.ColorPtr = NULL;
      ctx->Array.ColorEnabled = GL_FALSE;
      ctx->Array.IndexType = GL_FLOAT;
      ctx->Array.IndexStride = 0;
      ctx->Array.IndexStrideB = 0;
      ctx->Array.IndexPtr = NULL;
      ctx->Array.IndexEnabled = GL_FALSE;
      for (i = 0; i < MAX_TEX_COORD_SETS; i++) {
         ctx->Array.TexCoordSize[i] = 4;
         ctx->Array.TexCoordType[i] = GL_FLOAT;
         ctx->Array.TexCoordStride[i] = 0;
         ctx->Array.TexCoordStrideB[i] = 0;
         ctx->Array.TexCoordPtr[i] = NULL;
         ctx->Array.TexCoordEnabled[i] = GL_FALSE;
      }
      ctx->Array.TexCoordInterleaveFactor = 1;
      ctx->Array.EdgeFlagStride = 0;
      ctx->Array.EdgeFlagStrideB = 0;
      ctx->Array.EdgeFlagPtr = NULL;
      ctx->Array.EdgeFlagEnabled = GL_FALSE;

      /* Pixel transfer */
      ctx->Pack.Alignment = 4;
      ctx->Pack.RowLength = 0;
      ctx->Pack.SkipPixels = 0;
      ctx->Pack.SkipRows = 0;
      ctx->Pack.SwapBytes = GL_FALSE;
      ctx->Pack.LsbFirst = GL_FALSE;
      ctx->Unpack.Alignment = 4;
      ctx->Unpack.RowLength = 0;
      ctx->Unpack.SkipPixels = 0;
      ctx->Unpack.SkipRows = 0;
      ctx->Unpack.SwapBytes = GL_FALSE;
      ctx->Unpack.LsbFirst = GL_FALSE;

      /* Feedback */
      ctx->Feedback.Type = GL_2D;   /* TODO: verify */
      ctx->Feedback.Buffer = NULL;
      ctx->Feedback.BufferSize = 0;
      ctx->Feedback.Count = 0;

      /* Selection/picking */
      ctx->Select.Buffer = NULL;
      ctx->Select.BufferSize = 0;
      ctx->Select.BufferCount = 0;
      ctx->Select.Hits = 0;
      ctx->Select.NameStackDepth = 0;

      /* GL_EXT_multitexture */
      ctx->TexCoordSet = 0;

      /* Renderer and client attribute stacks */
      ctx->AttribStackDepth = 0;
      ctx->ClientAttribStackDepth = 0;

      /*** Miscellaneous ***/
      ctx->NewState = NEW_ALL;
      ctx->RenderMode = GL_RENDER;
      ctx->Primitive = GL_BITMAP;
      ctx->StippleCounter = 0;
      ctx->NeedNormals = GL_FALSE;
      ctx->DoViewportMapping = GL_TRUE;

      /* Display list */
      ctx->CallDepth = 0;
      ctx->ExecuteFlag = GL_TRUE;
      ctx->CompileFlag = GL_FALSE;
      ctx->CurrentListPtr = NULL;
      ctx->CurrentBlock = NULL;
      ctx->CurrentListNum = 0;
      ctx->CurrentPos = 0;

      ctx->ErrorValue = (GLenum) GL_NO_ERROR;

      /* For debug/development only */
      ctx->NoRaster = getenv("MESA_NO_RASTER") ? GL_TRUE : GL_FALSE;

      /* Dither disable */
      ctx->NoDither = getenv("MESA_NO_DITHER") ? GL_TRUE : GL_FALSE;
      if (ctx->NoDither) {
         if (getenv("MESA_DEBUG")) {
            fprintf(stderr, "MESA_NO_DITHER set - dithering disabled\n");
         }
         ctx->Color.DitherFlag = GL_FALSE;
      }
   }
}



/*
 * Allocate a new GLvisual object.
 * Input:  rgbFlag - GL_TRUE=RGB(A) mode, GL_FALSE=Color Index mode
 *         alphaFlag - alloc software alpha buffers?
 *         dbFlag - double buffering?
 *         stereoFlag - stereo buffer?
 *         depthFits - requested minimum bits per depth buffer value
 *         stencilFits - requested minimum bits per stencil buffer value
 *         accumFits - requested minimum bits per accum buffer component
 *         indexFits - number of bits per pixel if rgbFlag==GL_FALSE
 *         red/green/blue/alphaFits - number of bits per color component
 *                                     in frame buffer for RGB(A) mode.
 * Return:  pointer to new GLvisual or NULL if requested parameters can't
 *          be met.
 */
GLvisual *gl_create_visual( GLboolean rgbFlag,
                            GLboolean alphaFlag,
                            GLboolean dbFlag,
                            GLboolean stereoFlag,
                            GLint depthBits,
                            GLint stencilBits,
                            GLint accumBits,
                            GLint indexBits,
                            GLint redBits,
                            GLint greenBits,
                            GLint blueBits,
                            GLint alphaBits )
{
   GLvisual *vis;

   if (stereoFlag) {
      gl_warning(NULL, "stereo not supported");
      return NULL;   /* Stereo isn't supported yet! */
   }

   if (depthBits > (GLint) (8*sizeof(GLdepth))) {
      /* can't meet depth buffer requirements */
      return NULL;
   }
   if (stencilBits > (GLint) (8*sizeof(GLstencil))) {
      /* can't meet stencil buffer requirements */
      return NULL;
   }
   if (accumBits > (GLint) (8*sizeof(GLaccum))) {
      /* can't meet accum buffer requirements */
      return NULL;
   }

   vis = (GLvisual *) calloc( 1, sizeof(GLvisual) );
   if (!vis) {
      return NULL;
   }

   vis->RGBAflag   = rgbFlag;
   vis->DBflag     = dbFlag;
   vis->StereoFlag = stereoFlag;
   vis->RedBits    = redBits;
   vis->GreenBits  = greenBits;
   vis->BlueBits   = blueBits;
   vis->AlphaBits  = alphaFlag ? 8*sizeof(GLubyte) : alphaBits;

   vis->IndexBits   = indexBits;
   vis->DepthBits   = (depthBits>0) ? 8*sizeof(GLdepth) : 0;
   vis->AccumBits   = (accumBits>0) ? 8*sizeof(GLaccum) : 0;
   vis->StencilBits = (stencilBits>0) ? 8*sizeof(GLstencil) : 0;

   /* software alpha buffers */
   if (alphaFlag) {
      vis->FrontAlphaEnabled = GL_TRUE;
      if (dbFlag) {
         vis->BackAlphaEnabled = GL_TRUE;
      }
   }

   return vis;
}




void gl_destroy_visual( GLvisual *vis )
{
   free( vis );
}



/*
 * Allocate the proxy textures.  If we run out of memory part way through
 * the allocations clean up and return GL_FALSE.
 * Return:  GL_TRUE=success, GL_FALSE=failure
 */
static GLboolean alloc_proxy_textures( GLcontext *ctx )
{
   GLboolean out_of_memory;
   GLint i;

   ctx->Texture.Proxy1D = gl_alloc_texture_object(NULL, 0, 1);
   if (!ctx->Texture.Proxy1D) {
      return GL_FALSE;
   }

   ctx->Texture.Proxy2D = gl_alloc_texture_object(NULL, 0, 2);
   if (!ctx->Texture.Proxy2D) {
      gl_free_texture_object(NULL, ctx->Texture.Proxy1D);
      return GL_FALSE;
   }

   ctx->Texture.Proxy3D = gl_alloc_texture_object(NULL, 0, 3);
   if (!ctx->Texture.Proxy3D) {
      gl_free_texture_object(NULL, ctx->Texture.Proxy1D);
      gl_free_texture_object(NULL, ctx->Texture.Proxy2D);
      return GL_FALSE;
   }

   out_of_memory = GL_FALSE;
   for (i=0;i<MAX_TEXTURE_LEVELS;i++) {
      ctx->Texture.Proxy1D->Image[i] = gl_alloc_texture_image();
      ctx->Texture.Proxy2D->Image[i] = gl_alloc_texture_image();
      ctx->Texture.Proxy3D->Image[i] = gl_alloc_texture_image();
      if (!ctx->Texture.Proxy1D->Image[i]
          || !ctx->Texture.Proxy2D->Image[i]
          || !ctx->Texture.Proxy3D->Image[i]) {
         out_of_memory = GL_TRUE;
      }
   }
   if (out_of_memory) {
      for (i=0;i<MAX_TEXTURE_LEVELS;i++) {
         if (ctx->Texture.Proxy1D->Image[i]) {
            gl_free_texture_image(ctx->Texture.Proxy1D->Image[i]);
         }
         if (ctx->Texture.Proxy2D->Image[i]) {
            gl_free_texture_image(ctx->Texture.Proxy2D->Image[i]);
         }
         if (ctx->Texture.Proxy3D->Image[i]) {
            gl_free_texture_image(ctx->Texture.Proxy3D->Image[i]);
         }
      }
      gl_free_texture_object(NULL, ctx->Texture.Proxy1D);
      gl_free_texture_object(NULL, ctx->Texture.Proxy2D);
      gl_free_texture_object(NULL, ctx->Texture.Proxy3D);
      return GL_FALSE;
   }
   else {
      return GL_TRUE;
   }
}



/*
 * Allocate and initialize a GLcontext structure.
 * Input:  visual - a GLvisual pointer
 *         sharelist - another context to share display lists with or NULL
 *         driver_ctx - pointer to device driver's context state struct
 * Return:  pointer to a new gl_context struct or NULL if error.
 */
GLcontext *gl_create_context( GLvisual *visual,
                              GLcontext *share_list,
                              void *driver_ctx,
                              GLboolean direct )
{
   GLcontext *ctx;

   /* do some implementation tests */
   assert( sizeof(GLbyte) == 1 );
   assert( sizeof(GLshort) >= 2 );
   assert( sizeof(GLint) >= 4 );
   assert( sizeof(GLubyte) == 1 );
   assert( sizeof(GLushort) >= 2 );
   assert( sizeof(GLuint) >= 4 );

   /* misc one-time initializations */
   gl_init_math();
   gl_init_lists();
   gl_init_eval();

   ctx = (GLcontext *) calloc( 1, sizeof(GLcontext) );
   if (!ctx) {
      return NULL;
   }

   ctx->DriverCtx = driver_ctx;
   ctx->Visual = visual;
   ctx->Buffer = NULL;

   ctx->VB = gl_alloc_vb();
   if (!ctx->VB) {
      free( ctx );
      return NULL;
   }

   ctx->PB = gl_alloc_pb();
   if (!ctx->PB) {
      free( ctx->VB );
      free( ctx );
      return NULL;
   }

   if (share_list) {
      /* share the group of display lists of another context */
      ctx->Shared = share_list->Shared;
   }
   else {
      /* allocate new group of display lists */
      ctx->Shared = alloc_shared_state();
      if (!ctx->Shared) {
         free(ctx->VB);
         free(ctx->PB);
         free(ctx);
         return NULL;
      }
   }
   ctx->Shared->RefCount++;

   initialize_context( ctx );

   ctx->DirectContext = direct;

   if (visual->DBflag) {
      ctx->Color.DrawBuffer = GL_BACK;
      ctx->Pixel.ReadBuffer = GL_BACK;
   }
   else {
      ctx->Color.DrawBuffer = GL_FRONT;
      ctx->Pixel.ReadBuffer = GL_FRONT;
   }

#ifdef PROFILE
   init_timings( ctx );
#endif

#ifdef GL_VERSION_1_1
   if (!alloc_proxy_textures(ctx)) {
      free_shared_state(ctx, ctx->Shared);
      free(ctx->VB);
      free(ctx->PB);
      free(ctx);
      return NULL;
   }
#endif

   gl_init_api_function_pointers( ctx );
   ctx->API = ctx->Exec;   /* GL_EXECUTE is default */

   return ctx;
}



/*
 * Destroy a gl_context structure.
 */
void gl_destroy_context( GLcontext *ctx )
{
   if (ctx) {

#ifdef PROFILE
      if (getenv("MESA_PROFILE")) {
         print_timings( ctx );
      }
#endif

      free( ctx->PB );
      free( ctx->VB );

      ctx->Shared->RefCount--;
      assert(ctx->Shared->RefCount>=0);
      if (ctx->Shared->RefCount==0) {
	 /* free shared state */
	 free_shared_state( ctx, ctx->Shared );
      }

      /* Free proxy texture objects */
      gl_free_texture_object( NULL, ctx->Texture.Proxy1D );
      gl_free_texture_object( NULL, ctx->Texture.Proxy2D );
      gl_free_texture_object( NULL, ctx->Texture.Proxy3D );

      /* Free evaluator data */
      if (ctx->EvalMap.Map1Vertex3.Points)
         free( ctx->EvalMap.Map1Vertex3.Points );
      if (ctx->EvalMap.Map1Vertex4.Points)
         free( ctx->EvalMap.Map1Vertex4.Points );
      if (ctx->EvalMap.Map1Index.Points)
         free( ctx->EvalMap.Map1Index.Points );
      if (ctx->EvalMap.Map1Color4.Points)
         free( ctx->EvalMap.Map1Color4.Points );
      if (ctx->EvalMap.Map1Normal.Points)
         free( ctx->EvalMap.Map1Normal.Points );
      if (ctx->EvalMap.Map1Texture1.Points)
         free( ctx->EvalMap.Map1Texture1.Points );
      if (ctx->EvalMap.Map1Texture2.Points)
         free( ctx->EvalMap.Map1Texture2.Points );
      if (ctx->EvalMap.Map1Texture3.Points)
         free( ctx->EvalMap.Map1Texture3.Points );
      if (ctx->EvalMap.Map1Texture4.Points)
         free( ctx->EvalMap.Map1Texture4.Points );

      if (ctx->EvalMap.Map2Vertex3.Points)
         free( ctx->EvalMap.Map2Vertex3.Points );
      if (ctx->EvalMap.Map2Vertex4.Points)
         free( ctx->EvalMap.Map2Vertex4.Points );
      if (ctx->EvalMap.Map2Index.Points)
         free( ctx->EvalMap.Map2Index.Points );
      if (ctx->EvalMap.Map2Color4.Points)
         free( ctx->EvalMap.Map2Color4.Points );
      if (ctx->EvalMap.Map2Normal.Points)
         free( ctx->EvalMap.Map2Normal.Points );
      if (ctx->EvalMap.Map2Texture1.Points)
         free( ctx->EvalMap.Map2Texture1.Points );
      if (ctx->EvalMap.Map2Texture2.Points)
         free( ctx->EvalMap.Map2Texture2.Points );
      if (ctx->EvalMap.Map2Texture3.Points)
         free( ctx->EvalMap.Map2Texture3.Points );
      if (ctx->EvalMap.Map2Texture4.Points)
         free( ctx->EvalMap.Map2Texture4.Points );

      free( (void *) ctx );

#ifndef THREADS
      if (ctx==CC) {
         CC = NULL;
      }
#endif

   }
}



/*
 * Create a new framebuffer.  A GLframebuffer is a struct which
 * encapsulates the depth, stencil and accum buffers and related
 * parameters.
 * Input:  visual - a GLvisual pointer
 * Return:  pointer to new GLframebuffer struct or NULL if error.
 */
GLframebuffer *gl_create_framebuffer( GLvisual *visual )
{
   GLframebuffer *buffer;

   buffer = (GLframebuffer *) calloc( 1, sizeof(GLframebuffer) );
   if (!buffer) {
      return NULL;
   }

   buffer->Visual = visual;

   return buffer;
}



/*
 * Free a framebuffer struct and its buffers.
 */
void gl_destroy_framebuffer( GLframebuffer *buffer )
{
   if (buffer) {
      if (buffer->Depth) {
         free( buffer->Depth );
      }
      if (buffer->Accum) {
         free( buffer->Accum );
      }
      if (buffer->Stencil) {
         free( buffer->Stencil );
      }
      if (buffer->FrontAlpha) {
         free( buffer->FrontAlpha );
      }
      if (buffer->BackAlpha) {
         free( buffer->BackAlpha );
      }
      free(buffer);
   }
}



/*
 * Set the current context, binding the given frame buffer to the context.
 */
void gl_make_current( GLcontext *ctx, GLframebuffer *buffer )
{
#ifdef THREADS
   /* TODO: unbind old buffer from context? */
   set_thread_context( ctx );
#else
   if (CC && CC->Buffer) {
      /* unbind frame buffer from context */
      CC->Buffer = NULL;
   }
   CC = ctx;
#endif

   if (ctx && buffer) {
      /* TODO: check if ctx and buffer's visual match??? */
      ctx->Buffer = buffer;      /* Bind the frame buffer to the context */
      ctx->NewState = NEW_ALL;   /* just to be safe */
      gl_update_state( ctx );
   }
}


/*
 * Return current context handle.
 */
GLcontext *gl_get_current_context( void )
{
#ifdef THREADS
   return gl_get_thread_context();
#else
   return CC;
#endif
}



/*
 * Copy attribute groups from one context to another.
 * Input:  src - source context
 *         dst - destination context
 *         mask - bitwise OR of GL_*_BIT flags
 */
void gl_copy_context( const GLcontext *src, GLcontext *dst, GLuint mask )
{
   if (mask & GL_ACCUM_BUFFER_BIT) {
      MEMCPY( &dst->Accum, &src->Accum, sizeof(struct gl_accum_attrib) );
   }
   if (mask & GL_COLOR_BUFFER_BIT) {
      MEMCPY( &dst->Color, &src->Color, sizeof(struct gl_colorbuffer_attrib) );
   }
   if (mask & GL_CURRENT_BIT) {
      MEMCPY( &dst->Current, &src->Current, sizeof(struct gl_current_attrib) );
   }
   if (mask & GL_DEPTH_BUFFER_BIT) {
      MEMCPY( &dst->Depth, &src->Depth, sizeof(struct gl_depthbuffer_attrib) );
   }
   if (mask & GL_ENABLE_BIT) {
      /* no op */
   }
   if (mask & GL_EVAL_BIT) {
      MEMCPY( &dst->Eval, &src->Eval, sizeof(struct gl_eval_attrib) );
   }
   if (mask & GL_FOG_BIT) {
      MEMCPY( &dst->Fog, &src->Fog, sizeof(struct gl_fog_attrib) );
   }
   if (mask & GL_HINT_BIT) {
      MEMCPY( &dst->Hint, &src->Hint, sizeof(struct gl_hint_attrib) );
   }
   if (mask & GL_LIGHTING_BIT) {
      MEMCPY( &dst->Light, &src->Light, sizeof(struct gl_light_attrib) );
   }
   if (mask & GL_LINE_BIT) {
      MEMCPY( &dst->Line, &src->Line, sizeof(struct gl_line_attrib) );
   }
   if (mask & GL_LIST_BIT) {
      MEMCPY( &dst->List, &src->List, sizeof(struct gl_list_attrib) );
   }
   if (mask & GL_PIXEL_MODE_BIT) {
      MEMCPY( &dst->Pixel, &src->Pixel, sizeof(struct gl_pixel_attrib) );
   }
   if (mask & GL_POINT_BIT) {
      MEMCPY( &dst->Point, &src->Point, sizeof(struct gl_point_attrib) );
   }
   if (mask & GL_POLYGON_BIT) {
      MEMCPY( &dst->Polygon, &src->Polygon, sizeof(struct gl_polygon_attrib) );
   }
   if (mask & GL_POLYGON_STIPPLE_BIT) {
      /* Use loop instead of MEMCPY due to problem with Portland Group's
       * C compiler.  Reported by John Stone.
       */
      int i;
      for (i=0;i<32;i++) {
         dst->PolygonStipple[i] = src->PolygonStipple[i];
      }
   }
   if (mask & GL_SCISSOR_BIT) {
      MEMCPY( &dst->Scissor, &src->Scissor, sizeof(struct gl_scissor_attrib) );
   }
   if (mask & GL_STENCIL_BUFFER_BIT) {
      MEMCPY( &dst->Stencil, &src->Stencil, sizeof(struct gl_stencil_attrib) );
   }
   if (mask & GL_TEXTURE_BIT) {
      MEMCPY( &dst->Texture, &src->Texture, sizeof(struct gl_texture_attrib) );
   }
   if (mask & GL_TRANSFORM_BIT) {
      MEMCPY( &dst->Transform, &src->Transform, sizeof(struct gl_transform_attrib) );
   }
   if (mask & GL_VIEWPORT_BIT) {
      MEMCPY( &dst->Viewport, &src->Viewport, sizeof(struct gl_viewport_attrib) );
   }
}



/*
 * Someday a GLS library or OpenGL-like debugger may call this function
 * to register it's own set of API entry points.
 * Input: ctx - the context to set API pointers for
 *        api - if NULL, restore original API pointers
 *              else, set API function table to this table.
 */
void gl_set_api_table( GLcontext *ctx, const struct gl_api_table *api )
{
   if (api) {
      MEMCPY( &ctx->API, api, sizeof(struct gl_api_table) );
   }
   else {
      MEMCPY( &ctx->API, &ctx->Exec, sizeof(struct gl_api_table) );
   }
}




/**********************************************************************/
/*****                Miscellaneous functions                     *****/
/**********************************************************************/


/*
 * This function is called when the Mesa user has stumbled into a code
 * path which may not be implemented fully or correctly.
 */
void gl_problem( const GLcontext *ctx, const char *s )
{
   fprintf( stderr, "Mesa implementation error: %s\n", s );
   fprintf( stderr, "Report to Mesa author.\n" );
   (void) ctx;
}



/*
 * This is called to inform the user that he or she has tried to do
 * something illogical or if there's likely a bug in their program
 * (like enabled depth testing without a depth buffer).
 */
void gl_warning( const GLcontext *ctx, const char *s )
{
   GLboolean debug;
#ifdef DEBUG
   debug = GL_TRUE;
#else
   if (getenv("MESA_DEBUG")) {
      debug = GL_TRUE;
   }
   else {
      debug = GL_FALSE;
   }
#endif
   if (debug) {
      fprintf( stderr, "Mesa warning: %s\n", s );
   }
   (void) ctx;
}



/*
 * This is Mesa's error handler.  Normally, all that's done is the updating
 * of the current error value.  If Mesa is compiled with -DDEBUG or if the
 * environment variable "MESA_DEBUG" is defined then a real error message
 * is printed to stderr.
 * Input:  error - the error value
 *         s - a diagnostic string
 */
void gl_error( GLcontext *ctx, GLenum error, const char *s )
{
   GLboolean debug;

#ifdef DEBUG
   debug = GL_TRUE;
#else
   if (getenv("MESA_DEBUG")) {
      debug = GL_TRUE;
   }
   else {
      debug = GL_FALSE;
   }
#endif

   if (debug) {
      char errstr[1000];

      switch (error) {
	 case GL_NO_ERROR:
	    strcpy( errstr, "GL_NO_ERROR" );
	    break;
	 case GL_INVALID_VALUE:
	    strcpy( errstr, "GL_INVALID_VALUE" );
	    break;
	 case GL_INVALID_ENUM:
	    strcpy( errstr, "GL_INVALID_ENUM" );
	    break;
	 case GL_INVALID_OPERATION:
	    strcpy( errstr, "GL_INVALID_OPERATION" );
	    break;
	 case GL_STACK_OVERFLOW:
	    strcpy( errstr, "GL_STACK_OVERFLOW" );
	    break;
	 case GL_STACK_UNDERFLOW:
	    strcpy( errstr, "GL_STACK_UNDERFLOW" );
	    break;
	 case GL_OUT_OF_MEMORY:
	    strcpy( errstr, "GL_OUT_OF_MEMORY" );
	    break;
	 default:
	    strcpy( errstr, "unknown" );
	    break;
      }
      fprintf( stderr, "Mesa user error: %s in %s\n", errstr, s );
   }

   if (ctx->ErrorValue==GL_NO_ERROR) {
      ctx->ErrorValue = error;
   }

   /* Call device driver's error handler, if any.  This is used on the Mac. */
   if (ctx->Driver.Error) {
      (*ctx->Driver.Error)( ctx );
   }
}




GLenum gl_GetError( GLcontext *ctx )
{
   GLenum e;

   if (INSIDE_BEGIN_END(ctx)) {
      gl_error( ctx, GL_INVALID_OPERATION, "glGetError" );
      return GL_INVALID_OPERATION;
   }

   e = ctx->ErrorValue;
   ctx->ErrorValue = (GLenum) GL_NO_ERROR;
   return e;
}



void gl_ResizeBuffersMESA( GLcontext *ctx )
{
   GLint newsize;
   GLuint buf_width, buf_height;
   
   ctx->NewState |= NEW_ALL;   /* just to be safe */

   /* ask device driver for size of output buffer */
   (*ctx->Driver.GetBufferSize)( ctx, &buf_width, &buf_height );

   /* see if size of device driver's color buffer (window) has changed */
   newsize = ctx->Buffer->Width != (GLint) buf_width
          || ctx->Buffer->Height != (GLint) buf_height;

   /* save buffer size */
   ctx->Buffer->Width = buf_width;
   ctx->Buffer->Height = buf_height;

   /* Reallocate other buffers if needed. */
   if (newsize && ctx->Visual->DepthBits>0) {
      /* reallocate depth buffer */
      (*ctx->Driver.AllocDepthBuffer)( ctx );
   }
   if (newsize && ctx->Visual->StencilBits>0) {
      /* reallocate stencil buffer */
      gl_alloc_stencil_buffer( ctx );
   }
   if (newsize && ctx->Visual->AccumBits>0) {
      /* reallocate accum buffer */
      gl_alloc_accum_buffer( ctx );
   }
   if (newsize
       && (ctx->Visual->FrontAlphaEnabled || ctx->Visual->BackAlphaEnabled)) {
      gl_alloc_alpha_buffers( ctx );
   }
}




/**********************************************************************/
/*****                   State update logic                       *****/
/**********************************************************************/


/*
 * Since the device driver may or may not support pixel logic ops we
 * have to make some extensive tests to determine whether or not
 * software-implemented logic operations have to be used.
 */
static void update_pixel_logic( GLcontext *ctx )
{
   if (ctx->Visual->RGBAflag) {
      /* RGBA mode blending w/ Logic Op */
      if (ctx->Color.ColorLogicOpEnabled) {
	 if (ctx->Driver.LogicOp
             && (*ctx->Driver.LogicOp)( ctx, ctx->Color.LogicOp )) {
	    /* Device driver can do logic, don't have to do it in software */
	    ctx->Color.SWLogicOpEnabled = GL_FALSE;
	 }
	 else {
	    /* Device driver can't do logic op so we do it in software */
	    ctx->Color.SWLogicOpEnabled = GL_TRUE;
	 }
      }
      else {
	 /* no logic op */
	 if (ctx->Driver.LogicOp) {
            (void) (*ctx->Driver.LogicOp)( ctx, GL_COPY );
         }
	 ctx->Color.SWLogicOpEnabled = GL_FALSE;
      }
   }
   else {
      /* CI mode Logic Op */
      if (ctx->Color.IndexLogicOpEnabled) {
	 if (ctx->Driver.LogicOp
             && (*ctx->Driver.LogicOp)( ctx, ctx->Color.LogicOp )) {
	    /* Device driver can do logic, don't have to do it in software */
	    ctx->Color.SWLogicOpEnabled = GL_FALSE;
	 }
	 else {
	    /* Device driver can't do logic op so we do it in software */
	    ctx->Color.SWLogicOpEnabled = GL_TRUE;
	 }
      }
      else {
	 /* no logic op */
	 if (ctx->Driver.LogicOp) {
            (void) (*ctx->Driver.LogicOp)( ctx, GL_COPY );
         }
	 ctx->Color.SWLogicOpEnabled = GL_FALSE;
      }
   }
}



/*
 * Check if software implemented RGBA or Color Index masking is needed.
 */
static void update_pixel_masking( GLcontext *ctx )
{
   if (ctx->Visual->RGBAflag) {
      GLuint *colorMask = (GLuint *) ctx->Color.ColorMask;
      if (*colorMask == 0xffffffff) {
         /* disable masking */
         if (ctx->Driver.ColorMask) {
            (void) (*ctx->Driver.ColorMask)( ctx, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
         }
         ctx->Color.SWmasking = GL_FALSE;
      }
      else {
         /* Ask driver to do color masking, if it can't then
          * do it in software
          */
         GLboolean red   = ctx->Color.ColorMask[RCOMP] ? GL_TRUE : GL_FALSE;
         GLboolean green = ctx->Color.ColorMask[GCOMP] ? GL_TRUE : GL_FALSE;
         GLboolean blue  = ctx->Color.ColorMask[BCOMP] ? GL_TRUE : GL_FALSE;
         GLboolean alpha = ctx->Color.ColorMask[ACOMP] ? GL_TRUE : GL_FALSE;
         if (ctx->Driver.ColorMask
             && (*ctx->Driver.ColorMask)( ctx, red, green, blue, alpha )) {
            ctx->Color.SWmasking = GL_FALSE;
         }
         else {
            ctx->Color.SWmasking = GL_TRUE;
         }
      }
   }
   else {
      if (ctx->Color.IndexMask==0xffffffff) {
         /* disable masking */
         if (ctx->Driver.IndexMask) {
            (void) (*ctx->Driver.IndexMask)( ctx, 0xffffffff );
         }
         ctx->Color.SWmasking = GL_FALSE;
      }
      else {
         /* Ask driver to do index masking, if it can't then
          * do it in software
          */
         if (ctx->Driver.IndexMask
             && (*ctx->Driver.IndexMask)( ctx, ctx->Color.IndexMask )) {
            ctx->Color.SWmasking = GL_FALSE;
         }
         else {
            ctx->Color.SWmasking = GL_TRUE;
         }
      }
   }
}


static void update_fog_mode( GLcontext *ctx )
{
   if (ctx->Fog.Enabled) {
      if (ctx->Texture.Enabled)
         ctx->FogMode = FOG_FRAGMENT;
      else if (ctx->Hint.Fog == GL_NICEST)
         ctx->FogMode = FOG_FRAGMENT;
      else
         ctx->FogMode = FOG_VERTEX;

      if (ctx->Driver.GetParameteri)
         if ((ctx->Driver.GetParameteri)( ctx, DD_HAVE_HARDWARE_FOG ))
            ctx->FogMode = FOG_FRAGMENT;
   }
   else {
      ctx->FogMode = FOG_NONE;
   }
}


/*
 * Recompute the value of ctx->RasterMask, etc. according to
 * the current context.
 */
static void update_rasterflags( GLcontext *ctx )
{
   ctx->RasterMask = 0;

   if (ctx->Color.AlphaEnabled)		ctx->RasterMask |= ALPHATEST_BIT;
   if (ctx->Color.BlendEnabled)		ctx->RasterMask |= BLEND_BIT;
   if (ctx->Depth.Test)			ctx->RasterMask |= DEPTH_BIT;
   if (ctx->FogMode==FOG_FRAGMENT)	ctx->RasterMask |= FOG_BIT;
   if (ctx->Color.SWLogicOpEnabled)	ctx->RasterMask |= LOGIC_OP_BIT;
   if (ctx->Scissor.Enabled)		ctx->RasterMask |= SCISSOR_BIT;
   if (ctx->Stencil.Enabled)		ctx->RasterMask |= STENCIL_BIT;
   if (ctx->Color.SWmasking)		ctx->RasterMask |= MASKING_BIT;
   if (ctx->Visual->FrontAlphaEnabled)	ctx->RasterMask |= ALPHABUF_BIT;
   if (ctx->Visual->BackAlphaEnabled)	ctx->RasterMask |= ALPHABUF_BIT;

   if (   ctx->Viewport.X<0
       || ctx->Viewport.X + ctx->Viewport.Width > ctx->Buffer->Width
       || ctx->Viewport.Y<0
       || ctx->Viewport.Y + ctx->Viewport.Height > ctx->Buffer->Height) {
      ctx->RasterMask |= WINCLIP_BIT;
   }

   /* check if drawing to front and back buffers */
   if (ctx->Color.DrawBuffer==GL_FRONT_AND_BACK) {
      ctx->RasterMask |= FRONT_AND_BACK_BIT;
   }

   /* check if writing to color buffer(s) is disabled */
   if (ctx->Color.DrawBuffer==GL_NONE) {
      ctx->RasterMask |= NO_DRAW_BIT;
   }
   else if (ctx->Visual->RGBAflag && ctx->Color.ColorMask==0) {
      ctx->RasterMask |= NO_DRAW_BIT;
   }
   else if (!ctx->Visual->RGBAflag && ctx->Color.IndexMask==0) {
      ctx->RasterMask |= NO_DRAW_BIT;
   }
}


/*
 * Recompute the value of ctx->ClipMask according to the current context.
 * ClipMask depends on Texturing and Lighting.
 */
static void update_clipmask(GLcontext *ctx)
{
   /* Recompute ClipMask (what has to be interpolated when clipping) */
   ctx->ClipMask = 0;
   if (ctx->Texture.Enabled) {
      ctx->ClipMask |= CLIP_TEXTURE_BIT;
   }
   if (ctx->Light.ShadeModel==GL_SMOOTH) {
      if (ctx->Visual->RGBAflag) {
	 ctx->ClipMask |= CLIP_FCOLOR_BIT;
	 if (ctx->Light.Model.TwoSide) {
	    ctx->ClipMask |= CLIP_BCOLOR_BIT;
	 }
      }
      else {
	 ctx->ClipMask |= CLIP_FINDEX_BIT;
	 if (ctx->Light.Model.TwoSide) {
	    ctx->ClipMask |= CLIP_BINDEX_BIT;
	 }
      }
   }
 
   if (ctx->Texture.Enabled >= TEXTURE1_1D) {
      /* Multi texture coords */
      ctx->ClipInterpFunc = gl_clip_interp_all;
   }
   else {
      switch(ctx->ClipMask) {
         case CLIP_FCOLOR_BIT | CLIP_TEXTURE_BIT:
            ctx->ClipInterpFunc = gl_clip_interp_color_tex;
            break;
         case CLIP_TEXTURE_BIT:
            ctx->ClipInterpFunc = gl_clip_interp_tex;
            break;
         case CLIP_FCOLOR_BIT:
            ctx->ClipInterpFunc = gl_clip_interp_color;
            break;
         default:
            ctx->ClipInterpFunc = gl_clip_interp_all;
      }
   }
}



/*
 * If ctx->NewState is non-zero then this function MUST be called before
 * rendering any primitive.  Basically, function pointers and miscellaneous
 * flags are updated to reflect the current state of the state machine.
 */
void gl_update_state( GLcontext *ctx )
{
   if (ctx->NewState & NEW_RASTER_OPS) {
      update_pixel_logic(ctx);
      update_pixel_masking(ctx);
      update_fog_mode(ctx);
      update_rasterflags(ctx);
      if (ctx->Driver.Dither) {
         (*ctx->Driver.Dither)( ctx, ctx->Color.DitherFlag );
      }
   }

   if (ctx->NewState & (NEW_RASTER_OPS | NEW_LIGHTING)) {
      update_clipmask(ctx);
   }

   if (ctx->NewState & NEW_LIGHTING) {
      gl_update_lighting(ctx);
      gl_set_color_function(ctx);
   }

   if (ctx->NewState & NEW_TEXTURING) {
      gl_update_texture_state(ctx);
   }

   if (ctx->NewState & (NEW_LIGHTING | NEW_TEXTURING)) {
      /* Check if normal vectors are needed */
      GLboolean sphereGen = GL_FALSE;
      if (ctx->Texture.Enabled) {
         GLuint texSet;
         for (texSet=0; texSet<MAX_TEX_SETS; texSet++) {
            if ((ctx->Texture.Set[texSet].GenModeS==GL_SPHERE_MAP
                 && (ctx->Texture.Set[texSet].TexGenEnabled & S_BIT))
                || (ctx->Texture.Set[texSet].GenModeT==GL_SPHERE_MAP
                    && (ctx->Texture.Set[texSet].TexGenEnabled & T_BIT)))
               sphereGen = GL_TRUE;
         }
      }
      if (ctx->Light.Enabled || sphereGen) {
         ctx->NeedNormals = GL_TRUE;
      }
      else {
         ctx->NeedNormals = GL_FALSE;
      }
   }

   if (ctx->NewState & NEW_RASTER_OPS) {
      /* Check if incoming colors can be modified during rasterization */
      if (ctx->Fog.Enabled ||
          ctx->Texture.Enabled ||
          ctx->Color.BlendEnabled ||
          ctx->Color.SWmasking ||
          ctx->Color.SWLogicOpEnabled) {
         ctx->MutablePixels = GL_TRUE;
      }
      else {
         ctx->MutablePixels = GL_FALSE;
      }
   }

   if (ctx->NewState & (NEW_RASTER_OPS | NEW_LIGHTING)) {
      /* Check if all pixels generated are likely to be the same color */
      if (ctx->Light.ShadeModel==GL_SMOOTH ||
          ctx->Light.Enabled ||
          ctx->Fog.Enabled ||
          ctx->Texture.Enabled ||
          ctx->Color.BlendEnabled ||
          ctx->Color.SWmasking ||
          ctx->Color.SWLogicOpEnabled) {
         ctx->MonoPixels = GL_FALSE;       /* pixels probably multicolored */
      }
      else {
         /* pixels will all be same color,
          * only glColor() can invalidate this.
          */
         ctx->MonoPixels = GL_TRUE;
      }
   }

   if (ctx->NewState & NEW_POLYGON) {
      /* Setup CullBits bitmask */
      ctx->Polygon.CullBits = 0;
      if (ctx->Polygon.CullFlag) {
         if (ctx->Polygon.CullFaceMode==GL_FRONT ||
             ctx->Polygon.CullFaceMode==GL_FRONT_AND_BACK) {
            ctx->Polygon.CullBits |= 1;
         }
         if (ctx->Polygon.CullFaceMode==GL_BACK ||
             ctx->Polygon.CullFaceMode==GL_FRONT_AND_BACK) {
            ctx->Polygon.CullBits |= 2;
         }
      }
      /* Any Polygon offsets enabled? */
      ctx->Polygon.OffsetAny = ctx->Polygon.OffsetPoint ||
                               ctx->Polygon.OffsetLine ||
                               ctx->Polygon.OffsetFill;
      /* reset Z offsets now */
      ctx->PointZoffset   = 0.0;
      ctx->LineZoffset    = 0.0;
      ctx->PolygonZoffset = 0.0;
   }

   if (ctx->NewState & (NEW_POLYGON | NEW_LIGHTING)) {
      /* Determine if we can directly call the triangle rasterizer */
      if (   ctx->Polygon.Unfilled
          || ctx->Polygon.OffsetAny
          || ctx->Polygon.CullFlag
          || ctx->Light.Model.TwoSide
          || ctx->RenderMode!=GL_RENDER) {
         ctx->DirectTriangles = GL_FALSE;
      }
      else {
         ctx->DirectTriangles = GL_TRUE;
      }
   }

   /* update scissor region */
   ctx->Buffer->Xmin = 0;
   ctx->Buffer->Ymin = 0;
   ctx->Buffer->Xmax = ctx->Buffer->Width-1;
   ctx->Buffer->Ymax = ctx->Buffer->Height-1;
   if (ctx->Scissor.Enabled) {
      if (ctx->Scissor.X > ctx->Buffer->Xmin) {
         ctx->Buffer->Xmin = ctx->Scissor.X;
      }
      if (ctx->Scissor.Y > ctx->Buffer->Ymin) {
         ctx->Buffer->Ymin = ctx->Scissor.Y;
      }
      if (ctx->Scissor.X + ctx->Scissor.Width - 1 < ctx->Buffer->Xmax) {
         ctx->Buffer->Xmax = ctx->Scissor.X + ctx->Scissor.Width - 1;
      }
      if (ctx->Scissor.Y + ctx->Scissor.Height - 1 < ctx->Buffer->Ymax) {
         ctx->Buffer->Ymax = ctx->Scissor.Y + ctx->Scissor.Height - 1;
      }
   }

   /*
    * Update Device Driver interface
    */
   if (ctx->NewState & NEW_RASTER_OPS) {
      ctx->Driver.AllocDepthBuffer = gl_alloc_depth_buffer;
      if (ctx->Depth.Mask) {
         switch (ctx->Depth.Func) {
            case GL_LESS:
               ctx->Driver.DepthTestSpan = gl_depth_test_span_less;
               ctx->Driver.DepthTestPixels = gl_depth_test_pixels_less;
               break;
            case GL_GREATER:
               ctx->Driver.DepthTestSpan = gl_depth_test_span_greater;
               ctx->Driver.DepthTestPixels = gl_depth_test_pixels_greater;
               break;
            default:
               ctx->Driver.DepthTestSpan = gl_depth_test_span_generic;
               ctx->Driver.DepthTestPixels = gl_depth_test_pixels_generic;
         }
      }
      else {
         ctx->Driver.DepthTestSpan = gl_depth_test_span_generic;
         ctx->Driver.DepthTestPixels = gl_depth_test_pixels_generic;
      }
      ctx->Driver.ReadDepthSpanFloat = gl_read_depth_span_float;
      ctx->Driver.ReadDepthSpanInt = gl_read_depth_span_int;
   }

   ctx->Driver.PointsFunc = NULL;
   ctx->Driver.LineFunc = NULL;
   ctx->Driver.TriangleFunc = NULL;
   ctx->Driver.QuadFunc = NULL;
   ctx->Driver.RectFunc = NULL;

   /*
    * The ctx->Driver.UpdateState pointer _MUST_ be valid at this point
    * in order for any impending rendering to succeed.
    */
   assert(ctx->Driver.UpdateState);

   /*
    * Here the driver sets up all the ctx->Driver function pointers to
    * it's specific, private functions.
    */
   (*ctx->Driver.UpdateState)(ctx);

   /*
    * In case the driver didn't hook in an optimized point, line or
    * triangle function we'll now select "core/fallback" point, line
    * and triangle functions.
    */
   gl_set_point_function(ctx);
   gl_set_line_function(ctx);
   gl_set_triangle_function(ctx);
   gl_set_quad_function(ctx);

   gl_set_vertex_function(ctx);

   ctx->NewState = 0;
}

