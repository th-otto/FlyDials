/*
	@(#)Flydial/vqtdevin.c
	@(#)Julian F. Reschke, 11. Oktober 1991

	Binding fÅr vqt_devinfo
*/

#include <aes.h>
#include <vdi.h>

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

void vqt_devinfo (int handle, int devnum, int *devexists, char *devstr)
{
	int i;

	_GemParBlk.contrl[6] = handle;
	_GemParBlk.intin[0] = devnum;
	_GemParBlk.contrl[0] = 248;
	_GemParBlk.contrl[1] = 	_GemParBlk.contrl[2] = 0;
	_GemParBlk.contrl[3] = 1;
	vdi (&vdipb);
	*devexists = _GemParBlk.ptsout[0];

	if (_GemParBlk.contrl[2] != 1)
	{
		*devexists = 0;
	}
	else if (*devexists)
	{
		for (i = 0; i <=  _GemParBlk.contrl[4]; i++)
		{
			devstr[i] = (char)_GemParBlk.intout[i];
			devstr[i+1] = 0;
		}
	}
		

}
