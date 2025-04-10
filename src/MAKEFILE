#
# macros
#
FLAGS = -mv -J -o -r -D__MSDOS__=1

OBJECTS = wind.obj alrt.obj dial.obj form.obj objc.obj jazz.obj \
		  raster.obj rect.obj handle.obj graf.obj imqmark.obj imhand.obj \
		  iminfo.obj imfinger.obj imbomb.obj imprint.obj imdrive.obj \
	      imdisk.obj imexclam.obj imsignqu.obj imsignst.obj imsqques.obj \
		  imsqexcl.obj

#
# builtins
#
.c.obj:
    ztc -c $(FLAGS) $*

#
# targets
#
flydial.lib: $(OBJECTS)
	del flydial.lib
    lib flydial+wind+alrt+dial+form+objc+raster+rect,,flydial
	lib flydial+handle+graf+jazz+imqmark+imhand+iminfo+imfinger+imbomb,,flydial
	lib flydial+imprint+imdrive+imdisk+imexclam+imsignqu+imsignst,,flydial
	lib flydial+imsqques+imsqexcl,,flydial
    copy flydial.lib \zortech\lib

archive:
	del *.bak
	del *.obj
	del *.tmp
	zoo aunP flydial.zoo "*"
	del *.bak

all:
	touch *.c
	make

#
# dependencies
#
DIAL.OBJ : DIAL.C
OBJC.OBJ : OBJC.C
ALRT.OBJ : ALRT.C
GRAF.OBJ : GRAF.C
MENU.OBJ : MENU.C
IMBOMB.OBJ : IMBOMB.C
IMDISK.OBJ : IMDISK.C
IMDRIVE.OBJ : IMDRIVE.C
IMEXCLAM.OBJ : IMEXCLAM.C
IMQMARK.OBJ : IMQMARK.C
IMFINGER.OBJ : IMFINGER.C
IMHAND.OBJ : IMHAND.C
IMINFO.OBJ : IMINFO.C
IMPRINT.OBJ : IMPRINT.C
IMSIGNQU.OBJ : IMSIGNQU.C
IMSIGNST.OBJ : IMSIGNST.C
IMSQEXCL.OBJ : IMSQEXCL.C
IMSQQUES.OBJ : IMSQQUES.C
HANDLE.OBJ : HANDLE.C
RASTER.OBJ : RASTER.C
RECT.OBJ : RECT.C
WIND.OBJ : WIND.C
FORM.OBJ : FORM.C
JAZZ.OBJ : JAZZ.C

