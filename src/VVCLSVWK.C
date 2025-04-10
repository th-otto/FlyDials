/*
	@(#)FlyDial/clsvwk.c
	@(#)Julian F. Reschke, 7. September 1991

	Schliežen einer virtuellen Workstation
*/

#include <aes.h>
#include <vdi.h>
#include <flydial\vditool.h>

void
V_CloseVirtualWorkstation (VDIWK *V)
{
	v_clsvwk (V->handle);
}
