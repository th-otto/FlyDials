/*
	@(#)FlyDial/wk.c
	
	Julian F. Reschke, 1. Mai 1996
	Workstation-Funktionen
*/

#include "flydial/flydial.h"
#include "flydial/wk.h"

void
WkWritingMode (int mode)
{
	static int lastmode = -1;
	
	if (mode != lastmode)
		vswr_mode (DialWk, lastmode = mode);
}

static int lastcolor = -1, lastinterior = -1;
static int lastperimeter = -1, laststyle = -1;

void
WkFillMode (int color, int interior, int perimeter, int style)
{
	if (color != lastcolor)
		vsf_color (DialWk, lastcolor = color);
	if (interior != lastinterior)
		vsf_interior (DialWk, lastinterior = interior);
	if (perimeter != lastperimeter)
		vsf_perimeter (DialWk, lastperimeter = perimeter);
	if (style != laststyle)
		vsf_style (DialWk, laststyle = style);
}

void
WkFillColor (int color)
{
	if (color != lastcolor)
		vsf_color (DialWk, lastcolor = color);
}

void
WkFillInterior (int interior)
{
	if (interior != lastinterior)
		vsf_interior (DialWk, lastinterior = interior);
}

void
WkFillPerimeter (int perimeter)
{
	if (perimeter != lastperimeter)
		vsf_perimeter (DialWk, lastperimeter = perimeter);
}

void
WkFillStyle (int style)
{
	if (style != laststyle)
		vsf_style (DialWk, laststyle = style);
}
