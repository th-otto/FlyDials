/*
	@(#)MausTausch/fontsel.c
	
	Julian F. Reschke, 30. April 1996
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "flydial/flydial.h"
#include "flydial/fontsel.h"
#include "flydial/listman.h"
#include "flydial/wk.h"


/*
#include "fontrsc.h"
*/

static void messdraw (LISTSPEC *l, int x, int y, int offset,
	GRECT *cliprect, int how)
{
	int dummy, xy[8];
	char *line = ((FONTINFO *)l->entry)->facename;

	RectGRECT2VDI (cliprect, xy);
	vs_clip (DialWk, 1, xy);

	if (l)
	{
		if (how & LISTDRAWREDRAW)
		{
			WkWritingMode (MD_TRANS);
		
			vst_alignment (DialWk, 0, 5, &dummy, &dummy);
			vst_effects (DialWk, 0);
			v_gtext (DialWk, x-offset+HandXSize, y, line);
		}

		if (l->flags.selected || !how & LISTDRAWREDRAW)
		{
			memcpy (xy + 4, xy, 4 * sizeof (int));	
			vro_cpyfm (DialWk, D_INVERT, xy, &RastScreenMFDB, &RastScreenMFDB); 
		}
	}
	vs_clip (DialWk, 0, xy);
}

static void formdraw (LISTSPEC *l, int x, int y, int offset,
	GRECT *cliprect, int how)
{
	int dummy, xy[8];
	char *line = ((FONTINFO *)l->entry)->formname;

	if (!strlen (line)) line = "-";

	RectGRECT2VDI (cliprect, xy);
	vs_clip (DialWk, 1, xy);

	if (l)
	{
		if (how & LISTDRAWREDRAW)
		{
			WkWritingMode (MD_TRANS);

			vst_alignment (DialWk, 0, 5, &dummy, &dummy);
			vst_effects (DialWk, 0);
			v_gtext (DialWk, x-offset+HandXSize, y, line);
		}

		if (l->flags.selected || !how & LISTDRAWREDRAW)
		{
			memcpy (xy + 4, xy, 4 * sizeof (int));	
			vro_cpyfm (DialWk, D_INVERT, xy, &RastScreenMFDB, &RastScreenMFDB); 
		}
	}
	vs_clip (DialWk, 0, xy);
}

static void ptdraw (LISTSPEC *l, int x, int y, int offset,
	GRECT *cliprect, int how)
{
	int dummy, xy[8];
	char line[7];
	
	RectGRECT2VDI (cliprect, xy);
	vs_clip (DialWk, 1, xy);

	if (l)
	{	
		if (how & LISTDRAWREDRAW)
		{
			sprintf (line, " %2d", (int)(l->entry));
			
			WkWritingMode (MD_TRANS);
			
			vst_alignment (DialWk, 0, 5, &dummy, &dummy);
			vst_effects (DialWk, 0);
			v_gtext (DialWk, x-offset+HandXSize, y, line);
		}
		
		if (l->flags.selected || !how & LISTDRAWREDRAW)
		{
			memcpy (xy + 4, xy, 4 * sizeof (int));	
			vro_cpyfm (DialWk, D_INVERT, xy, &RastScreenMFDB, &RastScreenMFDB); 
		}
	}
	vs_clip (DialWk, 0, xy);
}

static LISTSPEC *
FontPointList (FONTWORK *F, int index, int maxpoints)
{
	int attrib[10];
	int i, dummy, where = 0;
	LISTSPEC *sizes;
	int *buffer;
	
	buffer = calloc (maxpoints, sizeof(int));
	if (!buffer) return NULL;
	
	vqt_attributes (F->handle, attrib);
	vst_font (F->handle, index);
	
	i = maxpoints - 1;

	/* Fontlisten nur fr NICHT-FSM-Fonts! */

	if (!FontIsFSM (F, index))
	{
		while (i > 0)
		{
			i = FontSetPoint (F, F->handle, index, i, &dummy, &dummy,
				&dummy, &dummy);
			if (i > 0)
			{
				if (buffer[i]) break;
				buffer[i] = TRUE;
			}
			i--;
		}
	}
	else
	{
		/* Nur Standardgr”žen setzen */

		buffer[9] = buffer[10] = buffer[12] = buffer[14] =
			buffer[18] = buffer[20] = buffer[24] = buffer[27] = TRUE;
	}
	
	sizes = calloc (maxpoints, sizeof(LISTSPEC));
	if (!sizes)
	{
		free (buffer);
		return NULL;
	}

	where = 0;
	for (i = 0; i < maxpoints; i++)
	{
		if (buffer[i])
		{
			sizes[where].entry = (void *)i;
			where++;
		}
	}
	
	free (buffer);
	sizes = realloc (sizes, where * sizeof(LISTSPEC));
	if (sizes)
	{
		for (i = 1; i < where; ++i)
			sizes[i-1].next = &sizes[i];
	}
	vst_font (F->handle, attrib[0]);
	FontSetPoint (F, F->handle, attrib[0], attrib[7], &dummy, &dummy,
		&dummy, &dummy);

	return sizes;
}

static LISTSPEC *
FontFormList (FONTWORK *F, int index)
{
	int i, cnt = 1, first = 0;
	LISTSPEC *forms;
	FONTINFO *f = FontGetEntry (F, index);

	if (f && f->base_id >= 0)
		index = F->list[f->base_id].id;
	
	/* Schnitte z„hlen */
	
	i = 0;
	while (F->list[i].id != -1)
	{
		if (F->list[i].id == index && !F->list[i].flags.famentry)
		{
			first = i;
			break;
		}
		
		i++;
	}
	
	i = first;
	while (F->list[i].nextform)
	{
		cnt++;
		i = F->list[i].nextform;
	}

	forms = calloc (cnt, sizeof(LISTSPEC));
	if (!forms)
		return NULL;

	for (i = 0; i < cnt; i++)
	{
		forms[i].entry = &F->list[first];
		forms[i].flags.selected = 0;
		forms[i].next = NULL;
		if (F->list[first].id == index)
			forms[i].flags.selected = 1;
		if (i) forms[i-1].next = &forms[i];
		first = F->list[first].nextform;
	}
	
	return forms;
}


static LISTSPEC *
FontListmanList (FONTWORK *fw, int select, int prop)
{
	int i,j;	
	int cnt,total;
	LISTSPEC *lp;
	FONTINFO *f = FontGetEntry (fw, select);
	FONTINFO *fp = fw->list;
	
	if (f && f->base_id >= 0)
		select = fp[f->base_id].id;

	cnt = i = 0;
	while (fp[i].id != -1)
	{
		if (!fp[i].flags.famentry)
			if (prop || (!fp[i].flags.isprop)) cnt++;
		i++;
	}
	
	total = i;
	
	lp = calloc (cnt, sizeof(LISTSPEC));
	if (!lp) return NULL;
	
	for (j = i = 0; i < total; i++)
	{
		if (!fp[i].flags.famentry)
		if (prop || (!fp[i].flags.isprop))
		{
			lp[j].flags.selected = 0;
			if (fp[i].id == select) lp[j].flags.selected = 1;
			lp[j].entry = &fp[i];
			lp[j].next = &lp[j+1];
			j++;
		}
	}
	lp[cnt-1].next = NULL;

	return lp;
}

void FontShowFont (FONTWORK *F, OBJECT *tree, int frame, int font,
	int size, char *teststring)
{
	int attrib[10];
	int dummy,x,y;
	int xy[8];
	char buf[256];
	
	vqt_attributes (F->handle, attrib);
	vst_font (F->handle, font);
	FontSetPoint (F, F->handle, font, size, &dummy, &dummy, &dummy, &dummy);
	vst_alignment (F->handle, 0, 3, &dummy, &dummy);
	
	ObjcOffset (tree, frame, &x, &y);
	RectAES2VDI (x, y, tree[frame].ob_width, tree[frame].ob_height,
		xy);
	vs_clip (F->handle, 1, xy);
	GrafMouse (M_OFF, NULL);

	vsf_color (F->handle, 0);
	vsf_interior (F->handle, FIS_SOLID);
	v_bar (F->handle, xy);
#if 0
	memcpy (&xy[4], xy, 4*sizeof(int));	
	vro_cpyfm (F->handle, ALL_WHITE, xy, &screen, &screen); 
#endif

	sprintf (buf, teststring, font, size);

	/* Hack fuer Arnds Treiber */
	v_justified (F->handle, x, y + tree[frame].ob_height - 1,
		buf, 0, 0, 0);	
/*	v_gtext (handle, x, y + tree[frame].ob_height - 1, teststring);	
*/	GrafMouse (M_ON, NULL);
	vs_clip (F->handle, 0, xy);
	vst_font (F->handle, attrib[0]);
	FontSetPoint (F, F->handle, attrib[0], attrib[7], &dummy, &dummy, &dummy, &dummy);
	vst_alignment (F->handle, attrib[3], attrib[4], &dummy, &dummy);
}

void
FontSelectSize (LISTINFO *P, int *size, int redraw)
{
	LISTSPEC *L = P->list;
	int bestfit = 1000;
	long cnt = 0L;
	
	while (L)
	{
		int dist;
		
		dist = abs (*size - (int)L->entry);
		if (dist < bestfit) bestfit = dist;
		L = L->next;
	}

	L = P->list;
	while (L)
	{
		int dist;
		
		dist = abs (*size - (int)L->entry);

		if (dist == bestfit)
		{
			bestfit = 0;	/* no double selection */
			L->flags.selected = 1;
			*size = (int)L->entry;
			if (redraw) 
			{
				ListUpdateEntry (P, cnt);	
				ListVScroll (P, cnt - P->startindex);
			}
		}
		else
		{
			if (L->flags.selected)
			{
				L->flags.selected = 0;
				if (redraw) 
					ListUpdateEntry (P, cnt);	
			}
		}
		
		L = L->next;
		cnt++;
	}
}

int
hammingdist (int cls1, int cls2)
{
	int dist = 0;
	
	dist += (cls1 ^ cls2) & 1;
	dist += (cls1 ^ cls2) & 2;
	dist += (cls1 ^ cls2) & 4;
	dist += (cls1 ^ cls2) & 8;
	dist += (cls1 ^ cls2) & 16;
	dist += (cls1 ^ cls2) & 32;
	
	return dist;
}

void
FontSelectForm (LISTINFO *P, FONTWORK *F, int *index, int redraw)
{
	LISTSPEC *L = P->list;
	long bestfit = 1000000L;
	long cnt = 0L;
	int weight = 6;
	int classification = 0;
	FONTINFO *f = FontGetEntry (F, *index);
	
	if (f)
	{
		weight = f->weight;	
		classification = f->classification;
	}

	while (L)
	{
		int dist;
		
		dist = 256 * abs (weight - ((FONTINFO *)L->entry)->weight);
		dist += hammingdist (classification, ((FONTINFO *)L->entry)->classification);

		if (dist < bestfit) bestfit = dist;
		L = L->next;
	}

	L = P->list;
	while (L)
	{
		int dist;
		
		dist = 256 * abs (weight - ((FONTINFO *)L->entry)->weight);
		dist += hammingdist (classification, ((FONTINFO *)L->entry)->classification);

		if (dist == bestfit)
		{
			bestfit = -1;	/* no double selection */
			L->flags.selected = 1;
			*index = ((FONTINFO *)L->entry)->id;
			if (redraw) 
			{
				ListUpdateEntry (P, cnt);	
				ListVScroll (P, cnt - P->startindex);
			}
		}
		else
		{
			if (L->flags.selected)
			{
				L->flags.selected = 0;
				if (redraw) 
					ListUpdateEntry (P, cnt);	
			}
		}
		
		L = L->next;
		cnt++;
	}
}

int FontSelInit (FONTSELINFO *F, FONTWORK *fw, OBJECT *tree,
				int fontbox, int fontbgbox, int pointbox,
				int pointbgbox, int formbox, int formbgbox,
				int showbox, char *teststring,
				int proportional, int *actfont, int *actsize)
{
	int width = 41 * HandXSize;
	int holdsize;
	
	memset (F, 0, sizeof (FONTSELINFO));
	F->fw = fw;
	F->test = teststring;
	F->tree = tree;
	F->fontbox = fontbox;
	F->fontbgbox = fontbgbox;
	F->pointbox = pointbox;
	F->pointbgbox = pointbgbox;
	F->formbox = formbox;
	F->formbgbox = formbgbox;
	F->showbox = showbox;
	
	if (!F->fw->list)
		return FALSE;
	
	ListStdInit (&F->L, tree, fontbox, fontbgbox, messdraw,
				FontListmanList (F->fw, *actfont, proportional),
				tree[fontbox].ob_width == width ? 0 : width, 0, 1);
	F->L.hstep = 8;
	ListInit (&F->L);
	ListScroll2Selection (&F->L);
	
	ListStdInit (&F->S, tree, formbox, formbgbox, formdraw,
		FontFormList (F->fw, *actfont), 0, 0, 1);
	FontSelectForm (&F->S, F->fw, actfont, FALSE);
	ListInit (&F->S);
	ListScroll2Selection (&F->S);

	ListStdInit (&F->P, tree, pointbox, pointbgbox, ptdraw,
		FontPointList (fw, *actfont, 50), 0, 0, 1);

	holdsize = *actsize;
	FontSelectSize (&F->P, &holdsize, FALSE);
	if (!FontIsFSM (fw, *actfont)) *actsize = holdsize;
	
	ListInit (&F->P);
	ListScroll2Selection (&F->P);
	

	return TRUE;
}

int FontClFont (FONTSELINFO *F, int clicks, int *font, int *size)
{
	LISTSPEC *where;
	int what;
	int holdsize;
	
	holdsize = *size;
	
	if (clicks < 0)
		what = (int)ListMoveSelection (&F->L, clicks == FONT_CL_UP);
	else
		what = (int)ListClick (&F->L, clicks);
	
	if (what >= 0)
	{
		where = ListIndex2List (F->L.list, what);
		what = ((FONTINFO *)where->entry)->id;
	
		if (what != *font)
		{
			free (F->S.list);
			ListExit (&F->S);
			ListStdInit (&F->S, F->tree, F->formbox, F->formbgbox,
						formdraw, FontFormList (F->fw, what),
						0, 0, 1);
			ListInit (&F->S);
			FontSelectForm (&F->S, F->fw, font, FALSE);
			ListDraw (&F->S);	
			
/*			*font = what; */

			free (F->P.list);
			ListExit (&F->P);
			ListStdInit (&F->P, F->tree, F->pointbox, F->pointbgbox,
						ptdraw,	FontPointList (F->fw, *font, 50),
						0, 0, 1);
			ListInit (&F->P);
			FontSelectSize (&F->P, &holdsize, FALSE);
			if (!FontIsFSM (F->fw, *font)) *size = holdsize;
			
			ListScroll2Selection (&F->P);
			ListDraw (&F->P);	
			
			
			FontShowFont (F->fw, F->tree, F->showbox, *font, *size,
					F->test);
		}
	}
	return (int)what;
}

int FontClForm (FONTSELINFO *F, int clicks, int *font, int *size)
{
	LISTSPEC *where;
	int what;
	int holdsize;
	
	holdsize = *size;
	
	if (clicks < 0)
		what = (int)ListMoveSelection (&F->S, clicks == FONT_CL_UP);
	else
		what = (int)ListClick (&F->S, clicks);
	
	if (what >= 0)
	{
		where = ListIndex2List (F->S.list, what);
		what = ((FONTINFO *)where->entry)->id;
	
		if (what != *font)
		{
			*font = what;
			free (F->P.list);
			ListExit (&F->P);
			ListStdInit (&F->P, F->tree, F->pointbox, F->pointbgbox,
						ptdraw,	FontPointList (F->fw, *font, 50),
						0, 0, 1);
			ListInit (&F->P);
			FontSelectSize (&F->P, &holdsize, FALSE);
			if (!FontIsFSM (F->fw, *font)) *size = holdsize;
			
			ListScroll2Selection (&F->P);
			ListDraw (&F->P);	
			
			FontShowFont (F->fw, F->tree, F->showbox, *font, *size,
					F->test);
		}
	}
	return (int)what;
}

int FontClSize (FONTSELINFO *F, int clicks, int *font, int *size)
{
	int what;
	
	if (clicks < 0)
		what = (int)ListMoveSelection (&F->P, clicks == FONT_CL_UP);
	else
		what = (int)ListClick (&F->P, clicks);

	if (what >= 0)
	{
		*size = (int)F->P.list[what].entry;
		FontShowFont (F->fw, F->tree, F->showbox, *font, *size,
					F->test);
	}
	return (int)what;
}

void FontSelDraw (FONTSELINFO *F, int font, int size)
{
	ListDraw (&F->L);
	ListDraw (&F->P);
	ListDraw (&F->S);
	FontShowFont (F->fw, F->tree, F->showbox, font, size,
			F->test);
}

void FontSelExit (FONTSELINFO *F)
{
	free (F->L.list);
	free (F->P.list);
	free (F->S.list);
	ListExit (&F->L);
	ListExit (&F->P);
	ListExit (&F->S);
}

/*
void FontBox (int handle, int *font, int *size)
{
	OBJECT *o = NULL;
	DIALINFO D;
	FONTSELINFO F;	
	int bt,what,dc;

	if (!o)
	{
		rsrc_gaddr (0, 0, &o);
		ObjcTreeInit (o);
	}
	
	DialCenter (o);
	
	FontSelInit(&F, handle, o, FONTBOX, FONTBG, PTBOX, PTBG,
		FONTSHOW, "Dies ist ein Test!", 0, font, size);

	DialStart (o, &D);
	DialDraw (&D);
	FontSelDraw(&F, *font, *size);
	
	do
	{
		dc = 1;
		what = -1;
		bt = DialDo (&D, 0);
		if (bt & 0x8000) dc = 2;
		bt &= 0x7fff;
		
		if ((bt == FONTBG)||(bt == FONTBOX))
			what = FontClFont (&F, dc, font, size);

		if ((bt == PTBG)||(bt == PTBOX))
			what = FontClSize (&F, dc, font, size);
		
	} while ((bt != FONTOK) && ((dc == 1) || (what == -1)));
	
	FontSelExit(&F);
	DialEnd (&D);
}



int main (void)
{
	int font = 1, size = 20;

	rsrc_load("fontrsc.rsc");
	GrafMouse (ARROW, NULL);
	DialInit();
	FontBox (DialWk, &font, &size);
	FontExit();
	FontUnLoad (DialWk);
	DialExit();
	return 0;
}
*/
