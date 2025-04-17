/*
	@(#)FlyDial/vvsclip.c
	@(#)Julian F. Reschke, 10. Oktober 1991

	™ffnen einer Workstation
*/

#include <string.h>
#include "flydial/flydial.h"
#include "flydial/vditool.h"

void
V_SetClipping (VDIWK *V, WORD clip_flag, WORD *array)
{
	if (!clip_flag)
	{
		if (V->clipping)
		{
			vs_clip (V->handle, 0, array);
			V->clipping = 0;
		}
	}
	else
	{
		if ((!V->clipping) || (memcmp (array, &(V->clip_rect), 4 * sizeof(WORD))))
		{
			V->clipping = 1;
			vs_clip (V->handle, 1, array);
			memcpy (&(V->clip_rect), array, 4 * sizeof(WORD));
		}
	}
}
