/*
	@(#)FlyDial/vvopnwk.c
	
	Julian F. Reschke, 12. November 1992
	™ffnen einer Workstation
*/

#include <aes.h>
#include <stddef.h>
#include <vdi.h>
#include <flydial\vditool.h>

WORD
V_OpenWorkstation (VDIWK *V, WORD device, WORD coord_system)
{
	static WORD work_in[] = {1,1,1,1,1,1,1,1,1,1,2};
	
	work_in[0] = device;
	work_in[10] = coord_system;
	
	/* Folgende Zeile enth„lt einen Hack zur Effizienzsteigerung */
	
	V->handle = 0;
	v_opnwk (work_in, &(V->handle), &(V->max_x));

	if (!V->handle) return FALSE;

	if ((_VDIParBlk.contrl[0] == 1) && (_VDIParBlk.contrl[1] == 0))
		V->buffer = NULL;
	else
		V->buffer = (void *)((unsigned)_VDIParBlk.contrl[0] * 65536L + 
					(unsigned)_VDIParBlk.contrl[1]);

	V->clipping = 0;
	V->has_loaded_fonts = FALSE;
	
	return TRUE;
}