/*
	@(#)FlyDial/wndl.c
	
	Julian F. Reschke, 27. Oktober 1995
*/

#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "flydial/flydial.h"
#include "flydial/evntevnt.h"
#include "flydial/wndl.h"

/* Standard-Fensterattribute */
#define MODAL_ATTRIBUTES	(NAME|MOVER)

/* Utilityfunktionen */
#define HIGHBYTE(a) ((int)((char *)(&a))[0])
#define LOWBYTE(a)	((int)((char *)(&a))[1])


/* erzeuge ein Fenster, sofern es 'genug' Fenster gibt */

static int
create_window (int kind)
{
	int handle;
	int other_handle;
	int x, y, w, h;
	
	HandScreenSize (&x, &y, &w, &h);
	other_handle = wind_create (kind, x, y, w, h);
	handle = wind_create (kind, x, y, w, h);
	wind_delete (other_handle);
	
	return handle;
}


/* Finde den Titel einer Dialogbox */

static int
find_title (WNDL_INFO *W)
{
	OBJECT *tree = W->tree;
	short max_x = SHRT_MAX, max_y = SHRT_MAX;
	int	title_object = -1, i;
	
	i = 0;

	while (1)
	{
		if (ObjcIsUnderlined (tree, i))
		{
			/* Hier haben wir ein passendes gefunden */
			
			if (tree[i].ob_x < max_x || tree[i].ob_y < max_y)
			{
				title_object = i;
				max_x = tree[i].ob_x;
				max_y = tree[i].ob_y;
			}		
		}
		
		if (tree[i].ob_flags & LASTOB) break;
		
		i++;
	}

	if (title_object != -1)
	{
		OBSPEC *o = ObjcGetObspec (tree, title_object);
		
		W->window_title = dialmalloc (3 +
			strlen ((char *)o->free_string));
		
		if (W->window_title)
		{
			strcpy (W->window_title, " ");
			strcat (W->window_title, o->free_string);
			strcat (W->window_title, " ");
			
			return 1;
		}
		else
		{
			W->window_title = "";
			
			return 0;
		}
	}

	return 1;
}


/* Bereite Dialog vor */

WNDL_INFO *
WndlStart (OBJECT *tree)
{
	WNDL_INFO *W = dialmalloc (sizeof (WNDL_INFO));

	if (W == NULL) return W;

	/* WNDL_INFO-Struktur initialisieren */

	W->tree = tree;	
	W->window_title = "";
	W->vertical_offset = 0;
	W->handle = create_window (MODAL_ATTRIBUTES);

	if (W->handle < 0)
	{
		/* Wenn kein Fenster geîffnet werden konnte */
		
		dialfree (W);
		return NULL;
	}
	
	/* Es konnte ein Fenster geîffnet werden, jetzt muû der
	   Dialog entsprechend angepaût werden */
	
	if (! find_title (W))
	{
		dialfree (W);
		return NULL;
	}
	
	/* Falls Titelzeile gefunden wurde: vertikalen Offset
	   anpassen */
	   
	wind_set (W->handle, WF_NAME, W->window_title);
	
	if (W->window_title[0] != '\0')
		W->vertical_offset = (HandYSize * 5) / 2;

	/* Koordinaten initialisieren */
	
	wind_calc (WC_BORDER, MODAL_ATTRIBUTES, tree->ob_x,
		tree->ob_y + W->vertical_offset, tree->ob_width,
		tree->ob_height - W->vertical_offset, &W->g.g_x,
		&W->g.g_y, &W->g.g_w, &W->g.g_h);

	/* Rand wegschmeissen */
	
	if (LOWBYTE (tree->ob_type) != G_USERDEF)
	{
		W->old_border = tree->ob_spec.index;
		tree->ob_spec.index = 0L;
	}

	return W;
}


/* Dialog beenden */

void
WndlEnd (WNDL_INFO *W)
{
	wind_close (W->handle);
	wind_delete (W->handle);

	if (W->window_title[0] != '\0')
		dialfree (W->window_title);
	
	if (LOWBYTE (W->tree->ob_type) != G_USERDEF)
		W->tree->ob_spec.index = W->old_border;	

	dialfree (W);
}


/* Dialog auf den Bildschirm bringen */

void
WndlOpen (WNDL_INFO *W)
{
	wind_open (W->handle, W->g.g_x, W->g.g_y, W->g.g_w,
		W->g.g_h);
}


/* Standard-Redraw-Funktion */

static void
redraw (WNDL_INFO *W, int x, int y, int w, int h)
{
	int dx, dy, dw, dh;
	int x1, y1, w1, h1;

	WindUpdate (BEG_UPDATE);

	wind_get (W->handle, WF_FIRSTXYWH, &dx, &dy, &dw, &dh);

	while (dw && dh)
	{
		if (RectInter (x, y, w, h, dx, dy, dw, dh, &x1, &y1, &w1,
			&h1))
		{
			objc_draw (W->tree, 0, MAX_DEPTH, x1, y1, w1, h1);
		}			
			
		wind_get (W->handle, WF_NEXTXYWH, &dx, &dy, &dw, &dh);
	}	

	WindUpdate (END_UPDATE);
}


/* Standard-Move-Funktion */

static void
move (WNDL_INFO *W, int x, int y)
{
	int dx, dy, dummy;

	W->g.g_x = x;
	W->g.g_y = y;
	wind_set (W->handle, WF_CURRXYWH, x, y, W->g.g_w,
		W->g.g_h);
	wind_calc (WC_WORK, MODAL_ATTRIBUTES, x, y, W->g.g_w,
		W->g.g_h, &dx, &dy, &dummy, &dummy);
	W->tree->ob_x = dx;
	W->tree->ob_y = dy - W->vertical_offset;
}




/* Events verarbeiten */

int
WndlFeedEvent (WNDL_INFO *W, MEVENT *E, int event_type,
	int *exit_button)
{
	(void)exit_button;
	if (event_type & MU_MESAG)
	{
		switch (E->e_mepbuf[0])
		{
			case WM_REDRAW:
				if (E->e_mepbuf[3] == W->handle)
				{
					event_type &= ~MU_MESAG;
					redraw (W, E->e_mepbuf[4], E->e_mepbuf[5],
						E->e_mepbuf[6], E->e_mepbuf[7]);
				}
				break;
			
			case WM_MOVED:
				if (E->e_mepbuf[3] == W->handle)
				{
					event_type &= ~MU_MESAG;
					move (W, E->e_mepbuf[4], E->e_mepbuf[5]);
				}
				break;

			case WM_TOPPED:
			case WM_NEWTOP:
				if (E->e_mepbuf[3] == W->handle)
				{
					event_type &= ~MU_MESAG;
					wind_set (W->handle, WF_TOP);
				}
				break;
			
			default:
				break;
		}
	}
	
	if (event_type & MU_BUTTON)
	{
		int ob = objc_find (W->tree, 0, 10, E->e_mx, E->e_my);
		
		if (ob >= 0)
		{
			FormWButton	(W->handle, W->tree, ob, E->e_bclk, &ob);
		}
	}
	
	return event_type;	
}

/* Einfaches Frontend fÅr WndlFeedEvent */

short
WndlDo (WNDL_INFO *W)
{
	int exit_button;
	int Done = 0;

	MenuBarEnable (0);
	
	while (!Done)
	{
		static int mbuf[8];
		static MEVENT E = { MU_KEYBD|MU_MESAG|MU_BUTTON, 
			2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			mbuf, 5000L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, NULL, 0L, 0L };
		int which;
		
		which = evnt_event (&E);
		
		which = WndlFeedEvent (W, &E, which, &exit_button);
		
		if (which & MU_KEYBD)
			Done = 1;
	}

	MenuBarEnable (1);
	return 0;
}
