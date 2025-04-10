/*
	@(#)FlyDial/clrwk.c
	@(#)Julian F. Reschke, 6. Oktober 1991

	Workstation clearen
*/

#include <aes.h>
#include <vdi.h>
#include <flydial\vditool.h>

void
V_ClearWorkstation (VDIWK *V)
{
	v_clrwk (V->handle);
}
