/*
	@(#)Flydial/objcsysv.c

	Julian F. Reschke, 26. Januar 1993
	Binding fÅr objc_sysvar
*/

#include "flydial/flydial.h"

static int    contrl[15];
static int    global[15];
static int    intin[132];
static int    intout[140];
static int    addrin[16];
static int    addrout[16];

static AESPB A = { contrl, global, intin, intout, addrin, addrout };

int
objc_sysvar (int mode, int which, int in1, int in2, int *out1,
	int *out2)
{
	contrl[0] = 48;
	contrl[1] = 4;
	contrl[2] = 3;
	contrl[3] = contrl[4] = 0;
	intin[0] = mode;
	intin[1] = which;
	intin[2] = in1;
	intin[3] = in2;
	_crystal (&A);
	*out1 = intout[1];
	*out2 = intout[2];
	return intout[0];
}
