/*
	@(#)FlyDial/clrwk.c
	@(#)Julian F. Reschke, 6. Oktober 1991

	Workstation clearen
*/

#include "flydial/flydial.h"
#include "flydial/vditool.h"

void
V_ClearWorkstation (VDIWK *V)
{
	v_clrwk (V->handle);
}
