/*
	@(#)FlyDial/clsvwk.c
	@(#)Julian F. Reschke, 6. Oktober 1991

	Schlieen einer Workstation
*/

#include "flydial/flydial.h"
#include "flydial/vditool.h"

void
V_CloseWorkstation (VDIWK *V)
{
	v_clswk (V->handle);
}
