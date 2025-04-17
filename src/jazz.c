/*
	@(#)FlyDial/jazz.c
	
	Julian F. Reschke, 15. Juli 1994
	Jazz-Popups
*/

#include <stddef.h>
#include <stdlib.h>

#include "flydial/flydial.h"

#define MAX(a,b)	(((a)>(b))?(a):(b))
#define MIN(a,b)	(((a)<(b))?(a):(b))

static long
get_ms (void)
{
	long val;

	val = *((volatile long *)0x4ba);
	return val;
}

/* ausgehend von einem Objekt, such das naheste links oder
   rechts davon liegende */
   
static int
goleftright (OBJECT *tree, int ob, int left)
{
	int i;
	int xdist = 32767, found = -1;
	int rx, ry;
	
	ObjcOffset (tree, ob, &rx, &ry);
	
	for (i = 1; !(tree[i-1].ob_flags & LASTOB); i++)
	{
		if (tree[i].ob_flags & SELECTABLE &&
			!(tree[i].ob_state & DISABLED))
		{
			int xpos, ypos, dist;
			
			ObjcOffset (tree, i, &xpos, &ypos);
			
			dist = left ? rx - xpos : xpos - rx;

			if (ry == ypos && dist > 0 && dist < xdist)
			{
				found = i;
				xdist = dist;
			}
		}
	}
	
	return found > -1 ? found : ob;
}


/* ausgehend von einem Objekt, such das naheste links oder
   rechts davon liegende */
   
static int
goupdown (OBJECT *tree, int ob, int up)
{
	int i;
	int ydist = 32767, found = -1;
	int rx, ry;
	
	ObjcOffset (tree, ob, &rx, &ry);
	
	for (i = 1; !(tree[i-1].ob_flags & LASTOB); i++)
	{
		if (tree[i].ob_flags & SELECTABLE &&
			!(tree[i].ob_state & DISABLED))
		{
			int xpos, ypos, dist;
			
			ObjcOffset (tree, i, &xpos, &ypos);
			
			dist = up ? ry - ypos : ypos - ry;

			if (rx == xpos && dist > 0 && dist < ydist)
			{
				found = i;
				ydist = dist;
			}
		}
	}
	
	return found > -1 ? found : ob;
}

static int
lastchild (OBJECT *tree, int ob)
{
	int selob;
	int lastfnd = -1;
	
	/* unsichtbaren Rahmen Åberspringen */
	if ((tree[ob].ob_width == tree->ob_width) && 
		(tree[ob].ob_height == tree->ob_height))
		ob = tree[ob].ob_head;

	selob = ObjcGParent (tree, ob);
	selob = tree[selob].ob_head;
	if (selob == ob) return ob;
	
	do
	{
		if (!(tree[selob].ob_flags & HIDETREE) &&
			!(tree[selob].ob_state & DISABLED) &&
			tree[selob].ob_flags & SELECTABLE) lastfnd = selob;
		
		selob = tree[selob].ob_next;
	} while (selob != ob);

	if (lastfnd != -1)
		return lastfnd;
	else
		return ob;
}


static int
nextchild (OBJECT *tree, int ob)
{
	int selob;
	int parent;
	
	/* unsichtbaren Rahmen Åberspringen */
	if ((tree[ob].ob_width == tree->ob_width) && 
		(tree[ob].ob_height == tree->ob_height))
		ob = tree[ob].ob_head;
	
	parent = ObjcGParent (tree, ob);
	selob = ob;

	do
	{
		selob = tree[selob].ob_next;
		/* noch drin? */
		
		if (selob != parent)
		{
			if (!(tree[selob].ob_flags & HIDETREE) &&
				!(tree[selob].ob_state & DISABLED) &&
				tree[selob].ob_flags & SELECTABLE) return selob;
		}
	} while (selob != parent);

	return ob;
}


/*	limitsize: TRUE, wenn eine maximale Grîûe nicht Åberschritten
	werden soll (beim Aufruf von einer Dialogbox aus), FALSE sonst
	(normales Popup-MenÅ) */

static void
LowUp (OBJECT *Tree, int x, int y, int rel, int cob, int mustbuffer,
	int limitsize, int first_ob, OBJECT **ResTree, int *resob)
{
	int original_height;
	int cx, cy, cw, ch;
	DIALINFO TT;
	int mx,my,mb,dummy;
	int olditem, item, founditem;
	int event;
	int done_by_keyboard;
	int org_x, org_y;

	(void)limitsize;
	org_x = x;
	org_y = y;

	WindUpdate (BEG_MCTRL);
	graf_mkstate (&mx, &my, &mb, &dummy);
	if (rel)
	{
		x += mx;
		y += my;
	}

	if (cob != -1)
	{
		if (rel)
		{
			int tx, ty;

			ObjcOffset (Tree, cob, &tx, &ty);
			x -= (Tree[cob].ob_width >> 1) + tx - Tree->ob_x;
			y -= (Tree[cob].ob_height >> 1) + ty - Tree->ob_y; 
		}
		else
		{
			/* jr: hier wird's etwas empirisch wg. Steves dummer
			Farbpopups */
			
			if (Tree[cob].ob_x >= Tree[cob].ob_width)
				x -= Tree[cob].ob_x;
			y -= Tree[cob].ob_y;
		}
	}

	HandScreenSize (&cx, &cy, &cw, &ch);
	*resob = olditem = -1;
	*ResTree = NULL;
	

	if (x < (cx+1)) x = cx + 1;
	if (y < (cy+1)) y = cy + 1;
	if (x + (Tree->ob_width+3) > cx + cw) x = cx + cw - (Tree->ob_width+3);
	if (y + (Tree->ob_height+3) > cy + ch) y = cy + ch - (Tree->ob_height+3);

	Tree->ob_x = x;
	Tree->ob_y = y;

	original_height = Tree->ob_height;
	(void)original_height;

	if (!DialStart (Tree,&TT))
		if (mustbuffer)
		{
			*resob = -2;
			WindUpdate (END_MCTRL);
			return;
		}
	

	founditem = item = objc_find (Tree, 0, MAX_DEPTH, mx, my);

	event = 0;

	if (item == -1)
	{
		founditem = item = objc_find (Tree, 0, MAX_DEPTH, org_x, org_y);
		
		if (first_ob > 0)
			founditem = item = first_ob;

		if (item != -1) event |= MU_KEYBD;	/* von Tastatur aufge. */
	}

	
	if (item != -1)
		if ((Tree[item].ob_state & DISABLED) || 
			(! (Tree[item].ob_flags & SELECTABLE))) item = -1;

	if (item != -1)
		if (Tree[item].ob_flags & SELECTABLE)
			 Tree[item].ob_state |= SELECTED;	

	
	DialDraw (&TT);
	do
	{
		int kreturn;
		int leave;
		GRECT g;
		
		if (founditem != -1)
		{
			leave = 1;
			ObjcOffset (Tree, founditem, &g.g_x, &g.g_y);
			g.g_w = Tree[founditem].ob_width;
			g.g_h = Tree[founditem].ob_height;
		}				
		else
		{
			leave = 0;
			ObjcOffset (Tree, 0, &g.g_x, &g.g_y);
			g.g_w = Tree[0].ob_width;
			g.g_h = Tree[0].ob_height;
		}
	
		if (event & MU_KEYBD)
		{
			leave = 1;
			g.g_x = mx; g.g_y = my;
			g.g_w = g.g_h = 1;
		}
	
		event = evnt_multi (MU_KEYBD|MU_BUTTON|MU_M1, 1, 1, ~mb & 1, leave,
			g.g_x, g.g_y, g.g_w, g.g_h, 0, 0, 0, 0, 0,
			NULL, 0, 0, &mx, &my, &dummy, &dummy,
			&kreturn, &dummy);
			
		olditem = item;

		done_by_keyboard = 0;		

		if (event & MU_KEYBD)
		{
			int over_item = objc_find (Tree, 0, MAX_DEPTH, mx, my);
		
			if ((kreturn & 0xff00) == 0x5000)
				founditem = item = goupdown (Tree, olditem >= 0 ?
					olditem : over_item >= 0 ? over_item : 1, 0);
			else if ((kreturn & 0xff00) == 0x4800)
				founditem = item = goupdown (Tree, olditem >= 0 ?
					olditem : over_item >= 0 ? over_item : 1, 1);
			else if ((kreturn & 0xff00) == 0x4b00)
				founditem = item = goleftright (Tree, olditem >= 0 ?
					olditem : over_item >= 0 ? over_item : 1, 1);
			else if ((kreturn & 0xff00) == 0x4d00)
				founditem = item = goleftright (Tree, olditem >= 0 ?
					olditem : over_item >= 0 ? over_item : 1, 0);
			else if ((kreturn & 0xff00) == 0x0F00)	/* TAB */
				founditem = item = nextchild (Tree, olditem >= 0 ?
					olditem : over_item >= 0 ? over_item : 1);
			else if ((kreturn & 0xff00) == 0x6100)
				done_by_keyboard = item = -1;
			else if ((kreturn & 0xff) == 13)
				done_by_keyboard = 1;
		}
		else
			founditem = item = objc_find (Tree, 0, MAX_DEPTH, mx, my);
			
		if (item != -1)
			if ((Tree[item].ob_state & DISABLED) || 
				(! (Tree[item].ob_flags & SELECTABLE))) item = -1;

		if ((olditem != item) && (olditem != -1))
			objc_change (Tree, olditem, 0, cx, cy, cw, ch, 
				Tree[olditem].ob_state & (~SELECTED), 1);
		
		if (item != -1)
			objc_change (Tree, item, 0, cx, cy, cw, ch, 
				Tree[item].ob_state | SELECTED, 1);
	} while (! (event & MU_BUTTON) && (!done_by_keyboard));


	DialEnd (&TT);

	if ((~mb & 1) && !done_by_keyboard)
		evnt_button (1, 1, 0, &dummy, &dummy, &dummy, &dummy);

	if (item != -1)
	{
		*ResTree = Tree;
		*resob = item;
	}
	WindUpdate (END_MCTRL);
}


static int
cyclechild (OBJECT *tree, int child)
{
	int ob;
	
	ob = nextchild (tree, child);

	if (ob == child)
	{
		ob = tree->ob_head;
		
		if ((tree[ob].ob_width == tree->ob_width) && 
			(tree[ob].ob_height == tree->ob_height))
			ob = tree[ob].ob_head;
		
		if (tree[ob].ob_flags & HIDETREE || 
			tree[ob].ob_state & DISABLED ||
			!(tree[ob].ob_flags & SELECTABLE))
			ob = nextchild (tree, ob);
	}
	
	return ob;
}


int
JazzSelect (OBJECT *Box, int ob, int txtob, OBJECT *Poppup,
	int docheck, int docycle, long *obs)
{
	int i,x,y;
	int found = -1;
	int ret;
	OBJECT *dummy;
	
	i = 0;
	do
	{
		if (Poppup[i].ob_flags & SELECTABLE)
			Poppup[i].ob_state &= ~SELECTED;

		if (docheck)
			Poppup[i].ob_state &= ~CHECKED;

		if (Poppup[i].ob_spec.index == *obs)
			found = i;
		
		i++;
	} while (!(Poppup[i-1].ob_flags & LASTOB));

	if (found == -1) return -1;
	
	if (docheck)	/* selektieren */
		Poppup[found].ob_state |= CHECKED;

	if (!docycle)
	{
		if (txtob != -1)
			objc_change (Box, txtob, 0, 0, 0, 32767, 32767,
				Box[txtob].ob_state | SELECTED, 1);
		ObjcOffset (Box, ob, &x, &y);
		LowUp (Poppup, x, y, 0, found, 1, 1, found, &dummy, &ret);
		if (txtob != -1)
			objc_change (Box, txtob, 0, 0, 0, 32767, 32767,
				Box[txtob].ob_state & ~SELECTED, 1);
	}
	else
	{
		if (docycle == 1)
			ret = nextchild (Poppup, found);
		else if (docycle == -1)
			ret = lastchild (Poppup, found);
		else
			ret = cyclechild (Poppup, found);
	}
	
	if (ret == -2)
		ret = cyclechild (Poppup, found);

	if (ret != -1)
		*obs = Poppup[ret].ob_spec.index;
	else
		*obs = 0L;

	return ret;
}

void
JazzUp (OBJECT *Tree, int x, int y, int rel, int cob, int mustbuffer,
	OBJECT **ResTree, int *resob)
{
	LowUp (Tree, x, y, rel, cob, mustbuffer, 0, -1, ResTree, resob);
}

int
JazzCycle (OBJECT *Box, int ob, int cyc_button, OBJECT *Poppup,
	int docheck)
{
	long resob;
	int ret;
	int mx, my, mm, mk;
	long was_ms, new_ms;
	
	/* Cycle-Button invertieren, falls notwendig */

	if (!(Box[cyc_button].ob_state & SELECTED))
	{
		objc_change (Box, cyc_button, 0, Box->ob_x, Box->ob_y,
			Box->ob_width, Box->ob_height,
			Box[cyc_button].ob_state | SELECTED,
			TRUE);
	}
	
	resob = Box[ob].ob_spec.index;

again:
	was_ms = Supexec (get_ms);

	ret = JazzSelect (Box, ob, -1, Poppup, docheck, -2, &resob);

	if (resob)
	{
		int x, y;
		
		ObjcOffset (Box, ob, &x, &y);
		Box[ob].ob_spec.index = resob;
/*		objc_draw (Box, ob, 1, x + 1, y + 1,
			Box[ob].ob_width - 2, Box[ob].ob_height - 2);
*/		objc_draw (Box, 0, MAX_DEPTH, x, y,
			Box[ob].ob_width, Box[ob].ob_height);
	}

wait:
	graf_mkstate (&mx, &my, &mm, &mk);
	
	/* Immer noch gedrÅckt? */
	
	if (mm) /*  && (cyc_button == objc_find (Box, 0, 10, mx, my))) */
	{
		new_ms = Supexec (get_ms);
		
		if (new_ms - was_ms < 50L)
			goto wait;
		else
			goto again;
	}

	objc_change (Box, cyc_button, 0, Box->ob_x, Box->ob_y,
		Box->ob_width, Box->ob_height,
		Box[cyc_button].ob_state & ~SELECTED,
		TRUE);

	return ret;
}

int
JazzId (OBJECT *Box, int ob, int txtob, OBJECT *Poppup, int docheck)
{
	int ret;
	long newspec;
	
	newspec = Box[ob].ob_spec.index;

	ret = JazzSelect (Box, ob, txtob, Poppup, docheck, 0, &newspec);
	
	if ((ret != -1) && (newspec != Box[ob].ob_spec.index))
	{
		int x, y;
		
		ObjcOffset (Box, ob, &x, &y);
		Box[ob].ob_spec.index = newspec;
/*		objc_draw (Box, ob, 1, x + 1, y + 1,
			Box[ob].ob_width - 2, Box[ob].ob_height - 2);
*/		objc_draw (Box, 0, MAX_DEPTH, x, y,
			Box[ob].ob_width, Box[ob].ob_height);
	}
	
	return ret;
}
