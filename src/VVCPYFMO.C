/*
	@(#)FlyDial/vvcpyfmo.c
	@(#)Julian F. Reschke, 7. September 1991

	Copy Raster Form, Opaque
*/

#include <aes.h>
#include <vdi.h>
#include <flydial\vditool.h>

void
V_CopyRasterOpaque (VDIWK *V, WORD mode, WORD *xy, MFDB *src, MFDB *dst)
{
	vro_cpyfm (V->handle, mode, xy, src, dst);
}
