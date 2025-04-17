/*
	@(#)Flydial\handle.c
	
	Julian F. Reschke, 14. Oktober 1997
*/

#include "flydial/flydial.h"

static char sccsid[] = "@(#)Fliegende Dialoge 0.57, Copyright (c) "
	"Julian F. Reschke, "__DATE__;
			
int HandAES, HandXSize, HandYSize, HandBXSize, HandBYSize;
int HandSFId, HandSFHeight, HandSIId, HandSIHeight;

#if defined(__TOS__) || defined(__atarist__)
int HandFast (void)
{
	int outs[57];

	vq_extnd (HandAES, 1, outs);
	return (outs[6]>2000) + 0 * (int)sccsid;
}
#endif

/*
int HandYText ( void )
{
	static int Ys = -1;
	int HandStdWorkIn[]={1,1,1,1,1,1,1,1,1,1,2};
	int workout[57];
	int handle;

	if (Ys == -1)
	{
		handle = HandAES;
		v_opnvwk (HandStdWorkIn,&handle,workout);
		vqt_attributes (handle,workout);
		Ys = workout[7];
		v_clsvwk (handle);
	}
	return Ys;
}
*/


void
HandScreenSize (int *x, int *y, int *w, int *h)
{
	static int wx, wy, ww = 0, wh;

	if (!ww)
		wind_get (0, WF_WORKXYWH, &wx, &wy, &ww, &wh);
	
	*x = wx; *y = wy; *w = ww; *h = wh;
}

void HandInit (void)
{
	HandAES = graf_handle (&HandXSize, &HandYSize,
		&HandBXSize, &HandBYSize);
}

void HandClip (int x, int y, int w, int h, int flag)
{
	static int mx = -1, my = -1, mw = -1, mh = -1, mflag = -1;

	if (!w) flag = FALSE;

	if ((flag != mflag)||(mx != x)||(my != y)||(mw != w)||(mh != h))
	{
		int xy[4];
		
		mx = x; my = y; mw = w; mh = h; mflag = flag;
		RectAES2VDI (x, y, w, h, xy);
		vs_clip (DialWk, flag, xy); 
	}
}
