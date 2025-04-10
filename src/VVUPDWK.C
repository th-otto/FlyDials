/*
	@(#)FlyDial/vvupdwk.c
	
	Julian F. Reschke, 12. November 1992
	Workstation aktualisieren
*/

#include <aes.h>
#include <vdi.h>
#include <flydial\vditool.h>

int
V_UpdateWorkstation (VDIWK *V)
{
	v_updwk (V->handle);
	
	if (_VDIParBlk.contrl[4])
		return _VDIParBlk.intout[0];
	else
		return 0;
}
