# $Id: Makefile,v 3.5 1998/07/26 17:52:48 brianp Exp $

# Mesa 3-D graphics library
# Version:  3.0
# Copyright (C) 1995-1998  Brian Paul
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


# Makefile for core library


# $Log: Makefile,v $
# Revision 3.5  1998/07/26 17:52:48  brianp
# added rule for ggimesa.o
#
# Revision 3.4  1998/07/09 03:16:10  brianp
# added GGI driver
#
# Revision 3.3  1998/06/19 03:15:53  brianp
# added mthreads.c to CORE_SOURCES
#
# Revision 3.2  1998/06/10 02:16:00  brianp
# added FX/fxddspan.c to sources, part of v0.27 of 3Dfx driver
#
# Revision 3.1  1998/03/10 01:27:13  brianp
# updated for David's v0.23 fxmesa driver
#
# Revision 3.0  1998/02/14 18:03:27  brianp
# initial rev
#



##### MACROS #####

VPATH = RCS

INCDIR = ../include
LIBDIR = ../lib

CORE_SOURCES = accum.c alpha.c alphabuf.c api1.c api2.c apiext.c attrib.c \
	bitmap.c blend.c clip.c colortab.c context.c copypix.c depth.c \
	dlist.c drawpix.c enable.c eval.c feedback.c fog.c \
	get.c hash.c image.c light.c lines.c logic.c masking.c matrix.c \
	misc.c mmath.c mthreads.c pb.c pixel.c points.c pointers.c polygon.c \
	quads.c rastpos.c readpix.c rect.c scissor.c shade.c span.c \
	stencil.c teximage.c texobj.c texstate.c texture.c triangle.c \
	varray.c winpos.c vb.c vbfill.c vbrender.c vbxform.c xform.c \
	zoom.c

DRIVER_SOURCES = X/glxapi.c X/fakeglx.c X/realglx.c X/xfonts.c \
	X/xmesa1.c X/xmesa2.c X/xmesa3.c X/xmesa4.c \
	OSmesa/osmesa.c \
	SVGA/svgamesa.c \
	FX/fxapi.c FX/fxdd.c FX/fxddtex.c FX/fxvsetup.c FX/fxsetup.c \
	FX/fxpoints.c FX/fxlines.c FX/fxtris.c FX/fxquads.c \
	FX/fxrender.c FX/fxtexman.c FX/fxddspan.c \
	GGI/ggimesa.c

ASM_SOURCES = 


OBJECTS = $(ASM_SOURCES:.S=.o) $(CORE_SOURCES:.c=.o) $(DRIVER_SOURCES:.c=.o)



##### RULES #####

.c.o:
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@

.S.o:
	$(CC) -c $(CFLAGS) $<

# UGH! These rules shouldn't be needed but IRIX's make (and others?) does!
X/glxapi.o: X/glxapi.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
X/fakeglx.o: X/fakeglx.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
X/realglx.o: X/realglx.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
X/xfonts.o: X/xfonts.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
X/xmesa1.o: X/xmesa1.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
X/xmesa2.o: X/xmesa2.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
X/xmesa3.o: X/xmesa3.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
X/xmesa4.o: X/xmesa4.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
SVGA/svgamesa.o: SVGA/svgamesa.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
OSmesa/osmesa.o: OSmesa/osmesa.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxapi.o: FX/fxapi.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxdd.o: FX/fxdd.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxddtex.o: FX/fxddtex.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxvsetup.o: FX/fxvsetup.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxsetup.o: FX/fxsetup.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxpoints.o: FX/fxpoints.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxlines.o: FX/fxlines.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxtris.o: FX/fxtris.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxquads.o: FX/fxquads.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxrender.o: FX/fxrender.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxtexman.o: FX/fxtexman.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
FX/fxddspan.o: FX/fxddspan.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@
GGI/ggimesa.o: GGI/ggimesa.c
	$(CC) -c -I. -I$(INCDIR) $(CFLAGS) $< -o $@



##### TARGETS #####

default:
	@echo "Specify a target configuration"

clean:
	-rm *.o *~ */*.o */*~

targets: $(LIBDIR)/$(GL_LIB)

# Make the library
$(LIBDIR)/$(GL_LIB): $(OBJECTS)
	$(MAKELIB) $(GL_LIB) $(MAJOR) $(MINOR) $(OBJECTS)
	mv $(GL_LIB)* $(LIBDIR)


include ../Make-config

include depend



#
# Run 'make dep' to update the dependencies if you change what's included
# by any source file.
# 
dep: $(CORE_SOURCES) $(DRIVER_SOURCES)
	makedepend -fdepend -Y -I../include -DGGI -DSVGA -DFX $(CORE_SOURCES) $(DRIVER_SOURCES)
