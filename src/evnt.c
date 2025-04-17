/*
	@(#)FlyDial/evnt.c
	@(#)Julian F. Reschke, 19. Juli 1992
*/

#include <string.h>

#include "flydial/flydial.h"
#include "flydial/evntevnt.h"

#ifdef _BEHNELIB
static AESPB aespb =
{
	_GemParBlk.contrl,
	_GemParBlk.global,
	_GemParBlk.intin,
	_GemParBlk.intout,
	(int *)_GemParBlk.addrin,
	(int *)_GemParBlk.addrout,
};
#else
extern int _AesCtrl (int);
#endif

int evnt_event (MEVENT *pmevent)
{
	memcpy (_GemParBlk.intin, pmevent, sizeof (unsigned int[14]));
	_GemParBlk.addrin[0] = pmevent->e_mepbuf;
	_GemParBlk.intin[14] = (int) ((pmevent->e_time) & 0xFFFF);
	_GemParBlk.intin[15] = (int) ((pmevent->e_time) >> 16);
#ifdef _BEHNELIB
	_GemParBlk.contrl[0] = 25;
	_GemParBlk.contrl[1] = 16;
	_GemParBlk.contrl[2] = 7;
	_GemParBlk.contrl[3] = 1;
	_GemParBlk.contrl[4] = 0;
	_crystal (&aespb);
#else
	_AesCtrl (25);
#endif
	memcpy (&(pmevent->e_mx), &(_GemParBlk.intout[1]), 6 * sizeof (unsigned int));
	return _GemParBlk.intout[0];
}
