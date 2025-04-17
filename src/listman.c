/*
	@(#)ListMan/listman.c
	
	Julian F. Reschke & Stefan Eissing, 30. April 1996
*/

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "flydial/flydial.h"
#include "flydial/slid.h"
#include "flydial/listman.h"
#include "flydial/wk.h"

#define DIM(x) (sizeof(x) / sizeof(x[0]))

#define MIN(a,b) ((a>b)?(b):(a))
#define MAX(a,b) ((a<b)?(b):(a))

#define FALSE	0
#define TRUE	(!FALSE)

/* Schnittstelle: listman <-> slid unter Bercksichtigung von
   Slidern in Fenstern */

static void
slidlpos (LISTINFO *L, long pos)
{
	if (L->windhandle < 0)
		SlidPos (L->lslid, pos);
	else
	{
		if (!L->lslid->scale)
			wind_set (L->windhandle, WF_VSLIDE, 1);
		else
			wind_set (L->windhandle, WF_VSLIDE,  (int)
				((pos * 1000L) / L->lslid->scale));
	}
}

static void
slidmpos (LISTINFO *L, long pos)
{
	if (L->windhandle < 0)
		SlidPos (L->mslid, pos);
	else
	{
		if (!L->mslid->scale)
			wind_set (L->windhandle, WF_HSLIDE, 1);
		else
			wind_set (L->windhandle, WF_HSLIDE,  (int)
				((pos * 1000L) / L->mslid->scale));
	}
}

static void
slidlsize (LISTINFO *L, int size)
{
	if (L->windhandle < 0)
		SlidSlidSize (L->lslid, size);
	else
	{
		if (size == -1)
			wind_set (L->windhandle, WF_VSLSIZE, size);
		else
			wind_set (L->windhandle, WF_VSLSIZE, (int)
				((size * 1000L) / L->listlength));
	}
}

static void
slidmsize (LISTINFO *L, int size)
{
	if (L->windhandle < 0)
		SlidSlidSize (L->mslid, size);
	else
	{
		if (size == -1)
			wind_set (L->windhandle, WF_HSLSIZE, size);
		else
			wind_set (L->windhandle, WF_HSLSIZE, (int)
				((size * 1000L) / L->maxwidth));
	}
}


/* invertiert den mit <G> angegebenen Bereich */
static void
invert (GRECT *G)
{
	int xy[4];
	GRECT scr;
	GRECT res;

	HandScreenSize (&scr.g_x, &scr.g_y, &scr.g_w, &scr.g_h);
	if (RectGInter (G, &scr, &res))
	{	
		RectGRECT2VDI (&res, xy);
		
		WkWritingMode (MD_XOR);
		WkFillMode (BLACK, FIS_SOLID, 1, 8);

		vr_recfl (DialWk,xy);
	}
}

/* sagt mir, ob der Eintrag sichtbar ist */
static int
visible (LISTINFO *L, long obnum)
{
	return ((obnum >= L->startindex) && (obnum < L->startindex+L->lines));
}

static int
inrange (long beg, long end, long isit)
{
	if (beg > end)
		return ((beg >= isit) && (isit >= end));
	else
		return ((end >= isit) && (isit >= beg));
}


/* ermittelt die L„nge der Liste <list> */
static long
listlength (LISTSPEC *list)
{
	long l = 0;
	
	while (list)
	{
		l++;
		list = list->next;
	}
	return l;	
}

LISTSPEC *
ListIndex2List (LISTSPEC *L, long ob)
{
	while (ob)
	{
		ob--;
		L = L->next;
	}
	
	return L;
}

/* berechnet die Anzahl der darstellbaren Eintr„ge */
static int
countlines (LISTINFO *pli)
{
	int cnt = 1;
	
	if (pli->boxtree)
	{
		ObjcOffset(pli->boxtree, pli->boxindex,
			&pli->area.g_x, &pli->area.g_y);
		
		pli->area.g_w = pli->boxtree[pli->boxindex].ob_width;
		pli->area.g_h = pli->boxtree[pli->boxindex].ob_height;
	}

	if (pli->voffset)
	{	
		cnt = pli->area.g_h / pli->voffset;
		if ((pli->area.g_h % pli->voffset) != 0)
			++cnt;
	}
	else
	{
		cnt = pli->area.g_w / pli->hoffset;
		if ((pli->area.g_w % pli->hoffset) != 0)
			++cnt;
	}

	return cnt;
}

int
ListInit (LISTINFO *L)
{
	if (L->boxtree)
	{
		int fob = L->boxtree[L->boxindex].ob_head;
		int i,j;
		
		if (fob >= 0)
		{
			L->hoffset = L->hsize = L->boxtree[fob].ob_width;
			L->voffset = L->vsize = L->boxtree[fob].ob_height;
		}
		
		i = L->boxtree[L->boxindex].ob_head;

		if ((i >= 0) && ((j = L->boxtree[i].ob_next) >= 0))
		{
			L->hoffset = abs(L->boxtree[j].ob_x - L->boxtree[i].ob_x);
			L->voffset = abs(L->boxtree[j].ob_y - L->boxtree[i].ob_y);
		}
	}

	L->listlength = listlength (L->list);

	L->lines = countlines (L);
	L->lines = (int) MIN (L->lines, L->listlength);

	if ((L->startindex + L->lines) > L->listlength)
		L->startindex = L->listlength - L->lines;


	L->lslid = SlidCreate (NULL, FALSE, 1);
	if (!L->lslid)
		return FALSE;

	if (L->lslidx <= 0)
	{
		L->lslidx = L->area.g_x + L->area.g_w;
	}

	if (L->lslidy <= 0)
	{
		L->lslidy = L->area.g_y;
	}

	if (L->lslidlen <= 0)
		L->lslidlen = L->area.g_h;
	
	L->lslidx++;

	SlidExtents (L->lslid, L->lslidx, L->lslidy, L->lslidlen); 
	SlidScale (L->lslid, L->listlength - L->lines);
	slidlsize (L, (int)L->lines);
	slidlpos (L, L->startindex);
	

	if (L->maxwidth > 0)
	{
		L->mslid = SlidCreate (NULL, TRUE, L->hstep);

		if (!L->mslid)
		{
			SlidDelete (L->lslid);
			return FALSE;
		}
		
		if (L->mslidx <= 0)
		{
			L->mslidx = L->area.g_x;
		}

		if (L->mslidy <= 0)
		{
			L->mslidy = L->area.g_y + L->area.g_h;
		}

		if (L->mslidlen <= 0)
			L->mslidlen = L->area.g_w;
		
		L->mslidy++;

		if ((L->hpos + L->area.g_w) > L->maxwidth)
			L->hpos = L->maxwidth - L->area.g_w;
		
		SlidExtents (L->mslid, L->mslidx, L->mslidy, L->mslidlen); 
		SlidScale (L->mslid, (L->maxwidth - L->mslidlen));
		slidmsize (L, L->mslidlen);
		slidmpos (L, L->hpos);
	}
	else
		L->mslid = NULL;

	
	/* Hintergrundbox sizen */
	
	if (L->boxtree)
	{
		L->boxtree[L->boxbgindex].ob_width = L->boxtree[L->boxindex].ob_width;
		L->boxtree[L->boxbgindex].ob_height = L->boxtree[L->boxindex].ob_height;

		if (L->lslid)
			L->boxtree[L->boxbgindex].ob_width += (HandBXSize - 1);

		if (L->mslid)
			L->boxtree[L->boxbgindex].ob_height += (HandBYSize - 1);
	}

	return TRUE;
}

static void
listrectdraw (LISTINFO *L, GRECT *G)
{
	int line;
	LISTSPEC *lp;
	GRECT gr;
	GRECT cg;
	GRECT doit;

	cg = L->area;
	if (RectGInter (G, &cg, &gr))
	{
		if (L->boxtree)
			objc_draw (L->boxtree, L->boxindex, 0, gr.g_x, gr.g_y,
						gr.g_w, gr.g_h);
		else
			L->drawfunc (NULL, cg.g_x, cg.g_y, L->hpos, &gr, NORMAL);
	}
	
	cg = L->area;
	cg.g_w = L->hsize;
	cg.g_h = L->vsize;
	
	lp = ListIndex2List (L->list, L->startindex);

	for (line = 0; line < L->lines; line++)
	{
		if (RectGInter (&gr, &cg, &doit))
		{
			L->drawfunc (lp, cg.g_x, cg.g_y, L->hpos, &doit,
				LISTDRAWREDRAW);
		}
		cg.g_x += L->hoffset;
		cg.g_y += L->voffset;
		lp = lp->next;
	}
}

void
ListWindDraw (LISTINFO *L, GRECT *G, int fullredraw)
{
	int wx,wy,ww,wh;
	GRECT neu;

	GrafMouse (M_OFF, NULL);
	if (L->windhandle < 0)
	{
		listrectdraw (L, G);
		SlidDraw(L->lslid, 0, 0, 32767, 32767, fullredraw);
		SlidDrCompleted (L->lslid);

		if (L->mslid)
		{
			SlidDraw(L->mslid, 0, 0, 32767, 32767, fullredraw);
			SlidDrCompleted (L->mslid);
		}
		
		GrafMouse (M_ON, NULL);
		return;
	}
	
	WindUpdate (BEG_UPDATE);
	wind_get (L->windhandle, WF_FIRSTXYWH, &wx, &wy, &ww, &wh);

	while (ww && wh)
	{
		if (RectInter (G->g_x,G->g_y,G->g_w,G->g_h,
			wx,wy,ww,wh,&neu.g_x,&neu.g_y,&neu.g_w,&neu.g_h))
				listrectdraw (L, &neu);

		wind_get (L->windhandle, WF_NEXTXYWH, &wx, &wy, &ww, &wh);
	}
	WindUpdate (END_UPDATE);
	GrafMouse (M_ON, NULL);
}



void
ListDraw (LISTINFO *L)
{
	GRECT rect, screen;

	GrafMouse(M_OFF, NULL);	

	HandScreenSize(&screen.g_x, &screen.g_y,
					 &screen.g_w, &screen.g_h);
/*	
	if (L->boxtree)
	{
		ObjcOffset (L->boxtree, L->boxindex,
					&L->area.g_x, &L->area.g_y);
	}
*/
	rect = L->area;
		
	if (L->windhandle < 0)
	{
		SlidDraw (L->lslid, screen.g_x, screen.g_y,
					screen.g_w, screen.g_h, TRUE);
		if (L->mslid)
			SlidDraw (L->mslid, screen.g_x, screen.g_y,
					screen.g_w, screen.g_h, TRUE);

		listrectdraw (L, &rect);
	}
	else
	{
		int wx,wy,ww,wh;

		WindUpdate (BEG_UPDATE);
		wind_get (L->windhandle, WF_FIRSTXYWH, &wx, &wy, &ww, &wh);
		while (ww && wh)
		{
			GRECT dr;

			if (RectInter (rect.g_x, rect.g_y, rect.g_w, rect.g_h,
				wx, wy, ww, wh, &dr.g_x, &dr.g_y, &dr.g_w, &dr.g_h))
			{
				listrectdraw (L, &dr);
			}
			wind_get (L->windhandle, WF_NEXTXYWH, &wx, &wy, &ww, &wh);
		}
		WindUpdate (END_UPDATE);
	}

	GrafMouse(M_ON, NULL);	
}

static void
copyrect (LISTINFO *L, int x, int y, int w, int h, int dx, int dy)
{
	if (L->windhandle < 0)
		RastBufCopy (x, y, w, h, dx, dy, &RastScreenMFDB);
	else
	{
		int wx,wy,ww,wh,x1,y1,w1,h1;

		WindUpdate (BEG_UPDATE);
		wind_get (L->windhandle, WF_FIRSTXYWH, &wx, &wy, &ww, &wh);
		while (ww && wh)
		{
			/* Ist das Destination-Rect. sichtbar? */
			if (RectInter (dx,dy,w,h,wx,wy,ww,wh,&x1,&y1,&w1,&h1))
			{
				int x2,y2,w2,h2,x3,y3,w3,h3;
				GRECT G;
				
				/* x1,y1,... enthalten jetzt das sichtbare Dest.-Rec. */
				/* In x2,... wird der entsprechende Source-Rect.
				 * gespeichert. */
				x2 = x + (x1 - dx);
				y2 = y + (y1 - dy);
				w2 = w1;
				h2 = h1;
				
				/* Ist der x2,.. Teil vom Source-Rect. sichtbar? */
				if (RectInter(x2,y2,w2,h2,wx,wy,ww,wh,&x3,&y3,&w3,&h3))
				{
					int xdest,ydest,giveup;
					
					/* xdest und ydest sind die x3 und y3 entsprechenden
					 * Punkte im Destination-Rect. */
					xdest = x1 + (x3 - x2);
					ydest = y1 + (y3 - y2);
					
					giveup = FALSE;
					
					/* G ist das Rectangle, was wir von Hand zeichnen
					 * mssen */
					G.g_x = x1;
					G.g_y = y1;
					G.g_w = G.g_h = 0;
					
					if (w1 > w3)		/* horiz. Scrollen? */
					{
						/* Well, wir haben einen L- oder U-f”rmigen
						 * Bereich neu zu zeichnen; also vergessen
						 * wir das Blitten und malen lieber das ganze
						 * Rectangle von Hand. */
						giveup = (h1 > h3);
						G.g_w = w1 - w3;
						G.g_h = h1;
						if (x1 == xdest)
							G.g_x += w3;
					}
					else if (h1 > h3)	/* vertikales Scrollen? */
					{
						G.g_h = h1 - h3;
						G.g_w = w1;
						if (y1 == ydest)
							G.g_y += h3;
					}
					
					if (!giveup)
						RastBufCopy (x3, y3, w3, h3, xdest, ydest,
									 &RastScreenMFDB);
					else
					{
						G.g_x = x1;
						G.g_y = y1;
						G.g_w = w1;
						G.g_h = h1;
					}
				}
				else
				{
					G.g_x = x1;
					G.g_y = y1;
					G.g_w = w1;
					G.g_h = h1;
				}
				
				/* Ist noch was von Hand zu zeichnen? */
				if ((G.g_w > 0) && (G.g_h > 0))
					listrectdraw(L, &G);
			}
			wind_get (L->windhandle, WF_NEXTXYWH, &wx, &wy, &ww, &wh);
		}
		WindUpdate (END_UPDATE);
	}
}





/* scrolle in die angegebene Richtung <lines>; negative Zahlen
   bedeuten zum Anfang der Liste, positive... */
void
ListVScroll (LISTINFO *L, long Lines)
{
	long currindex;
	GRECT src, dest;
	long keeplines;
	int direction;
	
	ListMoved (L);	/* verschoben? */

	currindex = L->startindex + Lines;
	if ((currindex + L->lines) > L->listlength)
		currindex = L->listlength - L->lines;

	/* wir checken das erst hier, da die Liste zu kurz sein k”nnte */
	if (currindex < 0)
		currindex = 0;

	if (currindex == L->startindex)
		return;
	
	direction = (int)(L->startindex - currindex);
	Lines = labs (L->startindex - currindex);
	L->startindex = currindex;
	slidlpos (L, L->startindex);
	
	keeplines = L->lines - labs(Lines);
	src = L->area;
	RectClipWithScreen (&src);
	dest = src;
	
	v_hide_c (HandAES);

	if (keeplines > 0)	/* noch sichtbar */
	{
		int copied;

		src.g_h = (int)keeplines * L->voffset;
		src.g_h -= ((int)(L->lines * L->voffset) - L->area.g_h);
		dest.g_h = src.g_h;
		
		if (direction < 0)
			src.g_y += (int)Lines * L->voffset;
		else
			dest.g_y += (int)Lines * L->voffset;
			
		RectClipWithScreen (&src);
		copied = src.g_h;
		copyrect (L, src.g_x, src.g_y, src.g_w, src.g_h, dest.g_x, dest.g_y);
		
		src = L->area;
		src.g_h = L->area.g_h - dest.g_h;

		if (direction < 0)
			src.g_y += copied;
	}
	
	ListWindDraw(L, &src, FALSE);
	v_show_c (HandAES, 1);
}


void
ListHScroll (LISTINFO *L, int Cols)
{
	int currpos;
	GRECT src, dest;
	int keepcols;
	int direction;

	ListMoved (L);	/* verschoben? */
	
	currpos = L->hpos + Cols;
	if ((currpos + L->area.g_w) > L->maxwidth)
		currpos = L->maxwidth - L->area.g_w;

	/* wir checken das erst hier, da die Liste zu kurz sein k”nnte */
	if (currpos < 0)
		currpos = 0;

	if (currpos == L->hpos)
		return;
	
	direction = L->hpos - currpos;
	Cols = abs (L->hpos - currpos);
	L->hpos = currpos;
	slidmpos (L, L->hpos);
	
	keepcols = L->area.g_w - abs(Cols);
	src = L->area;
	RectClipWithScreen (&src);
	dest = src;
	
	v_hide_c (HandAES);

	if (keepcols > 0)	/* noch sichtbar */
	{
		int copied;
	
		src.g_w = keepcols;
		dest.g_w = src.g_w;
		
		if (direction < 0)
			src.g_x += Cols;
		else
			dest.g_x += Cols;

		RectClipWithScreen (&src);
		RectClipWithScreen (&dest);
		src.g_w = MIN (src.g_w, dest.g_w);
		copied = src.g_w;
		copyrect (L, src.g_x, src.g_y, src.g_w, src.g_h, dest.g_x, dest.g_y);

		src = L->area;
		src.g_w = Cols; /* L->area.g_w - dest.g_w; */

		if (direction < 0)
			src.g_x += copied;
	}
	
	ListWindDraw (L, &src, FALSE);
	v_show_c (HandAES, 1);
}

/*
	invertiere Eintrag posinbox
*/
static void
invertNr (LISTINFO *L, LISTSPEC *lp, long pos)
{
	GRECT ar,old;
	int posinbox = (int)(pos - L->startindex);
	
	ar = L->area;
	ar.g_x += posinbox * L->hoffset;
	ar.g_y += posinbox * L->voffset;
	ar.g_w = L->hsize;
	ar.g_h = L->vsize;
	old = ar;
	
	if (RectGInter (&old, &(L->area), &ar))
	{
		v_hide_c (HandAES);

		if (L->windhandle < 0)
			L->drawfunc (lp, old.g_x, old.g_y,
				L->hpos, &ar, 0);
		else
		{
			int wx,wy,ww,wh;
			GRECT neu;
	
			WindUpdate (BEG_UPDATE);
			wind_get (L->windhandle, WF_FIRSTXYWH, &wx, &wy, &ww, &wh);
			while (ww && wh)
			{
				if (RectInter (ar.g_x,ar.g_y,ar.g_w,ar.g_h,
					wx,wy,ww,wh,&neu.g_x,&neu.g_y,&neu.g_w,&neu.g_h))
				{
					L->drawfunc (lp, old.g_x, old.g_y,
						L->hpos, &neu, 0);
				}
				wind_get (L->windhandle, WF_NEXTXYWH, &wx, &wy, &ww, &wh);
			}
			WindUpdate (END_UPDATE);
		}		
		v_show_c (HandAES, 1);
	}
}


/*
	einen Eintrag neu zeichnen
*/

static void
update (LISTINFO *L, int posinbox)
{
	GRECT ar,old;
	
	v_hide_c (HandAES);
	ar = L->area;
	ar.g_x += posinbox * L->hoffset;
	ar.g_y += posinbox * L->voffset;
	ar.g_w = L->hsize;
	ar.g_h = L->vsize;
	old = ar;
	
	if (RectGInter (&old, &(L->area), &ar))
	{
		if (L->windhandle < 0)
			listrectdraw (L, &ar);
		else
		{
			int wx,wy,ww,wh;
			GRECT neu;
	
			WindUpdate (BEG_UPDATE);
			wind_get (L->windhandle, WF_FIRSTXYWH, &wx, &wy, &ww, &wh);
			while (ww && wh)
			{
				if (RectInter (ar.g_x,ar.g_y,ar.g_w,ar.g_h,
					wx,wy,ww,wh,&neu.g_x,&neu.g_y,&neu.g_w,&neu.g_h))
				{
					listrectdraw (L, &neu);
				}
				wind_get (L->windhandle, WF_NEXTXYWH, &wx, &wy, &ww, &wh);
			}
			WindUpdate (END_UPDATE);
		}		
	}
	
	v_show_c (HandAES, 1);
}

void
ListUpdateEntry (LISTINFO *L, long entry)
{
	ListMoved (L);	/* verschoben? */

	if (visible (L, entry))
		update (L, (int)(entry - L->startindex));
}

void
ListInvertEntry (LISTINFO *L, long entry)
{
	ListMoved (L);	/* verschoben? */

	if (visible (L, entry))
		invertNr (L, ListIndex2List (L->list, entry), entry);
}


/* Delay for scrolling. Die Formel besagt Folgendes:
   full_amout zeigt an, wieviele Objekte gleichzeitig sichtbar
   sind (Beispiel 5). Die Konstante 85 dividiert durch 5
   (also 15) zeigt an, wieviele Jiffies der Scrollschritt
   mindestens dauern muž. Die Konstante gibt also an, wie lange ein
   Objekt beim Durchscrollen mindestens sichtbar bleiben muž */

static void
wait (long lasttime, long full_amount)
{
	long oldstack;
/* XXX: BUG */
	oldstack = Super(0L);
	while (*((long *)0x4ba) - lasttime < (85 / full_amount));
	Super ((void *)oldstack);
}

static long
get_ms (void)
{
	long oldstack,val;

	oldstack = Super(0L);
	val = *((long *)0x4ba);
	Super ((void *)oldstack);
	return val;
}


/* mit dieser Routine kann man ListMan sagen, daž die Liste nun an
   einer neuen Position steht. Der Returnwert ist FALSE, wenn
   ListMan keine Ver„nderung feststellen konnte; sonst TRUE */
   
int
ListMoved (LISTINFO *L)
{
	if (L->boxtree)
	{
		int x,y;
		
		ObjcOffset (L->boxtree, L->boxindex, &x, &y);
		
		if ((L->area.g_x != x) || (L->area.g_y != y))
		{
			int dx, dy;

			dx = x - L->area.g_x;
			dy = y - L->area.g_y;
			L->area.g_x += dx;
			L->area.g_y += dy;

			L->lslidx += dx;
			L->lslidy += dy;
			L->lslid->drawn = FALSE;
			SlidExtents (L->lslid, L->lslidx, L->lslidy, L->lslidlen); 
			SlidScale (L->lslid, L->listlength - L->lines);
			slidlsize (L, L->lines);
			slidlpos (L, L->startindex);
			
			if (L->mslid)
			{	
				L->mslidx += dx;
				L->mslidy += dy;
				L->mslid->drawn = FALSE;
				SlidExtents (L->mslid, L->mslidx, L->mslidy, L->mslidlen); 
				SlidScale (L->mslid, L->maxwidth - L->mslidlen);
				slidmsize (L, L->mslidlen);
				slidmpos (L, L->hpos);
			}
			return TRUE;
		}
	}
	return FALSE;
}


/* Diese Routine wird bei einem Mausklick aufgerufen */

long
ListClick (LISTINFO *L, int clicks)
{
	LISTSPEC *selob;
	int mx,my,mk,mb;
	long which;
	long newpos, was;
	long visible_amount;
	int looping = FALSE;

	/* visible_amount enth„lt die Anzahl der sichtbaren `Streifen' */
	
	visible_amount = L->lines;

	/* erstmal schauen, ob die Liste noch da ist, wo sie
	   hingeh”rt */
	
	ListMoved (L);

	graf_mkstate (&mx, &my, &mb, &mk);

	if (L->lslid)
	{
		newpos = SlidClick (L->lslid, mx, my, clicks, L->windhandle < 0);
		
		if (newpos == -2)	/* realtime scrolling... */
		{
			MFORM TempForm;
			int TempNum;
	
			GrafGetForm (&TempNum, &TempForm);
			GrafMouse (FLAT_HAND, NULL);
	
			do
			{
				newpos = SlidAdjustSlider (L->lslid, mx, my);
				newpos -= L->startindex;
				ListVScroll (L, newpos);
				graf_mkstate (&mx, &my, &mb, &mk);
			} while (mb);
	
			SlidDeselect (L->lslid);
			GrafMouse (TempNum, &TempForm);
			return -1;
		}
	
		if (newpos != -1)
		{
			int dummy;
			
			newpos -= L->startindex;
			
			do
			{
				long was = get_ms ();
				ListVScroll (L, newpos); 
				if (!(mb & 2)) wait (was, visible_amount);
				graf_mkstate (&dummy, &dummy, &mb, &dummy);
			} while (mb);
	
			SlidDeselect (L->lslid);
			return -1 /* no change */;
		}
	}
	
	if (L->mslid)
	{
		newpos = SlidClick (L->mslid, mx, my, clicks, L->windhandle < 0);

		if (newpos == -2)	/* realtime scrolling... */
		{
			MFORM TempForm;
			int TempNum;
	
			GrafGetForm (&TempNum, &TempForm);
			GrafMouse (FLAT_HAND, NULL);

			do
			{
				newpos = SlidAdjustSlider (L->mslid, mx, my);
				newpos -= L->hpos;
				ListHScroll (L, (int)newpos); 
				graf_mkstate (&mx, &my, &mb, &mk);
			}
			while (mb);
			SlidDeselect (L->mslid);
			GrafMouse (TempNum, &TempForm);
			return -1;
		}

		if (newpos != -1)
		{
			int dummy;
	
			newpos -= L->hpos;		
			do
			{
				long was = get_ms ();
				ListHScroll (L, (int)newpos);
				/* ob das funktioniert? */
				if (!(mb & 2)) wait (was, L->maxwidth / L->hstep);
				graf_mkstate (&dummy, &dummy, &mb, &dummy);
			} while (mb);

			SlidDeselect (L->mslid);
			return -1;
		}
	}


	if (!RectInside (L->area.g_x, L->area.g_y, L->area.g_w, 
		L->area.g_h, mx, my))
		return -1;

	
	if (!L->selectionservice) return -1;

Again:
	which = (my - L->area.g_y) / L->voffset;

	was = get_ms ();

	if (looping)
	{
		if (which >= L->lines)
		{
			if (which <= L->lines) wait (was, visible_amount);
			which = L->lines - 1;
			ListVScroll (L, 1);
			was = get_ms ();
		}
		else
			if (my < L->area.g_y)
			{
				which = 0;
				if (L->area.g_y - my <= L->vsize) wait (was, visible_amount);
				ListVScroll (L, -1);
				was = get_ms ();
			}
	}	
	
	which += L->startindex;
	
	if ((which < 0) || (which >= L->listlength)) return -1;

	selob = ListIndex2List (L->list, which);

	if ((!(mk & 3)) || (L->selectionservice == 1))
		/* keine Shift-Taste oder nur einzelne? */
	{
		LISTSPEC *l;
		int currnum = 0;
		
		l = L->list;
		
		while (l)
		{
			if ((l->flags.selected) && (l != selob))
			{
				l->flags.selected = 0;
				
				if (visible(L, currnum))
					invertNr (L, l, currnum);
			}
			currnum++;
			l = l->next;
		}	
	}


	if ((mk & 3) && (L->selectionservice != 1))	/* switching */
	{
		selob->flags.selected ^= 1;
		invertNr (L, selob, which);
	}
	else
		if (!selob->flags.selected)
		{
			selob->flags.selected = 1;
			invertNr (L, selob, which);
		}
	
	{
		int dx, dy;
		
		dx = L->area.g_x + (int)(which - L->startindex) * L->hoffset;
		dy = L->area.g_y + (int)(which - L->startindex) * L->voffset;
		
		do
		{
			graf_mkstate (&mx, &my, &mb, &mk);	
		}
		while (mb && RectInside (dx, dy, L->hsize, L->vsize, mx, my));
		looping = TRUE;

		if (mb && (L->selectionservice == 1)) goto Again;	
	}

	if (L->selectionservice < 2) return which;	
	
	/* hier wird ein Block aufgezogen */

	{
		long anchor = which;
		long lastwohin = which;
		long wohin;
		
		while (mb)
		{
			graf_mkstate (&mx, &my, &mb, &mk);	

			if (L->hoffset)
			{
				if (mx < L->area.g_x)
					wohin = -1;
				else
					wohin = (mx - L->area.g_x) / L->hoffset;
			}
			else
			{
				if (my < L->area.g_y)
					wohin = -1;
				else
					wohin = (my - L->area.g_y) / L->voffset;
			}
			
			wohin += L->startindex;
			
			if (wohin < L->startindex)
			{
				ListVScroll (L, -1);
				if (L->area.g_y - my <= L->vsize) wait (was, visible_amount);
				was = get_ms ();
				wohin = L->startindex;
			}
			else if (wohin >= L->startindex + L->lines)
			{
				ListVScroll (L, 1);
				if (wohin <= L->startindex + L->lines) wait (was, visible_amount);
				was = get_ms ();
				wohin = L->startindex + L->lines-1;
			}
			
			{
				long von = MIN (wohin, MIN (lastwohin, anchor));
				long bis = MAX (wohin, MAX (lastwohin, anchor));
				long i;
				LISTSPEC *l;

				l = ListIndex2List (L->list, von);
				
				for (i=von; i<=bis; i++)
				{
					int oldstate;
					
					oldstate = l->flags.selected;
					
					if (inrange (lastwohin, anchor, i))
						l->flags.selected ^= 1;
					
					if (inrange (wohin, anchor, i))
						l->flags.selected ^= 1;
						
					if (l->flags.selected != oldstate)
						if (visible (L, i))
							invertNr (L, l, i);
					
					l = l->next;
				}
			}			
			lastwohin = wohin;
		}
	}
	return 0;
}

void
ListExit (LISTINFO *L)
{
	if (L->lslid)
	{
		SlidDelete(L->lslid);
		L->lslid = NULL;
	}

	if (L->mslid)
	{
		SlidDelete(L->mslid);
		L->mslid = NULL;
	}

	L->lslidx = L->lslidy = L->lslidlen = 0;
	L->mslidx = L->mslidy = L->mslidlen = 0;
}

void
ListStdInit (LISTINFO *L, OBJECT *tree, int box1, int box2, 
	void (*drawfunc)(LISTSPEC *entry, int x, int y, int offset,
	GRECT *cliprect, int how), LISTSPEC *list, int maxwidth, long startindex,
	int selectionservice)
{
	L->boxtree = tree;
	L->boxindex = box1;
	L->boxbgindex = box2;
	L->drawfunc = drawfunc;
	L->windhandle = -1;
	L->list = list;
	L->maxwidth = maxwidth;
	L->startindex = startindex;
	L->selectionservice = selectionservice;
	L->lslidx = L->lslidy = L->lslidlen = 0;
	L->mslidx = L->mslidy = L->mslidlen = L->hpos = 0;
}

void
ListScroll2Selection (LISTINFO *L)
{
	int entry = 0, found = -1;
	LISTSPEC *w;
	
	/* Ist die Liste berhaupt zu lang? */
	if (L->listlength <= L->lines) return;
	
	w = L->list;
	
	while (w && (found == -1))
	{
		if (w->flags.selected) found = entry;
		w = w->next;
		entry++;
	}
	
	if (entry == -1) return;
	
	if ((found >= L->startindex) && (found < L->startindex + L->lines))
		return;
		
	L->startindex = found;

	if (found + L->lines > L->listlength)
		L->startindex = L->listlength - L->lines;	

	slidlpos (L, L->startindex);
}

/* Diese Funktion bewegt die Selektion rauf oder runter und
   wird zB von den tastaturbedienten Listen aufgerufen */

long
ListMoveSelection (LISTINFO *L, int up)
{
	LISTSPEC *w = L->list, *last = NULL;
	long cnt = 0;
	
	while (w)
	{
		if (up)
		{
			if (w->flags.selected)
			{
				if (last)
				{
					int direction = L->startindex > cnt - 1 ? -1 : 1;

					w->flags.selected = 0;
					last->flags.selected = 1;
					
					while (!visible (L, cnt - 1))
						ListVScroll (L, direction);
						
					ListUpdateEntry (L, cnt - 1);
					ListUpdateEntry (L, cnt);
					return cnt - 1;
				}
				
				/* ist bereits am Beginn */
				return 0;
			}
		}
		else
		{
			if (last && last->flags.selected)
			{
				int direction = L->startindex > cnt ? -1 : 1;
			
				last->flags.selected = 0;
				w->flags.selected = 1;

				while (!visible (L, cnt))
					ListVScroll (L, direction);

				ListUpdateEntry (L, cnt - 1);
				ListUpdateEntry (L, cnt);
				
				return cnt;
			}		
		}
	
		last = w;
		cnt++;
		w = w->next;	
	}
	
	return cnt - 1;	/* shouldn't happen */
}


void
ListPgDown (LISTINFO *L)
{
	ListVScroll (L, L->lines);
}

void
ListPgUp (LISTINFO *L)
{
	ListVScroll (L, - L->lines);
}

void
ListPgRight (LISTINFO *L)
{
	ListHScroll (L, L->area.g_w);
}

void
ListPgLeft (LISTINFO *L)
{
	ListHScroll (L, - L->area.g_w);
}

void
ListLnRight (LISTINFO *L)
{
	ListHScroll (L, L->hstep);
}

void
ListLnLeft (LISTINFO *L)
{
	ListHScroll (L, - L->hstep);
}

void
ListVSlide (LISTINFO *L, int pos)
{
	long newpos;
	
	newpos = pos * L->lslid->scale;
	newpos /= 1000;

	ListVScroll (L, newpos - L->startindex);
}

void
ListHSlide (LISTINFO *L, int pos)
{
	long newpos;
	
	newpos = pos * L->mslid->scale;
	newpos /= 1000;

	ListHScroll (L, (int)(newpos - L->hpos));
}
