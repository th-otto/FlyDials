/*
	@(#)FlyDial/fdlight.c
	Julian F. Reschke, 6. Mai 1995
	
	'Light'-Version fÅr MagiC 3
*/

#include <aes.h>
#include <tos.h>
#include <stdio.h>

#include <flydial/fdlight.h>

static int    contrl[15];
static int    global[15];
static int    intin[132];
static int    intout[140];
static long   addrin[16];
static long   addrout[16];

static AESPB A = { contrl, global, intin, intout, (int *)addrin, (int *) addrout };

static char sccsid[] = "@(#)Fliegende Dialoge (light) 0.49, Copyright (c) "
	"Julian F. Reschke, "__DATE__;

static int
form_xdial (int flag, int ltx, int lty, int ltw, int lth,
	int bgx, int bgy, int bgw, int bgh, void **flydial)
{
	contrl[0] = 51;
	contrl[1] = 9;
	contrl[2] = 1;
	contrl[3] = 2;
	contrl[4] = 0;
	intin[0] = flag;
	intin[1] = ltx; intin[2] = lty;  intin[3] = ltw; intin[4] = lth;
	intin[5] = bgx; intin[6] = bgy;  intin[7] = bgw; intin[8] = bgh;
	addrin[0] = (long) flydial;
	_crystal (&A);
	return intout[0] + 0 * (int)sccsid;;
}

static int
form_xdo (OBJECT *tree, int startob, int *lastcrsr, void *tabs,
	void *flydial)
{
	contrl[0] = 50;
	contrl[1] = 1;
	contrl[2] = 2;
	contrl[3] = 3;
	contrl[4] = 0;
	intin[0] = startob;
	addrin[0] = (long) tree;
	addrin[1] = (long) tabs;
	addrin[2] = (long) flydial;
	_crystal (&A);
	*lastcrsr = intout[1];
	return intout[0];
}

static long
cookieptr (void)
{
	return *((long *)0x5a0);
}

static int getcookie (long cookie, long *p_value)
{
	long *cookiejar;

	cookiejar = (long *) Supexec (cookieptr);
	
	if (!cookiejar)
		return 0;

	do
	{
		if (cookiejar[0] == cookie)
		{
			if (p_value) *p_value = cookiejar[1];
			return 1;
		}
		else
			cookiejar = &(cookiejar[2]);
	} while (cookiejar[-2]);

	return 0;
}

typedef struct
{
	long	cookie;		/* VSCR */
	long	product;	/* Produktname */
	int		version;
	int		x, y, w, h;	/* sichtbares Fenster */
} INFOVSCR;

void
HandScreenSize (int *x, int *y, int *w, int *h)
{
	static int wx, wy, ww = 0, wh;

	if (!ww)
		wind_get (0, WF_WORKXYWH, &wx, &wy, &ww, &wh);
	
	*x = wx; *y = wy; *w = ww; *h = wh;
}

#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)

int RectInter (int x1, int y1, int w1, int h1, int x2, int y2, int w2,
	int h2, int *x3, int *y3, int *w3, int *h3)
{
	int t1,t2;	

	*x3 = MAX(x1,x2);
	*y3 = MAX(y1,y2);
	t1 = x1+w1; t2 = x2+w2;
	*w3 = MIN(t1,t2) - *x3;
	t1 = y1+h1; t2 = y2+h2;
	*h3 = MIN(t1,t2) - *y3;
	return((*w3>0)&&(*h3>0));
}

void
DialCenter (OBJECT *T)
{
	int dummy;
	int ow, oh;

	form_center (T, &dummy, &dummy, &ow, &oh);

	{
		INFOVSCR *I;
		
		if (getcookie ('VSCR', (long *)&I))
			if (I->cookie == 'VSCR')
			{
				int sx, sy, sw, sh;
				int rx, ry, rw, rh;
				int hoffset, voffset;
				int xrand, yrand;
			
				xrand = (ow - T->ob_width) >> 1;
				yrand = (oh - T->ob_height) >> 1;
				
				HandScreenSize (&sx, &sy, &sw, &sh);
				
				if (!RectInter (sx, sy, sw, sh, I->x, I->y, I->w, I->h,
					&rx, &ry, &rw, &rh))
					return;
				
				hoffset = (rw - ow) / 2;
				voffset = (rh - oh) / 2;
				
				if (hoffset < 0)	/* zu wenig Platz? */
				{
					if (rx + hoffset < sx)
						hoffset = sx - rx;
					if (rx + hoffset + ow > sx + sw)
						hoffset = sx + sw - ow - rx;
					if (rx + hoffset < sx)	/* immer noch? */
						return;
				}

				if (voffset < 0)	/* zu wenig Platz? */
				{
					if (ry + voffset < sy)
						voffset = sy - ry;
					if (ry + voffset + oh > sy + sh)
						voffset = sy + sh - oh - ry;
					if (ry + voffset < sy)	/* immer noch? */
						return;
				}


				T->ob_x = rx + hoffset + xrand;
				T->ob_y = ry + voffset + yrand;

				return;
			}
	}
}

static int
has_m_fd (void)
{
	int dummy;
	static int hasit = -1;
	
	if (hasit < 0)
	{
		hasit = 0;
		appl_xgetinfo (14, &hasit, &dummy, &dummy, &dummy);
	}
		
	return hasit;
}

int
DialStart (OBJECT *tree, DIALINFO *dip)
{
	dip->tree = tree;
	dip->buffer = 0L;
	
	wind_update (BEG_UPDATE);
	wind_update (BEG_MCTRL);
	dip->buffer = 0;
	form_center (tree, &dip->x, &dip->y, &dip->w, &dip->h);
	
	if (has_m_fd())
		form_xdial (FMD_START, dip->x, dip->y, dip->w, dip->h,
			dip->x, dip->y, dip->w, dip->h, &dip->buffer);
	else
		form_dial (FMD_START, dip->x, dip->y, dip->w, dip->h,
			dip->x, dip->y, dip->w, dip->h);
	
	return 1;
}

void
DialDraw (DIALINFO *dip)
{
	objc_draw (dip->tree, 0, 12, dip->x, dip->y, dip->w, dip->h);
	
}

void
DialEnd (DIALINFO *dip)
{
	if (has_m_fd())
		form_xdial (FMD_FINISH, dip->x, dip->y, dip->w, dip->h,
			dip->x, dip->y, dip->w, dip->h, &dip->buffer);
	else
		form_dial (FMD_FINISH, dip->x, dip->y, dip->w, dip->h,
			dip->x, dip->y, dip->w, dip->h);
	
	wind_update (END_MCTRL);
	wind_update (END_UPDATE);
}

int
DialDo (DIALINFO *dip, int *startob)
{
	int so = 0;
	int but;
	int lastcrsr;
	
	if (!startob) startob = &so;
	
	if (has_m_fd())
		but = form_xdo (dip->tree, *startob, &lastcrsr, 0, dip->buffer);
	else
		but = form_do (dip->tree, *startob);

	return but;
}