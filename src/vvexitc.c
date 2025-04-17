/*
	@(#)FlyDial/vvexitc.c
	@(#)Julian F. Reschke, 7. September 1991

	Exit alpha mode
*/

#include "flydial/flydial.h"
#include "flydial/vditool.h"

void
V_ExitAlphaMode (VDIWK *V)
{
	v_exit_cur (V->handle);
}
