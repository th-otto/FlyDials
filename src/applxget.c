/*
	@(#)Flydial/applxget.c

	Julian F. Reschke, 19. M„rz 1995
	Binding fr appl_xgetinfo
*/

#include "flydial/flydial.h"

int
appl_xgetinfo (int type, int *out1, int *out2, int *out3, int *out4)
{
	static short hasagi = -1;

	if (hasagi < 0)
		hasagi = _GemParBlk.global[0] >= 0x400 ||
			appl_find( "?AGI\0\0\0\0") == 0;

	return !hasagi ? 0 : appl_getinfo (type, out1, out2, out3, out4);
}
