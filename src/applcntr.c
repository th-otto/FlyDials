/*
	@(#)Flydial/applcntr.c

	Julian F. Reschke, 26. Januar 1997
	Binding fÅr appl_control
*/

#include "flydial/flydial.h"

void 	_aes(int dummy, long parm);

int appl_control(int ap_cid, int ap_cwhat, void *ap_cout)
{
	_GemParBlk.intin[0]  = ap_cid;
	_GemParBlk.intin[1]  = ap_cwhat;
	_GemParBlk.addrin[0] = ap_cout;
	_aes(0, 0x81020101UL);
	return _GemParBlk.intout[0];
}
