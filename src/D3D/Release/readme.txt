Mesa 3-D graphics library  DirectX 6 Driver                August 25, 1999
Windows 9x, Windows NT 5.0                                         Build 7
==========================================================================
                                README.TXT
==========================================================================

Contact information:  
        alt.software inc.
        207 Queen's Quay West, Suite 404
        Toronto, Ontario,  M5J 1A7
        Voice  (416) 203-8508
        Technical Support email:  TechSupport@altsoftware.com

	www.altsoftware.com
	Sales@altsoftware.com

Please read all of this file before using this software.
This software is provided free of charge as a courtesy to the OpenGL
community.   Source code will only be available when the next release
of the MESA tree is published.

Contact alt.software for your contract software development projects.

--------------------------------------------------------------------------
INSTALLATION INSTRUCTIONS

Requires DirectX 6 to run.   DirectX 5 is not supported.
This is a release of Mesa-3.0 mapped to DirectX 6. The Direct3D Wrapper
driver was designed to use DirectX 6 to do rasterization for Mesa-3.0.  

The driver chooses between the Primary HAL and HEL at context creation and 
resizing.  If the current HAL can create a rendering surface, with Z-Buffer 
attached, then the HAL will be used.  Textures will be managed by the driver
by creating and destroying surfaces as need be. At the very least the driver 
will fall back to the HEL for rendering if it fails to allocate the required 
resources.  

These instructions assume that the Windows is installed on drive 'C' using 
the default path (c:\windows).  Before installing this software you should
backup your previous version of OpenGL (OGL) if installed.  Simply rename the 
file    c:\windows\system\opengl32.dll to opengl32.dll.bak.

ren c:\windows\system\opengl32.dll to c:\windows\system\opengl32.dll.bak
 
The driver uses one dll (altogl.dll) that must be renamed to
opengl32.dll and copied to your system directory.

copy altogl.dll c:\windows\system\opengl32.dll

REMOVING THE DRIVER/SOFTWARE

del c:\windows\system\opengl32.dll
ren c:\windows\system\opengl32.dll.bak to c:\windows\system\opengl32.dll

---------------------------------------------------------------------------
RUNTIME SWITCHES

MESA_FORCE_SW
MESA_MIPMAP_OFF
MESA_TRIPLE_BUFFER
MESA_AUTO_TEXTURES

SET MESA_FORCE_SW=FALSE
 If this enviroment variable is set to 'TRUE' at the command line or in your
auctoexec.bat then the driver will use software rendering.

SET MESA_MIPMAP_OFF=FALSE
 When set to 'TRUE' then mipmaps will not be used.  This can save texture
memory on some smaller cards.
  
SET MESA_TRIPLE_BUFFER=FALSE
 If set to 'TRUE' the driver will try and create triple buffer support.  The
driver will still fall back to double if this fails.

SET MESA_AUTO_TEXTURES=FALSE
 Set this to 'TRUE' to have Direct3D manage the textures.  This is required
for some cards like the Voodoo3. 	

---------------------------------------------------------------------------
FUTURE PLANS

- add a separate Direct3D vertex buffer to reduce over head of single tris.
- use internal fomrat of textures to bypass pixel pipe when possable.
- added support for Mulit-Textures.
- possible rewrite using Mesa-3.1

---------------------------------------------------------------------------
FIXED IN THIS RELEASE

- interleaved vertex arrays now work
- support for Mipmaps
- square textures work properly

---------------------------------------------------------------------------
FIXED IN BUILD 6

- context create/sizing is much better (Viewperf works!)
- the layer between the Direct3D API and Mesa is smaller.
- displatch tables were created to support fallbacks for unsupported
  blending and texture modes.
- cards that are square texture limited work but are deformed.
- all primitives are hardware rendered (POINTS,LINES,TRIANGLES).
- some minor DPF support was added that can be changed at runtime.
- more optimizations.

--------------------------------------------------------------------------
KNOWN PROBLEMS and Limitations:

- DirectX 5 is not supported, it exits the OpenGL application gracefully.
- Will only work in color modes supported in your Direct3D driver.
- No mipmaps (highest level only).
- No stencil buffer in hardware.
- Choice of DX6 device before the OpenGL program starts not hooked up.
- Uses Microsoft Direct3D API, not directly to the drivers HAL.
- Needs gamma correction.
- Intel i740 may crash with some games.
- Some Alpha channels that are only 1bit won't blend.
- The application 'Bounce' won't resize.

Tested with:

Microsoft DirectX 6
Matrox G200 AGP (all resolutions)
ATI 3D Rage II+  PCI 
ATI 3D RagePro AGP 8M
ATR Rage 128
Voodoo 3
S3 Virge (VX/988)

Quake II / III
Hertic II
Sin Demo
All RedBook samples
All samples that come with the OpenGL Dev CD.
Viewperf
Windows Screen saver

We request testing for the following:
3D Studio MAX 2.x
Windowed applications in general.


-------------------------------------------------
Build 7  August 24, 1999

Feb 5 1999  10:43p               955,392 altogl.dll
Feb 4 1999  11:59a                 5,257 readme.txt


----------------------------------------   End of README.TXT   -----------
