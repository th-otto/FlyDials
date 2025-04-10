/*
	@(#)FlyDial/vvpline.c
	@(#)Julian F. Reschke, 7. September 1991

	Polyline
*/

#include <aes.h>
#include <vdi.h>
#include <flydial\vditool.h>

void
V_PolyLine (VDIWK *V, WORD count, WORD *xy)
{
	v_pline (V->handle, count, xy);
}
