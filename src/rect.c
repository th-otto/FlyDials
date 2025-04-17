/*
	@(#)FlyDial/rect.c
	@(#)Julian F. Reschke, 13. August 1990
	
	Rect-Functions
*/

#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)

#include "flydial/flydial.h"

void RectAES2VDI (int x, int y, int w, int h, int *xy)
{
    int *p;

    p = xy;
    *p++ = x;
    *p++ = y;
    *p++ = x+w-1;
    *p = y+h-1;
}

void RectGRECT2VDI (const GRECT *g, int *xy)
{
	*xy++ = g->g_x;
	*xy++ = g->g_y;
	*xy++ = g->g_x + g->g_w - 1;
	*xy   = g->g_y + g->g_h - 1;
}

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

int RectGInter (const GRECT *a, const GRECT *b, GRECT *c)
{
	int t1,t2;	

	c->g_x = MAX (a->g_x, b->g_x);
	c->g_y = MAX (a->g_y, b->g_y);
	t1 = a->g_x + a->g_w;
	t2 = b->g_x + b->g_w;
	c->g_w = MIN (t1, t2) - c->g_x;
	t1 = a->g_y + a->g_h;
	t2 = b->g_y + b->g_h;
	c->g_h = MIN (t1, t2) - c->g_y;
	return((c->g_w > 0)&&(c->g_h > 0));
}

int RectOnScreen (int x, int y, int w, int h)
{
	int ax, ay, aw, ah;
	
	HandScreenSize(&ax,&ay,&aw,&ah);
	return ( ( ax<=x) && (ay<=y) && (x+w <= ax+aw) && (y+h <= ay+ah)); 
}

int RectInside (int x1, int y1, int w1, int h1, int x2, int y2)
{
	return ( (x2>=x1) && (y2>=y1) && (x2<(x1+w1)) && (y2<(y1+h1)));
}

void RectClipWithScreen (GRECT *g)
{
	static GRECT screen = {0,0,0,0};
	GRECT temp;
	
	if (!screen.g_w)
		HandScreenSize (&screen.g_x, &screen.g_y, &screen.g_w, &screen.g_h);
	
	temp = *g;
	RectGInter (&temp, &screen, g);
}

