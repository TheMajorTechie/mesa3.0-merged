.SILENT:

!INCLUDE <..\mesawin32.mak> # get user directory settings for output

help:
	more < <<
usage: nmake[.exe] /f nmake.mak [options] [target]
where: [target] may be any one of the following ...
    (default)... shows this text
    libfiles.... builds all library variations and test programs
    update...... builds all variations and copies target files
                 to permanent storage locations
    clean....... deletes all intermediate files
    realclean .. deletes all intermediate files and all end-executables,
                 restoring directory heirarchy to 'distribution' state.
    all ........ builds all targets for Mesa-based code
    all.sgigl .. builds all targets for SGI for Windows OpenGL based code
    all.sysgl .. builds all targets for Microsoft OpenGL based code
    ---- build options -----------------
    MESAVER ................. (3.0) - version of MesaLib().tar.gz to build
    MESADEMOVER ............. (3.0) - version of MesaDemos().tar.gz to build
    NOCONSOLE .... [1 | 0] .. (0) ..- Disable console output of progs
    NODEBUG ...... [1 | 0] .. (0) ..- Disable building of Debug images
    NORELEASE .... [1 | 0] .. (0) ..- Disable building of Release images
    USE_MMX ...... [1 | 0] .. (0) ..- Enable use of Intel MMX opcodes
    USE_CTRDLL.... [1 | 0] .. (0) ..- Enable use of MSVCRT.DLL for CRTL in
    USE_SYSGL..... [1 | 0 | 2](0) ..- Enable use of System OpenGL/GLU for
                   GLUT and Demo builds. When set to '2' it will link
                   against SGI OpenGL for Windows instead of Microsoft
                   OopenGL.
    GD_NO_PROGS... [1 | 0] .. (0) ..- Disable diff generation for MesaDemos.
                   preference to statically linking in the CRTL. Makes for
                   smaller files *but* requires MSVCRT.DLL at runtime.
    STATIC_MESA... [1 | 0] .. (0) ..- Meaningful only when building executables.
        Indicates to the nmake.mak that exe files should be linked with
        static link libs (mesa|mesaglu|mesaglut).lib instead of the default
        DLL import libs.  This directive trickes down into a compiler macro
        definition which itself directs the mesa "gl.h" file to redefine
        APIENTRY, CALLBACK, and WINGDIAPI.
    ALLWARNINGS... [1 | 0] .. (0) ..- Enable all compiler warnings, by default
                   the following compile time warnings are disabled:
        C4244: '=' : conversion from 'type1' to 'type2',
               possible loss of data
        C4018: '<' : signed/unsigned mismatch
        C4305: '=' : truncation from 'type1' to 'type2'
        C4013: 'function' undefined; assuming extern returning int
        C4761: integral size mismatch in argument; conversion supplied
        C4273: 'identifier' : inconsistent DLL linkage. dllexport assumed
        C4101: 'identifier' : unreferenced local variable

        ... additionally, if you manually set the MESA_WARNQUIET CPPOPT
            definition greater than 1 then these additional warnings are
            turned off also:

        C4146: unary minus operator applied to unsigned type,
               result still unsigned

    ---- special targets ---------------
    libfiles ........................ all *.lib and *.dll files
    tests ..........(req libfiles)... all test executables
    progs ..........(req libfiles)... all demonstration executables
    progs.samples / progs.demos /
    progs.book / progs.3dfx.demos ... individual demo program group builds
    gendiffs ........................ generates complete diffs between
                   original distribution and locally modified source files.
    gendiffs.clr .................... cleans up behind gendiffs target.

    progs.sysgl ..................... builds all demo programs linking
                                      against GLUT32.DLL and Microsoft
                                      OpenGL link libraries for using
                                      Windows system OpenGL support.

    progs.sgigl ..................... builds all demos programs as progs.sysgl
                                      however it links against GLUT.DLL built
                                      against the SGI OpenGL for Windows.

    ---- convenience targets -----------
    allstatic ....................... builds all "standard" static lib files.
    alldynamic ...................... builds all "standard" dynamic lib files,
                                      excluding hardware accelerator versions.
    allaccel ........................ builds all hardware accelerated versions.
	allfx ........................... builds all 3Dfx targeted code.
	alls3 ........................... builds all S3 S3DTK targeted code.

    ---- individual targets ------------
    mesalg, mesaglu, mesaglut ....... static lib builds of respective code.

	mesagl32 ........................ dynamic lib build of Mesa core

    fxmesagl32 ...................... dynamic lib build for use on top of
                                      3Dfx Glide runtime (for use with
                                      3Dfx Voodoo based display hardware).

    s3mesagl32 ...................... dynamic lib build for use on top of
                                      S3 'S3Tk' runtime (for use with
                                      S3 Virge based display hardware).

	glu32 ........................... dynamic lib build of GLU routines

	glut32 .......................... dynamic lib build of GLUT routines

    glut.sysgl ...................... alias lib build of glut32 using system
                                      OpenGL/GLU routines.

    glut.sysgl.install .............. installs generated GLUT and GLUT.H into
                                      directores specified at end of this
                                      help screen.

    glut.sgigl ...................... alias lib build of glut32 using SGI
                                      OopenGL for Windows GL/GLU routines

    glut.sgigl.install .............. installs generated GLUT and GLUT.H into
                                      directores specified at end of this
                                      help screen.


    ---- makefile Author notes ------------------
    Mesa originated as a project of and is currently maintained by Brian Paul
    (brianp@elastic.avid.com). Please look the the "canonical" internet sites
    for detailed information and source distributions, either at:

        http://www.ssec.wisc.edu/~brianp/Mesa.html

        <or>

        ftp://iris.ssec.wisc.edu/pub/Mesa

    This makefile suite built against MS Visual C++ 5.0 SP3 and tested on
    WindowsNT 4.0 SP3, WindowsNT 5.0b1, Windows95 OSR 2.0, and Windows98.

    ---- change history -------------------------
    6/25/98 - taj - modified to support Mesa 3.0beta5, all core libs
         have been validated to build, with the exception of s3mesagl32.dll
         which does not at this time build with the current S3Tk headers.
         Big Todo: verify building of all demo code
    2/6/98  - taj - added support for MSVC 4 build (required addition to
         nmake.mif file and sensing of _NMAKE_VER macro).
    2/2/98  - taj - corrected "alldlls" target to be "alldynamic"
         80 column text display cleanup of nmake.mak
    1/18/98 - taj - checked with Mesa 2.6beta4 release, added S3MESA.DLL
         build files. Note that I can not vouch for the usability of
         S3MESA.DLL as I have no Virge-based hardware to test it upon.

    ---- output directories as configured -------

    Linking Libs . : $(LIBINSTALL)
    Runtime Libs . : $(DLLINSTALL)
    Header Files . : $(HDRINSTALL)

<<

# ----- Default / primary configuration stuff ---------------------------------

# Makefile elections, may be selected by command line but here are the defaults
#
# MESAVER     - Mesa version
# NOCONSOLE   - when 1, builds executables for Windowed runtime
#

!IF "$(NOCONSOLE)" == ""
NOCONSOLE=0
!ENDIF

!IF "$(NOCONSOLE)" == "1"
DOGUI=1
!ELSE
DOGUI=0
!ENDIF

!IF "$(MESAVER)" == ""
MESAVER=3.0
!ENDIF

# For Ted's use in local builds ...
#
!IF EXIST (..\..\Mesa-3.0tj\readme.win32)
MESAVER=$(MESAVER)tj
!ENDIF

# used in building all of the resource files for the Mesa DLLs
#
!IF "$(MESAFILEVER)" == ""
MESAFILEVER=3,0,0,0
!ENDIF

# Currently, MesaDemos are at rev 3.0 ..
#
!IF "$(MESADEMOVER)" == ""
MESADEMOVER=3.0
!ENDIF

# For Ted's use in local builds ...
#
!IF EXIST (..\..\Mesa-3.0tj\samples\makefile)
MESADEMOVER=$(MESADEMOVER)tj
!ENDIF

# used in building all of the resource files for the Auxillary DLLs
#
!IF "$(MESADEMOFILEVER)" == ""
MESADEMOFILEVER=3.0,0,0
!ENDIF

# Set to 1 to enable building with MMX instructions on VC5
#
!IF "$(USE_MMX)" == ""
USE_MMX=0
!ENDIF

# Set to 1 to enable building against MSVCRT.DLL instead of LIBC.LIB
#
!IF "$(USE_CRTDLL)" == ""
USE_CRTDLL=0
!ENDIF

!IF "$(GD_NO_PROGS)" == ""
GD_NO_PROGS=0
!ENDIF

!IF "$(USE_SYSGL)" == ""
USE_SYSGL=0
!ENDIF

#---------------------------------------------------------------------
# shouldn't need to play beyond here ...
#---------------------------------------------------------------------

!IF "$(LIBBASE)" == ""
LIBBASE=mesagl
!ENDIF

!IF "$(DLLBASE)" == ""
DLLBASE=$(LIBBASE)
!ENDIF

!IF "$(EXEFSERIES_UP)" == ""
EXEFSERIES_UP=1
!ENDIF

!IF "$(EXEFILE)" == ""
EXEFILE=-
!ENDIF

!IF "$(ALLWARNINGS)" == ""
ALLWARNINGS=0
!ENDIF

!IF "$(STATIC_MESA)" == ""
STATIC_MESA=0
!ENDIF

!IF "$(DEBUG)" == ""
DEBUG=0
!ENDIF

!IF "$(NODEBUG)" == ""
!IF "$(DEBUG)" == "1"
NODEBUG=0
!ELSE
NODEBUG=1
!ENDIF
!ENDIF

!IF "$(NORELEASE)" == ""
!IF "$(DEBUG)" == "1"
NORELEASE=1
!ELSE
NORELEASE=0
!ENDIF
!ENDIF

!IF (("$(DEBUG)" == "1") && ("$(USE_CRTDLL)" == "1"))
LIBTYPE=d-d
NORELEASE=1
NOSTATIC=1
!ELSEIF (("$(DEBUG)" == "1") && ("$(USE_CRTDLL)" == "0"))
LIBTYPE=d-s
NORELEASE=1
NODYNAMIC=1
!ELSEIF (("$(DEBUG)" == "0") && ("$(USE_CRTDLL)" == "1"))
LIBTYPE=r-d
NODEBUG=1
NOSTATIC=1
!ELSE
LIBTYPE=r-s
NODEBUG=1
NODYNAMIC=1
DEBUG=0
!ENDIF

# See nmake.mif file for documentations of these settings
NOPCH=1
H_FILES=

# Root source directory for primary Mesa lib files, check first that we are
# "in" the mesa source tree, then go extreme after that.
!IF EXIST (..\src\Makefile)
MESAROOT=..
!ELSEIF EXIST (..\..\mesa-$(MESAVER)\src\Makefile)
MESAROOT=..\..\mesa-$(MESAVER)
!ELSE
MESAROOT=..\mesa-$(MESAVER)
!ENDIF

# Root source directory for Demos/etc - may be alternative directory
!IF EXIST (..\samples\Makefile)
MESADEMOROOT=..
!ELSEIF EXIST (..\..\mesa-$(MESADEMOVER)\samples\Makefile)
MESADEMOROOT=..\..\mesa-$(MESADEMOVER)
!ELSE
MESADEMOROOT=..\mesa-$(MESADEMOVER)
!ENDIF

!IF "$(DEBUG)" == "1"
DEBTAG=d
!ELSE
DEBTAG=
!ENDIF

OBJDIR_R=.\$(MESAROOT)\lib
ALTLIBPATH=$(OBJDIR_R)

# ----- Standard meta build rules ---------------------------------------------

all: mesagl mesaglu mesaglut\
	mesagl32 glu32 glut32\
	fxmesagl32\
	progs

all.sysgl: glut.sysgl progs.sysgl

all.sgigl: glut.sgigl progs.sgigl

!IF EXIST(.\include\gl\glut.h)
CLEANSUPS = .\include
!ENDIF

realclean: cleanprogs
	echo. > ..\3dfx\demos\foo.dll
	echo. > ..\book\foo.dll
	echo. > ..\demos\foo.dll
	echo. > ..\samples\foo.dll
	echo. > ..\3dfx\demos\foo.pdb
	echo. > ..\book\foo.pdb
	echo. > ..\demos\foo.pdb
	echo. > ..\samples\foo.pdb
	del ..\3dfx\demos\*.dll
	del ..\demos\*.dll
	del ..\samples\*.dll
	del ..\book\*.dll
	del ..\3dfx\demos\*.pdb
	del ..\demos\*.pdb
	del ..\samples\*.pdb
	del ..\book\*.pdb
!IF EXIST($(OBJDIR_R))
	nmake /nologo /f nmake.mak CLEANSUPS=$(OBJDIR_R) clean
!ELSE
	nmake /nologo /f nmake.mak clean
!ENDIF

clean: cleanroot

cleanprogs:
	echo Clearing DEBUG build files ...
	nmake /nologo /f nmake.mak DEBUG=1 EXEFILE=progs.3dfx.demos clrxfileseries
	nmake /nologo /f nmake.mak DEBUG=1 EXEFILE=progs.book clrxfileseries
	nmake /nologo /f nmake.mak DEBUG=1 EXEFILE=progs.demos clrxfileseries
	nmake /nologo /f nmake.mak DEBUG=1 EXEFILE=progs.samples clrxfileseries
	echo Clearing RELEASE build files ...
	nmake /nologo /f nmake.mak DEBUG=0 EXEFILE=progs.3dfx.demos clrxfileseries
	nmake /nologo /f nmake.mak DEBUG=0 EXEFILE=progs.book clrxfileseries
	nmake /nologo /f nmake.mak DEBUG=0 EXEFILE=progs.demos clrxfileseries
	nmake /nologo /f nmake.mak DEBUG=0 EXEFILE=progs.samples clrxfileseries

# ----- link libraries build rules --------------------------------------------

STATICLIBLIST=mesagl.lib mesaglu.lib mesaglut.lib

DYNAMICLIBLIST=mesagl32.dll glu32.dll glut32.dll

$(STATICLIBLIST) $(STATICLIBLIST:.lib= ):
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(MESAROOT)\lib directory
	set STATIC_MESA=1
	set USE_CRTDLL=0
	set USE_MMX=$(USE_MMX)
	set DEBUG=$(DEBUG)
	nmake /nologo /f nmake.mak EXEFILE=- LIBBASE=$* libupr

$(DYNAMICLIBLIST) $(DYNAMICLIBLIST:.dll= ):
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(MESAROOT)\lib directory
	set STATIC_MESA=0
	set USE_CRTDLL=1
	set USE_MMX=$(USE_MMX)
	set DEBUG=$(DEBUG)
	nmake /nologo /f nmake.mak EXEFILE=- LIBBASE=$* dllupr

glut.sysgl:
	nmake /nologo /f nmake.mak USE_SYSGL=1 DEBUG=1 glut32
	nmake /nologo /f nmake.mak USE_SYSGL=1 DEBUG=0 glut32

glut.sysgl.install:
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(HDRINSTALL)\GL directory
	rem <<$(TEMP)\gsi.mak
.SILENT:

# Linking Libs . : $(LIBINSTALL)
# Runtime Libs . : $(DLLINSTALL)
# Header Files . : $(HDRINSTALL)\GL

install: $(LIBINSTALL)\GLUT32.LIB $(LIBINSTALL)\GLUT32d.LIB\
	$(DLLINSTALL)\GLUT32.DLL $(DLLINSTALL)\GLUT32d.DLL\
	$(DLLINSTALL)\GLUT32.PDB $(DLLINSTALL)\GLUT32d.PDB

$(LIBINSTALL)\GLUT32.LIB: .\Release\GLUT32.LIB
	echo $(LIBINSTALL)\GLUT32.LIB ...
	copy .\Release\GLUT32.LIB $(LIBINSTALL)\GLUT32.LIB

$(LIBINSTALL)\GLUT32d.LIB: .\Debug\GLUT32d.LIB
	echo $(LIBINSTALL)\GLUT32d.LIB ...
	copy .\Debug\GLUT32d.LIB $(LIBINSTALL)\GLUT32d.LIB

$(DLLINSTALL)\GLUT32.DLL: .\Release\GLUT32.DLL
	echo $(LIBINSTALL)\GLUT32.DLL ...
	copy .\Release\GLUT32.DLL $(DLLINSTALL)\GLUT32.DLL

$(DLLINSTALL)\GLUT32d.DLL: .\Debug\GLUT32d.DLL
	echo $(DLLINSTALL)\GLUT32d.DLL ...
	copy .\Debug\GLUT32d.DLL $(DLLINSTALL)\GLUT32d.DLL

$(DLLINSTALL)\GLUT32.PDB: .\Release\GLUT32.PDB
	echo $(LIBINSTALL)\GLUT32.PDB ...
	copy .\Release\GLUT32.PDB $(DLLINSTALL)\GLUT32.PDB

$(DLLINSTALL)\GLUT32d.PDB: .\Debug\GLUT32d.PDB
	echo $(DLLINSTALL)\GLUT32d.PDB ...
	copy .\Debug\GLUT32d.PDB $(DLLINSTALL)\GLUT32d.PDB

$(HDRINSTALL)\GL\GLUT.H: $(MESAROOT)\INCLUDE\GL\GLUT.H
	echo $(HDRINSTALL)\GL\GLUT.H ...
	copy $(MESAROOT)\INCLUDE\GL\GLUT.H $(HDRINSTALL)\GL\GLUT.H
<<KEEP
	nmake /nologo /f $(TEMP)\gsi.mak install

glut.sgigl:
	nmake /nologo /f nmake.mak USE_SYSGL=2 DEBUG=1 glut32
	nmake /nologo /f nmake.mak USE_SYSGL=2 DEBUG=0 glut32

glut.sgigl.install:
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(HDRINSTALL)\GL directory
	rem <<$(TEMP)\gsi.mak
.SILENT:

# Linking Libs . : $(LIBINSTALL)
# Runtime Libs . : $(DLLINSTALL)
# Header Files . : $(HDRINSTALL)\GL

install: $(LIBINSTALL)\GLUT.LIB $(LIBINSTALL)\GLUTd.LIB\
	$(DLLINSTALL)\GLUT.DLL $(DLLINSTALL)\GLUTd.DLL\
	$(DLLINSTALL)\GLUT.PDB $(DLLINSTALL)\GLUTd.PDB

$(LIBINSTALL)\GLUT.LIB: .\Release\GLUT.LIB
	echo $(LIBINSTALL)\GLUT.LIB ...
	copy .\Release\GLUT.LIB $(LIBINSTALL)\GLUT.LIB

$(LIBINSTALL)\GLUTd.LIB: .\Debug\GLUTd.LIB
	echo $(LIBINSTALL)\GLUTd.LIB ...
	copy .\Debug\GLUTd.LIB $(LIBINSTALL)\GLUTd.LIB

$(DLLINSTALL)\GLUT.DLL: .\Release\GLUT.DLL
	echo $(LIBINSTALL)\GLUT.DLL ...
	copy .\Release\GLUT.DLL $(DLLINSTALL)\GLUT.DLL

$(DLLINSTALL)\GLUTd.DLL: .\Debug\GLUTd.DLL
	echo $(DLLINSTALL)\GLUTd.DLL ...
	copy .\Debug\GLUTd.DLL $(DLLINSTALL)\GLUTd.DLL

$(DLLINSTALL)\GLUT.PDB: .\Release\GLUT.PDB
	echo $(LIBINSTALL)\GLUT.PDB ...
	copy .\Release\GLUT.PDB $(DLLINSTALL)\GLUT.PDB

$(DLLINSTALL)\GLUTd.PDB: .\Debug\GLUTd.PDB
	echo $(DLLINSTALL)\GLUTd.PDB ...
	copy .\Debug\GLUTd.PDB $(DLLINSTALL)\GLUTd.PDB

$(HDRINSTALL)\GL\GLUT.H: $(MESAROOT)\INCLUDE\GL\GLUT.H
	echo $(HDRINSTALL)\GL\GLUT.H ...
	copy $(MESAROOT)\INCLUDE\GL\GLUT.H $(HDRINSTALL)\GL\GLUT.H
<<KEEP
	nmake /nologo /f $(TEMP)\gsi.mak install

update: headers $(STATICLIBLIST:.lib=.up) $(DYNAMICLIBLIST:.dll=.up)

$(STATICLIBLIST:.lib=.up):
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(MESAROOT)\lib directory
	set STATIC_MESA=1
	set USE_CRTDLL=0
	set USE_MMX=$(USE_MMX)
	set DEBUG=$(DEBUG)
	nmake /nologo /f nmake.mak EXEFILE=- LIBBASE=$* libup

$(STATICLIBLIST:.lib=.db):
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(MESAROOT)\lib directory
	set NORELEASE=1
	set STATIC_MESA=1
	set USE_CRTDLL=0
	set USE_MMX=$(USE_MMX)
	set DEBUG=1
	nmake /nologo /f nmake.mak $*.lib

$(DYNAMICLIBLIST:.dll=.up):
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(MESAROOT)\lib directory
	set STATIC_MESA=0
	set USE_CRTDLL=1
	set USE_MMX=$(USE_MMX)
	set DEBUG=$(DEBUG)
	nmake /nologo /f nmake.mak EXEFILE=- LIBBASE=$* dllup

$(DYNAMICLIBLIST:.dll=.db):
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(MESAROOT)\lib directory
	set NORELEASE=1
	set STATIC_MESA=0
	set USE_CRTDLL=1
	set USE_MMX=$(USE_MMX)
	set DEBUG=1
	nmake /nologo /f nmake.mak $*.dll

$(DYNAMICLIBLIST:.dll=.status):
	set STATIC_MESA=0
	set USE_CRTDLL=1
	set DEBUG=$(DEBUG)
	set USE_MMX=$(USE_MMX)
	nmake /nologo /f nmake.mak EXEFILE=- LIBBASE=$* status

$(STATICLIBLIST:.lib=.status):
	set STATIC_MESA=1
	set USE_CRTDLL=0
	set DEBUG=$(DEBUG)
	set USE_MMX=$(USE_MMX)
	nmake /nologo /f nmake.mak EXEFILE=- LIBBASE=$* status

$(DYNAMICLIBLIST:.dll=.resources):
	set STATIC_MESA=0
	set DEBUG=$(DEBUG)
	set USE_CRTDLL=1
	set USE_MMX=$(USE_MMX)
	nmake /nologo /f nmake.mak EXEFILE=- LIBBASE=$* resources

libfiles: $(STATICLIBLIST:.lib= ) $(DYNAMICLIBLIST:.dll= )

allstatic: mesa mesaglu mesaglut
	set STATIC_MESA=1
	set USE_CRTDLL=0
	set DEBUG=$(DEBUG)
	set USE_MMX=$(USE_MMX)
	nmake /nologo /f nmake.mak progs.book progs.samples progs.demos

alldynamic: mesagl32 glu32 glut32 fxmesagl32 s3mesagl32
	set STATIC_MESA=0
	set USE_CRTDLL=0
	set DEBUG=$(DEBUG)
	set USE_MMX=$(USE_MMX)
	nmake /nologo /f nmake.mak progs.book progs.samples progs.demos

allaccel: allfx alls3

allfx: fxmesagl32 glu32 glut32 progs.3dfx.demos

alls3: s3msagl32

fxmesagl32:
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(MESAROOT)\lib\FX directory
	set STATIC_MESA=0
	set USE_CRTDLL=1
	set USE_MMX=$(USE_MMX)
	set DEBUG=$(DEBUG)
	nmake /nologo /f nmake.mak EXEFILE=- LIBBASE=$* dllupr

s3mesagl32:
	nmake /nologo /f nmake.mak DIRECTORYNAME=$(MESAROOT)\lib\S3 directory
	set STATIC_MESA=0
	set USE_CRTDLL=1
	set USE_MMX=$(USE_MMX)
	set DEBUG=$(DEBUG)
	nmake /nologo /f nmake.mak EXEFILE=- LIBBASE=$* dllupr

# ----- convenient aliases ----------------------------------------------------

opengl: mesagl32

glu: glu32

glut: glut32

fxmesagl: fxmesagl32

s3mesagl: s3mesagl32

# ----- test/demo program build ruiles ----------------------------------------
# NOTE: These require previous builds of the mesa libs, et. al., but are not
# made dependent upon their presence to facilitate SPEED of nmake processing.

PROGSLIST    = progs.book.p progs.demos.p progs.samples.p
PROGSLISTGL  = $(PROGSLIST:.p=.sysgl)
PROGSLISTGL2 = $(PROGSLIST:.p=.sgigl)
PROGSLISTST  = $(PROGSLIST:.p=.status) progs.3dfx.demos.status
PROGSLISTM   = $(PROGSLIST:.p= ) progs.3dfx.demos

progs: $(PROGSLISTM)

progs.sysgl: $(PROGSLISTGL)

progs.sgigl: $(PROGSLISTGL2)

$(PROGSLISTM):
	set STATIC_MESA=$(STATIC_MESA)
	set DEBUG=$(DEBUG)
	set USE_CRTDLL=1
	set USE_MMX=$(USE_MMX)
	nmake /nologo /f nmake.mak EXEFILE=$@ exefileseries libcopy

$(PROGSLISTST):
	set STATIC_MESA=$(STATIC_MESA)
	set DEBUG=$(DEBUG)
	set USE_CRTDLL=1
	set USE_MMX=$(USE_MMX)
	nmake /nologo /f nmake.mak EXEFILE=$@ status

$(PROGSLISTGL):
	set USE_SYSGL=1
	nmake /nologo /f nmake.mak $*

$(PROGSLISTGL2):
	set USE_SYSGL=2
	nmake /nologo /f nmake.mak $*

# ----- program/lib specific options ------------------------------------------

include\gl\glut.h: ..\include\gl\glut.h
	echo include\GL\glut.h ...
	nmake /nologo /f nmake.mak DIRECTORYNAME=include\GL directory
	copy ..\include\GL\glut.h include\GL\glut.h

CPPOPTS=/D__MSC__ /D__WIN32__ /DWIN32 /DWINDOWS_NT /DNO_PARALLEL /DNO_STEREO

!IF "$(STATIC_MESA)" == "1"
CPPOPTS=/D_STATIC_MESA $(CPPOPTS)
!ENDIF

!IF "$(ALLWARNINGS)" != "1"
CPPOPTS=$(CPPOPTS) /DMESA_MINWARN
!ENDIF

# -----------------------------------------------------------------------------
!IF "$(EXEFILE)" == "-"  # building a lib of some form ...
# -----------------------------------------------------------------------------

!IF ("$(DLLBASE)" != "glut32")
CPPOPTS=$(CPPOPTS) /I$(MESAROOT)\include
!ENDIF

INTDIRSFX = $(LIBBASE)
SRCALT1=.\$(MESAROOT)\src
!INCLUDE .\rules\lib.$(LIBBASE)

# -----------------------------------------------------------------------------
!ELSEIF "$(EXEFILE)" != "" # building a set of executables ...
# -----------------------------------------------------------------------------

INTDIRSFX=$(EXEFILE)
SUPLIBSROOT=.\rules\$(EXEFILE)

!IF "$(USE_SYSGL)" != "0"
CPPDEPS=include\GL\glut.h
CPPOPTS=$(CPPOPTS) /I.\include
!ELSE
CPPOPTS=$(CPPOPTS) /I$(MESAROOT)\include
!ENDIF

!IF EXIST (.\rules\$(EXEFILE))
!INCLUDE .\rules\$(EXEFILE)
!ENDIF

# Use possibly redefined SRCALT1 here to copy executables
# to their respective source directories ...

BINDIR       = $(SRCALT1)

libcopy:
	rem <<$(TEMP)\libcopyf.mak
.SILENT:

$$(TGTFILE): $$(SRCFILE)
	echo $$(SRCFILE) ...
	set COPYCMD=/y
	copy /v $$(SRCFILE) $$(TGTFILE)

copyfile: $$(TGTFILE)
<<KEEP
	<<$(TEMP)\libcopyf.bat $(OBJDIR_R) $(SRCALT1) $(RUNTIMELIBS)
@echo off
set _SRCDIR=%1
shift
set _TGTDIR=%1
shift
:loops
nmake /nologo /f $(TEMP)\libcopyf.mak SRCFILE=%_SRCDIR%\%1 TGTFILE=%_TGTDIR% copyfile
shift
if not [%1] == [] goto loops
set _SRCDIR=
set _TGTDIR=
<<KEEP

# -----------------------------------------------------------------------------
!ENDIF # EXEFILE keyed selection
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# ----- get final build rules -------------------------------------------------
# -----------------------------------------------------------------------------
!IF EXIST (nmake.mif)
!INCLUDE nmake.mif
!ELSE
!INCLUDE <nmake.mif>
!ENDIF
# -----------------------------------------------------------------------------
