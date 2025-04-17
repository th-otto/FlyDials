/*
	@(#)Flydial/vstarbpt.c
	@(#)Julian F. Reschke, 28. MÑrz 1991

	Binding fÅr vst_arb_pt
*/

#include "flydial/flydial.h"

typedef struct
{
	int	*contrl,*intin,*ptsin,*intout,*ptsout;
} VDIPB;

static VDIPB vdipb =
{
	_GemParBlk.contrl,
	_GemParBlk.intin,
	_GemParBlk.ptsin,
	_GemParBlk.intout,
	_GemParBlk.ptsout
};

extern void cdecl vdi (VDIPB *pb);

int vst_arbpt (int handle, int point, int *char_width, 
	int *char_height, int *cell_width, int *cell_height)
{
	_GemParBlk.intin[0] = point;
	_GemParBlk.contrl[0] = 246;
	_GemParBlk.contrl[2] = 	_GemParBlk.contrl[1] = 0;
	_GemParBlk.contrl[3] = 1;
	_GemParBlk.contrl[6] = handle;
	vdi (&vdipb);
	*char_width = _GemParBlk.ptsout[0];
	*char_height = _GemParBlk.ptsout[1];
	*cell_width = _GemParBlk.ptsout[2];
	*cell_height = _GemParBlk.ptsout[3];

	return _GemParBlk.intout[0];
}
