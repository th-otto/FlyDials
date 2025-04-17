/*
	@(#)FlyDial/dial.c
	
	Julian F. Reschke, 12. Januar 1995
		
	Dial-Functions
	
	AB:
	- Quarter-Screen-bg_buffer feature added
	- DialExStart() added for QSB-support
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flydial/flydial.h"


#define DIALMOVER 0x1100

int DialWk = 0;

DIALMALLOC dialmalloc;
DIALFREE dialfree;

static int work_in[]={1,1,1,1,1,1,1,1,1,1,2};
static int work_out[57];

#if defined(__TOS__) || defined(__atarist__)
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
#endif

static int findmover (OBJECT *t)
{
	int i = 0;

	do
	{
		if ((t[i].ob_type & 0xff00) == 0x1100)
			if ((t[i].ob_state & (CROSSED|OUTLINED)) == (CROSSED|OUTLINED))
				return i;
		if (t[i].ob_flags & LASTOB)
			return -1;

		i++;
	}
	while (1);
}

static void enablemover (OBJECT *t)
{
	int ob;

	ob = findmover (t);
	if (ob != -1)
		t[ob].ob_flags &= (~HIDETREE);
}

static void disablemover (OBJECT *t)
{
	int ob;

	ob = findmover (t);
	if (ob != -1)
		t[ob].ob_flags |= HIDETREE;
}

/*
	1. Gesamtkoordinaten feststellen (unter Bercksichtigung 
	des Randes
	2. Bildschirmplatz reservieren
*/

int DialExStart (OBJECT *TheTree, DIALINFO *D, int useQSB)
{
	int rand, thetype;
	unsigned long size;
	void *where;	

#if __MSDOS__
	long	qsbSize;
	void	*qsbAddr;
	int		w1, w2, w3, w4;
	int		xOffset;
	
#else
	(void)useQSB;
#endif

	D->tree = TheTree;
	D->offset = 0;	
	D->x = TheTree->ob_x;
	D->y = TheTree->ob_y;
	D->w = TheTree->ob_width;
	D->h = TheTree->ob_height;
	thetype = TheTree->ob_type & 0xff;

	if (TheTree->ob_state & OUTLINED)
		D->offset = 3;

	if (TheTree->ob_state & SHADOWED)
	{
#if MSDOS
		D->y -= 2;				/* merkwrdiger Hack fr's GEM/3 */
		D->h += 4;
#else
		D->h += 2;
#endif
		D->w += 2;
	}
	
	if ((thetype == G_BOX)||(thetype == G_IBOX))
	{
		rand = (TheTree->ob_spec.obspec.framesize);
		if (rand < 0)
			D->offset -= rand;
	}
	
	/* Hack fuer Popup-Menus */
	
	if (!D->offset)
	{
		int first_child = TheTree->ob_head;

		if (TheTree[first_child].ob_x == 0)
			D->offset = 1;
	}
	

	D->x -= D->offset; D->y -= D->offset; D->offset <<= 1;
	D->w += D->offset; D->h += D->offset; D->offset >>= 1;

#if __MSDOS__
	/* adjust the dialog background to a word boundary to prevent
	   GEM/3's VGA driver from processing raster plane per plane. */	
	xOffset = D->x & 0x0F;
	D->x -= xOffset;
	D->w += xOffset;	
#endif

	/* calculate size of needed memory */
	size = RastSize (D->w, D->h, &D->bg_buffer);

	D->bg_buffer.fd_addr = where = NULL;
	if (RectOnScreen (D->x, D->y, D->w, D->h))
	{
#if __MSDOS__
		disablemover(TheTree);
		if (useQSB)
		{
			wind_get(0,17,&w1,&w2,&w3,&w4);
			qsbSize = (((long)(unsigned int)w4) << 16) + (long)(unsigned int)w3;
			qsbAddr = MKFP(w2,w1);
			if (qsbSize >= size)
				D->bg_buffer.fd_addr = where = qsbAddr;
		}
#else
		D->bg_buffer.fd_addr = where = dialmalloc (size);
#endif
	}

	if (!where)
	{
#if __MSDOS__
		int ok;

		GrafMouse (M_OFF, NULL);
		D->rdb.planes = D->bg_buffer.fd_nplanes;
		ok = RastDiskSave(&D->rdb,D->x, D->y, D->w, D->h);
		GrafMouse (M_ON, NULL);
		if (ok)
			return 1;
#endif
		disablemover (TheTree);
		form_dial (FMD_START, D->x, D->y, D->w, D->h, D->x, D->y,
			D->w, D->h);
		return 0;
	}
	else
	{
#if !__MSDOS__
		enablemover (TheTree);
#endif
		GrafMouse (M_OFF, NULL);
		RastSave (D->x, D->y, D->w, D->h, 0, 0, &D->bg_buffer);
		GrafMouse (M_ON, NULL);
		return 1;
	}
}

 
int DialStart (OBJECT *TheTree, DIALINFO *D)
{
	/* DialStart may use the quarter screen buffer */
	return DialExStart(TheTree,D,1);
}

/*
	Bildschirmplatz wieder freigeben
*/

void DialEnd (DIALINFO *D)
{
	int xy[4];

	if (!D->bg_buffer.fd_addr)
	{
#if __MSDOS__
		if (D->rdb.saved)
		{
			int ok;

			GrafMouse (M_OFF, NULL);
			RectAES2VDI (D->x, D->y, D->w, D->h, xy);
			vs_clip (HandAES, 1, xy);
			ok = RastDiskRestore(&D->rdb,D->x, D->y, D->w, D->h);
			GrafMouse (M_ON, NULL);
			if (ok)
				return;
		}
#endif
		form_dial(FMD_FINISH, D->x, D->y, D->w, D->h, D->x, D->y,
			D->w, D->h);
	}
	else
	{
		
		RectAES2VDI (D->x, D->y, D->w, D->h, xy);
		GrafMouse (M_OFF, NULL);
		vs_clip (DialWk, 1, xy);
		RastRestore (D->x, D->y, D->w, D->h, 0, 0, &D->bg_buffer);
		vs_clip (DialWk, 0, xy);
#if ! __MSDOS__
		dialfree (D->bg_buffer.fd_addr);
#endif
		GrafMouse (M_ON, NULL);
	}
}



/* Dialogbox ber den Bildschirm bewegen */
#if !__MSDOS__
int DialMove (DIALINFO *D, int sx, int sy, int sw, int sh)
{
	DIALINFO *Df;
	MFDB ObBuf;	
	int Key;
	int ax,ay,oldx,oldy;
	int mx,my,Startx,Starty;
	int ix,iy,iw,ih,wasx,wasy;
	int w,h;
	int Blitter;
	int MyHandle;
	int tmp1;
	int dummy;
	MFDB *bu;
	
	Df = D;
	bu = &Df->bg_buffer;
	w = Df->w;
	h = Df->h;
	MyHandle = HandAES;
	Blitter = HandFast();
	v_opnvwk (work_in, &MyHandle, work_out);
	if (!MyHandle) return 0;

	vswr_mode (MyHandle, MD_XOR);

	vsl_udsty (MyHandle, 0x5555);

	{
		int xy[4];

		RectAES2VDI (sx, sy, sw, sh, xy);
		vs_clip (MyHandle, 1, xy);
	}
	
	if (Df->bg_buffer.fd_addr <= 0L)
	{
		v_clsvwk (MyHandle);
		return 0;
	}
	
	/* ben”tigten Speicherplatz berechnen */
	ObBuf.fd_addr = dialmalloc (RastSize (w, h, &ObBuf));

	if (ObBuf.fd_addr <= 0L) return 0;
	
	graf_mkstate (&Startx, &Starty, &Key, &dummy);

	/* Wenn die rechte Maustaste gedrckt ist */
	
	if (Key & 2)
	{
		GrafMouse (M_OFF, NULL);
		RastSave (Df->x, Df->y, w, h, 0, 0, &ObBuf);
		RastRestore (Df->x, Df->y, w, h, 0, 0, bu);
		RastDotRect (MyHandle, Df->x, Df->y, w, h);
		
		GrafMouse (M_ON, NULL);
		while (Key & 1)
			graf_mkstate (&Startx, &Starty, &Key, &dummy);

		GrafMouse (M_OFF, NULL);
		RastRestore (Df->x, Df->y, w, h, 0, 0, &ObBuf); 
		GrafMouse (M_ON, NULL);
		v_clsvwk(MyHandle);
		dialfree (ObBuf.fd_addr);
		return 1;
	}



	GrafMouse (M_OFF, NULL);
	RastSave (Df->x, Df->y, w, h, 0, 0, &ObBuf); 
	if (!Blitter) RastDotRect (MyHandle, Df->x, Df->y, w, h);
	GrafMouse (M_ON, NULL);


	wasx = oldx = Df->x; wasy = oldy = Df->y;

	
	do
	{
		graf_mkstate (&mx, &my, &Key, &dummy);
		
		ax = mx+(Df->x)-Startx;
		ay = my+(Df->y)-Starty;
		
		if(ax<sx) ax = sx;
		if(ay<sy) ay = sy;
		
		if(ax+w > sx+sw) ax = sx+sw-w;
		if(ay+h > sy+sh) ay = sy+sh-h;
		
		if ((oldx!=ax)||(oldy!=ay)||(!Key))
		{
			GrafMouse (M_OFF, NULL);

			if ((!Blitter)&&(Key))
			{
				RastDotRect (MyHandle,oldx,oldy,w,h);
				RastDotRect (MyHandle,ax,ay,w,h);
			}
			else
			{
				if (!Blitter)
				{
					RastDotRect (MyHandle,oldx,oldy,w,h);
					oldx = wasx;
					oldy = wasy;
				}
		
				if(!RectInter(oldx,oldy,w,h,ax,ay,w,h,&ix,&iy,&iw,&ih))
				{
					RastRestore(oldx,oldy,w,h,0,0,bu);
					RastSave(ax,ay,w,h,0,0,bu);
					RastRestore(ax,ay,w,h,0,0,&ObBuf);
				}
				else
				{
					if (h-ih)	/* vertikale Bewegung */
					{
						tmp1 = ih;
						if(ay>oldy) tmp1 = 0;
						RastRestore(oldx,oldy+tmp1,w,h-ih,0,tmp1,bu);
					}
					
					if (w-iw)
					{
						tmp1 = iw;
						if(ax>oldx) tmp1 = 0;
						RastRestore(oldx+tmp1,iy,w-iw,ih,tmp1,(oldy!=iy)?(h-ih):0,bu);
					}
	
					if((oldx!=ix)&&(oldy!=iy))
						RastBufCopy(w-iw,h-ih,iw,ih,0,0,bu);
					else
						if((oldx==ix)&&(oldy==iy))
							RastBufCopy(0,0,iw,ih,w-iw,h-ih,bu);
						else
							if((oldx==ix)&&(oldy!=iy))
								RastBufCopy(0,h-ih,iw,ih,w-iw,0,bu);
							else
								RastBufCopy(w-iw,0,iw,ih,0,h-ih,bu);
	
					if(h-ih)	/* vertikale Bewegung */
					{
						if (ay>oldy)
							RastSave(ax,oldy+h,w,h-ih,0,ih,bu);
						else
							RastSave(ax,ay,w,h-ih,0,0,bu);
					}
	
					if(w-iw)
					{
						if(ax>oldx)
							RastSave(oldx+w,iy,w-iw,ih,iw,(iy!=oldy)?0:(h-ih),bu);
						else
							RastSave(ax,iy,w-iw,ih,0,(iy!=oldy)?0:(h-ih),bu);
					}
					RastRestore(ax,ay,w,h,0,0,&ObBuf);
				}
			}
			GrafMouse (M_ON, NULL);
			oldx = ax;
			oldy = ay;
		}
	} while (Key);

	dialfree (ObBuf.fd_addr);
	
	Df->x = ax; Df->y = ay;
	Df->tree->ob_x = Df->x + Df->offset;
	Df->tree->ob_y = Df->y + Df->offset;
	v_clsvwk(MyHandle);
	return 1;
}
#endif

int
DialDo (DIALINFO *D, int *start_ob)
{
	return DialXDo (D, start_ob, NULL, NULL, NULL, NULL, NULL);
}


int
DialXDo (DIALINFO *D, int *StartOb, int *ks,
	int *kr, int *mx, int *my, int *mb)	
{
	int but;
	int DoMove = 1;
	int x, y, w, h;
	int Done = 0;
	FORMKEYFUNC merkkey;
	int so = 0;

	if (!StartOb)		/* NULL-Pointer? */
		StartOb = &so;
		
	HandScreenSize (&x, &y, &w, &h);
	WindUpdate (BEG_UPDATE);
	WindUpdate (BEG_MCTRL);
	merkkey = FormGetFormKeybd();
	do
	{
		FormSetFormKeybd (merkkey);
		but = FormXDo (D->tree, StartOb, ks, kr, mx, my, mb);
#if !__MSDOS__
		if ((D->tree[but&0xff].ob_type & 0xff00) == DIALMOVER)
		{
			if (DoMove)
			{
				MFORM TempForm;
				int TempNum;
				
				GrafGetForm (&TempNum, &TempForm);
				GrafMouse (FLAT_HAND, NULL);
				DoMove = DialMove (D, x, y, w, h);
				GrafMouse (TempNum, &TempForm);
			}
		}
		else
#endif
			Done = 1;
	} while (!Done);
	
	WindUpdate (END_MCTRL);
	WindUpdate (END_UPDATE);
	return but;
}


void DialDraw (DIALINFO *D)
{
	ObjcDraw (D->tree, 0, 12, D->x, D->y, D->w, D->h);
}

typedef struct
{
	long	cookie;		/* VSCR */
	long	product;	/* Produktname */
	int		version;
	int		x, y, w, h;	/* sichtbares Fenster */
} INFOVSCR;

static INFOVSCR __tv = {'VSCR','FLYD', 0x100, 0, 319, 640, 320};


void DialCenter (OBJECT *T)
{
	int dummy;
	int ow, oh;

	form_center (T, &dummy, &dummy, &ow, &oh);

#if defined(__TOS__) || defined(__atarist__)
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
#endif
}


/* Flydial-Konfigurations-Variable parsen */

char _fdconf_gem_edit_focus = 1;
char _fdconf_use3d = 1;

static void
parse (char *token)
{
	char *c = strchr (token, '=');
	char hold = '\0';

	if (c) {
		hold = *c;
		*c = '\0';
	}

	if (!strcmp (token, "edit_focus") && c)
	{
		if (!strcmp (c + 1, "tos"))
			_fdconf_gem_edit_focus = 1;
		else if (!strcmp (c + 1, "only_tab"))
			_fdconf_gem_edit_focus = 0;
	}

	if (!strcmp (token, "use_3d") && c)
	{
		if (!strcmp (c + 1, "yes"))
			_fdconf_use3d = 1;
		else if (!strcmp (c + 1, "no"))
			_fdconf_use3d = 0;
	}

	if (c) *c = hold;
}

void
DialSetConf (char *conf)
{
	char *start, *stop;

	/* Defaults setzen */
	
	_fdconf_use3d = _GemParBlk.global[10] >= 4;

	if (!conf) return;

	start = conf;

	while (NULL != (stop = strchr (start, ':')))
	{
		char hold = *stop;
		
		*stop = '\0';
		parse (start);
		*stop = hold;
		start = stop + 1;
	}
	
	parse (start);
}


static int fonts_loaded = 0;

int DialInit (void *mal, void *fre)	/* ”ffnen, falls noch nicht offen */
{
	int dummy;
	extern void ObjcInit (void);

	dialmalloc = mal;
	dialfree = fre;

	if (!DialWk)
	{
		int xy[4];

		HandInit();
		DialWk = HandAES;
		v_opnvwk (work_in, &DialWk, work_out);

		FontAESInfo (DialWk, &fonts_loaded, &HandSFId, &HandSFHeight,
			&HandSIId, &HandSIHeight);

		vst_alignment (DialWk, 0, 5, &dummy, &dummy);

		HandScreenSize (&xy[0], &xy[1], &xy[2], &xy[3]);
		if (DialWk) HandClip (xy[0], xy[1], xy[2], xy[3], 1);
		ObjcInit();
	}

	DialSetConf (getenv ("FLYDIAL_PREFS"));

	return DialWk;
}

void DialExit (void)
{
	extern void ObjcExit (void);

	ObjcExit ();

	if (DialWk)
	{
		if (fonts_loaded)
		{
			vst_unload_fonts (DialWk, 0);
			fonts_loaded = 0;
		}
			
		v_clsvwk (DialWk);
		DialWk = 0;
	}
}

