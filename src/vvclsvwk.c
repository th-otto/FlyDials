/*
	@(#)FlyDial/clsvwk.c
	@(#)Julian F. Reschke, 7. September 1991

	Schlieen einer virtuellen Workstation
*/

#include "flydial/flydial.h"
#include "flydial/vditool.h"

void
V_CloseVirtualWorkstation (VDIWK *V)
{
	v_clsvwk (V->handle);
}
