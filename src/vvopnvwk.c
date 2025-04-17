/*
	@(#)FlyDial/vvopnvwk.c
	@(#)Julian F. Reschke, 9. Oktober 1991

	™ffnen einer virtuellen Workstation
*/

#include <stddef.h>
#include "flydial/flydial.h"
#include "flydial/vditool.h"

WORD
V_OpenVirtualWorkstation (VDIWK *V, WORD coord_system)
{
	static WORD work_in[] = {1,1,1,1,1,1,1,1,1,1,2};
	WORD dummy;
	
	V->handle = graf_handle (&dummy, &dummy, &dummy, &dummy);
	work_in[10] = coord_system;
	
	/* Folgende Zeile enth„lt einen Hack zur Effizienzsteigerung */
	
	v_opnvwk (work_in, &(V->handle), &(V->max_x));

	if (!V->handle) return FALSE;

	V->buffer = NULL;
	V->clipping = 0;
	V->has_loaded_fonts = FALSE;
	
	return TRUE;
}
