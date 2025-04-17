/*
	@(#)FlyDial/vventerc.c
	@(#)Julian F. Reschke, 7. September 1991

	Enter alpha mode
*/

#include "flydial/flydial.h"
#include "flydial/vditool.h"

void
V_EnterAlphaMode (VDIWK *V)
{
	v_enter_cur (V->handle);
}
