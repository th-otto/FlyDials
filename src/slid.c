/*
	@(#)ListMan/slid.c

	Julian F. Reschke, 12. Januar 1995
*/

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "flydial/flydial.h"
#include "flydial/slid.h"

#define NIL -1

/*static int
wind_get_color (int element)
{
	_GemParBlk.intin[0] = 0;
	_GemParBlk.intin[1] = WF_DCOLOR;
	_GemParBlk.intin[2] = element;
	_AesCtrl (104);
	return _GemParBlk.intout[2];
}
*/

static OBJECT ProtoSlider[] =
{
	NIL, 1, 4, G_BOX, NONE, NORMAL, 0x11110L, 0, 0, 37, 1,
	2, NIL, NIL, G_BOXCHAR, NONE, NORMAL, 0x4011101L, 0, 0, 4, 1,
	4, 3, 3, G_BOX, NONE, NORMAL, 0x11111L, 4, 0, 29, 1,
	2, NIL, NIL, G_BOX, NONE, NORMAL, 0x11100L, 11, 0, 6, 1,
	0, NIL, NIL, G_BOXCHAR, LASTOB, NORMAL, 0x3011101L, 33, 0, 4, 1
};

#define SLIDERROOT 0
#define ARROW1 1
#define BAR 2
#define SLIDER 3
#define ARROW2 4

void
SlidDraw (SLIDERSPEC *s, int xc, int yc, int wc, int hc, int redraw)
{
	int new,last,size,doit;
	int sx,sy,sw,sh;
	int ax,ay,aw,ah;
	
	if (!s->drawn)
		redraw = s->drawn = TRUE;

	HandScreenSize (&sx,&sy,&sw,&sh);
	RectInter (xc,yc,wc,hc,sx,sy,sw,sh,&ax,&ay,&aw,&ah);


	last = s->lastdraw;

	if (s->hori)
	{
		new = s->ob[SLIDER].ob_x;
		size = s->ob[SLIDER].ob_width;
	}
	else
	{
		new = s->ob[SLIDER].ob_y;
		size = s->ob[SLIDER].ob_height;
	}

	if (redraw)
		objc_draw(&(s->ob[0]),0,10,ax,ay,aw,ah);
	else
	{
		int x,w;

		if (new != last)
		{
			if ( (last > new) && (last < new + size))
			{
				x = new+size;
				w = last-new;
			}
			else
			{
				if ( (new > last) && (new < last + size))				
				{
					x = last;
					w = new-last;
				}
				else
				{
					x = last;
					w = size;
				}
			}
					
			if (s->hori)
				doit = RectInter(ax,ay,aw,ah,x+s->offset,yc,w,hc,&sx,&sy,&sw,&sh);
			else
				doit = RectInter(ax,ay,aw,ah,xc,x+s->offset,wc,w,&sx,&sy,&sw,&sh);

			if (doit)
				objc_draw(&(s->ob[0]),BAR,0,sx,sy,sw,sh);
			objc_draw(&(s->ob[0]),SLIDER,0,xc,yc,wc,hc);

		}
	}

	if (redraw) s->lastdraw = new;
}

void SlidDrCompleted (SLIDERSPEC *s)
{
	if (s->hori)
		s->lastdraw = s->ob[SLIDER].ob_x;
	else
		s->lastdraw = s->ob[SLIDER].ob_y;
}

void SlidSlidSize (SLIDERSPEC *s, int size)
{
	int newsize;

	s->lastdraw = NIL;
	s->size = size;
	if (size == -1)
		newsize = s->ob[ARROW1].ob_width;
	else if (!size)
	{
		if (s->hori)
			newsize = s->ob[BAR].ob_width;
		else
			newsize = s->ob[BAR].ob_height;
	}
	else
	{
		long tmp;

		tmp = s->hori ? s->ob[BAR].ob_width : s->ob[BAR].ob_height;
		tmp *= size;
		tmp /= (s->scale + s->size);
		newsize = (int)tmp;
		if (newsize < s->ob[ARROW1].ob_width)
			newsize = s->ob[ARROW1].ob_width;
	}
	
	if (s->hori)
		s->ob[SLIDER].ob_width = newsize;
	else
		s->ob[SLIDER].ob_height = newsize;
}

void SlidScale (SLIDERSPEC *s, long scale)
{
	s->lastdraw = NIL;
	s->scale = scale;
	SlidSlidSize (s, s->size);
}

void SlidPos (SLIDERSPEC *s, long pos)
{
	int range;
	long newpos;

	if (pos == s->position) return;
	s->position = pos;

	if (s->hori)
		range = s->ob[BAR].ob_width - s->ob[SLIDER].ob_width;
	else
		range = s->ob[BAR].ob_height - s->ob[SLIDER].ob_height;
	
	newpos = range;
	newpos *= pos;
	newpos /= (s->scale);
	
	if (s->hori)
		s->ob[SLIDER].ob_x = (int)newpos;	
	else
		s->ob[SLIDER].ob_y = (int)newpos;	
}

SLIDERSPEC *
SlidCreate (SLIDERSPEC *Slider, int hori, int accel)
{
	int i;

	if (!Slider) Slider = calloc(1, sizeof(SLIDERSPEC));

	if (!Slider) return NULL;

	Slider->accel = accel;
	Slider->hori = hori;
	SlidScale(Slider,1000);
	SlidSlidSize(Slider,-1);
	SlidPos(Slider,0);
	memcpy(&(Slider->ob[0]),ProtoSlider,sizeof(ProtoSlider));

	for (i=SLIDERROOT; i <= ARROW2; i++)
	{
		Slider->ob[i].ob_x =
		Slider->ob[i].ob_y = 0;
		Slider->ob[i].ob_height = HandBYSize;
		Slider->ob[i].ob_width = HandBXSize;
	}

/*	value = wind_get_color (W_UPARROW);
	Slider->ob[ARROW1].ob_spec.index &= 0xffff0000L;
	Slider->ob[ARROW1].ob_spec.index |= value;
*/
	return Slider;
}

void SlidDelete (SLIDERSPEC *s)
{
	free(s);
}


void SlidExtents (SLIDERSPEC *s, int x, int y, int len)
{
	x--;
	y--;
	len += 2;
	
	s->lastdraw = NIL;
	s->ob[SLIDERROOT].ob_x = x;
	s->ob[SLIDERROOT].ob_y = y;

	if (s->hori)	/* Horizontaler Slider ? */
	{
		s->ob[SLIDERROOT].ob_width = len;
		s->ob[BAR].ob_x = s->ob[ARROW1].ob_width - 1;
		s->ob[BAR].ob_width = len + 2 - ((s->ob[ARROW1].ob_width)<<1);
		s->offset = x + s->ob[BAR].ob_x;
	}
	else
	{
		s->ob[ARROW1].ob_spec.obspec.character = 1;
		s->ob[ARROW2].ob_spec.obspec.character = 2;
		s->ob[SLIDERROOT].ob_height = len;
		s->ob[BAR].ob_y = s->ob[ARROW1].ob_height - 1;
		s->ob[BAR].ob_height = len + 2 - ((s->ob[ARROW1].ob_height)<<1);
		s->offset = y + s->ob[BAR].ob_y;
	}

	s->ob[ARROW1].ob_x = s->ob[ARROW1].ob_y = 0; 

	s->ob[ARROW2].ob_x =
		s->ob[SLIDERROOT].ob_width - s->ob[ARROW2].ob_width;

	s->ob[ARROW2].ob_y =
		s->ob[SLIDERROOT].ob_height - s->ob[ARROW2].ob_height;

	SlidSlidSize(s, s->size);
}

/*	In: Mauskoordinate, Out: neue Position */

long SlidClick (SLIDERSPEC *s, int x, int y, int clicks, int inwindow)
{
	int wodrinne;
	long new;
	int range;
		
	if (s->hori)
		range = s->ob[BAR].ob_width - s->ob[SLIDER].ob_width;
	else
		range = s->ob[BAR].ob_height - s->ob[SLIDER].ob_height;

	wodrinne = objc_find (s->ob, SLIDERROOT, 2, x, y);
	
	if (wodrinne == NIL) return NIL;	
	
	switch (wodrinne)
	{
		case ARROW1:
			if (inwindow)
			{
				OBJECT *o = s->ob;
				objc_change (s->ob, ARROW1, 0, o->ob_x, o->ob_y,
					o->ob_width, o->ob_height,
					SELECTED, TRUE);
			}
			
			new = s->position - s->accel;
			if (clicks > 1) new = 0;
			break;
	
		case ARROW2:
			if (inwindow)
			{
				OBJECT *o = s->ob;
				objc_change (s->ob, ARROW2, 0, o->ob_x, o->ob_y,
					o->ob_width, o->ob_height,
					SELECTED, TRUE);
			}

			new = s->position + s->accel;
			if (clicks > 1 ) new = s->scale;
			break;

		case BAR:
			{
				int sx,sy;
				
				ObjcOffset (s->ob, SLIDER, &sx, &sy);
				
				if ((x < sx) || (y < sy))
					new = s->position - s->size;
				else
					new = s->position + s->size;
			}
			break;

		case SLIDER:
			{
				int newpos;
				long tmp;

				/* Raltime-Scrolling? */
				if (clicks == 2 ||
					(inwindow && s->scale < (range * s->accel)))
				{
					OBJECT *o = s->ob;
	
					objc_change (s->ob, SLIDER, 0, o->ob_x, o->ob_y,
						o->ob_width, o->ob_height,
						SELECTED, TRUE);
					return -2;
				}

				WindUpdate (BEG_MCTRL);
				newpos = graf_slidebox (s->ob, BAR, SLIDER,
					1 - s->hori);
				WindUpdate (END_MCTRL);
				
				tmp = ((long)s->scale)*newpos;
				
				new = (int) (tmp/1000L);
				if ((tmp%1000L)>= 500L)
					new++;
			}
			break;
				
		default:
			break;
	}
	
	if (new > s->scale )
		new = s->scale;
	else
		if (new < 0) new = 0;
	return new;	
}

long SlidAdjustSlider (SLIDERSPEC *s, int x, int y)
{
	int bx,by,sx,sy;

	ObjcOffset (s->ob, BAR, &bx, &by);
	ObjcOffset (s->ob, SLIDER, &sx, &sy);

	if (!s->hori)
	{
		int space;
	
		y -= s->ob[SLIDER].ob_height >> 1;

		if (y <= by) return 0;
		if (y >= (by + s->ob[BAR].ob_height)) return s->scale;
		
		y -= by;
		
		space = s->ob[BAR].ob_height - s->ob[SLIDER].ob_height;
		
		if (space)
			return (((long)y * s->scale) / space);
		else
			return 0;
	}
	else
	{
		int space;
	
		x -= s->ob[SLIDER].ob_width >> 1;
	
		if (x <= bx) return 0;
		if (x >= (bx + s->ob[BAR].ob_width)) return s->scale;
		
		x -= bx;

		space = s->ob[BAR].ob_width - s->ob[SLIDER].ob_width;
		
		if (space)
			return (((long)x * s->scale) / space);
		else
			return 0;
	}
}

void SlidDeselect (SLIDERSPEC *s)
{
	int obs[] = {ARROW1, ARROW2, SLIDER};
	int i;
	OBJECT *o = s->ob;
	
	for (i = 0; i < sizeof(obs)/sizeof(obs[0]); i++)
		if (o[obs[i]].ob_state & SELECTED)
				objc_change (s->ob, obs[i], 0, o->ob_x, o->ob_y, o->ob_width,
					o->ob_height, NORMAL, TRUE);
}
