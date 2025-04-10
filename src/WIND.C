/*
	@(#)FlyDial/wind.c
	@(#)Julian F. Reschke, 13. August 1990
	
	Window-Functions
*/

#include <flydial\flydial.h>

static int updateon = 0;
static int msctrl = 0;

void WindUpdate (int mod)
{
	switch (mod)
	{
		case BEG_UPDATE:
			if (!updateon)
				wind_update (BEG_UPDATE);
			updateon++;
			break;

		case END_UPDATE:
			if (updateon==1)
				wind_update (END_UPDATE);
			if (updateon) updateon--;
			break;

		case BEG_MCTRL:
			if (!msctrl)
				wind_update (BEG_MCTRL);
			msctrl++;
			break;

		case END_MCTRL:
			if (msctrl==1)
				wind_update (END_MCTRL);
			if (msctrl) msctrl--;
			break;

		default:	break;
	}
}

void WindRestoreControl (void)
{
	if (updateon)
		wind_update (BEG_UPDATE);
	if (msctrl)
		wind_update (END_MCTRL);
}


/* Fenster auf dem sichtbaren Bildschirm zentrieren */

void
WindCenter (GRECT *g)
{
	static OBJECT tree[] =
	{ 
        -1,        -1,        -1, G_BOX     ,   /* Object 0  */
	  	NONE, SHADOWED, (long)0x00FF1100L,
 	 	0x0000, 0x0000, 0x0034, 0x0009,
 	};
 	
	tree->ob_x = 0;
	tree->ob_y = 0;
	tree->ob_width = g->g_w;
	tree->ob_height = g->g_h;

	DialCenter (tree);
	
	g->g_x = tree->ob_x;
	g->g_y = tree->ob_y;
	g->g_w = tree->ob_width;
	g->g_h = tree->ob_height;
}
