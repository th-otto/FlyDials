/*
	@(#)FlyDial/menu.c
	
	Julian F. Reschke, 30. April 1996
	Menu-Functions
*/


#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "flydial/flydial.h"
#include "flydial/wk.h"


static int cdecl _MenuLine (PARMBLK *p)
{
	int x, y, xy[4];

	ObjcOffset (p->pb_tree, p->pb_obj, &x, &y);
	
	if (p->pb_wc && p->pb_hc) {
		RectAES2VDI (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc, xy);
		vs_clip (DialWk, 1, xy);
	}

	RectAES2VDI (x, y + (p->pb_h >> 1) - 1, p->pb_w, 2, xy);

	WkWritingMode (MD_TRANS);
	WkFillMode (BLACK, FIS_PATTERN, 0, 4);
	
	vr_recfl (DialWk, xy);

	if (p->pb_wc && p->pb_hc)
		vs_clip (DialWk, 0, xy);

	return 0;
}


static USERBLK _MenuThinLine =  { _MenuLine, 0L };

void MenuSet2ThinLine (OBJECT *tree, int ob)
{
	tree[ob].ob_type = G_USERDEF;
	tree[ob].ob_spec.userblk = &_MenuThinLine;
}

static void
MenuTuneOne (OBJECT *tree, int Parent, int tune)
{
	int Current;

	Current = tree[Parent].ob_head;
	do
	{
		if (tune) tree[Current].ob_width -= HandXSize;
		
		if (tree[Current].ob_spec.free_string[0] == '-')
			MenuSet2ThinLine (tree, Current);
			
		Current = tree[Current].ob_next;
	} while (Current != Parent);
	if (tune) tree[Current].ob_width -= HandXSize;
}

void MenuTune (OBJECT *bar, int tune)
{
	int current,parent;

	parent = bar[bar[0].ob_head].ob_next;
	current = bar[parent].ob_head;

	do
	{
		MenuTuneOne (bar, current, tune);
		current = bar[current].ob_next;
	} while (current != parent);
}

int MenuItemEnable (OBJECT *menu, int item, int enable)
{
	if (enable)
	{
		if (menu[item].ob_state & DISABLED)
			return menu_ienable (menu, item, 1);
		else
			return 1;
	}
	else
	{
		if (!(menu[item].ob_state & DISABLED))
			return menu_ienable (menu, item, 0);
		else
			return 1;
	}
}

static OBJECT *mbar;

int
MenuBar (OBJECT *tree, int show)
{

	if (show < MENUBAR_INSTALL || show > MENUBAR_REDRAW)
		return 0;

	mbar = show == MENUBAR_REMOVE ? NULL : tree;

	if (show != MENUBAR_INSTALL &&
		_GemParBlk.global[0] >= 0x400 &&
		_GemParBlk.global[2] != menu_bar (NULL, -1))
			return 0;
	
	return menu_bar (tree, show == MENUBAR_REMOVE ? 0 : 1);
}

void
MenuBarEnable (int enable)
{
	int current, parent;

	if (!mbar) return;

	parent = mbar[mbar[0].ob_head].ob_head;
	current = mbar[parent].ob_head;

	do
	{
		if ((mbar[current].ob_type & 0xff) == G_TITLE)
			MenuItemEnable (mbar, current, enable);
		
		current = mbar[current].ob_next;
	} while (current != parent);
	
	MenuBar (mbar, MENUBAR_REDRAW);
}
