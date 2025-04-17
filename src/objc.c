/*
	@(#)FlyDial/objc.c
	
	Julian F. Reschke, 1. Mai 1996
	Objc-Functions
	Double-Brace-Extra-Feature by Stefan Eissing 16.10.1989
*/

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "flydial/flydial.h"
#include "flydial/wk.h"

#if defined(__TOS__) || defined(__atarist__)

#define HIGHBYTE(a) ((int)((char *)(&a))[0])
#define LOWBYTE(a)	((int)((char *)(&a))[1])

#define UDreturn return

#endif

#ifdef __MSDOS__

#define HIGHBYTE(a) ((int)((char *)(&a))[1])
#define LOWBYTE(a)	((int)((char *)(&a))[0])

EXTERN PARMBLK  *fardr_start   _((VOID));
EXTERN VOID     fardr_end      _((WORD));

#include <dos.h>
#define UDreturn fardr_end
#endif

#define G_ANIMIMAGE		42

typedef struct
{
	MFDB	*fdb1,*fdb2;
	int		fontw,fonth;
} CIRCDESC;

static int blk_1_8_16[] = {	0x0000, 0x03C0, 0x0C30, 0x1008, 0x2004,
	0x2004, 0x4002, 0x4002,	0x4002,	0x4002,	0x2004,	0x2004,	0x1008,
	0x0C30,	0x03C0,	0x0000 };
static int blk_2_8_16[] = { 0x0000,	0x03C0,	0x0C30,	0x1188,	0x27E4,
	0x2FF4,	0x4FF2,	0x5FFA,	0x5FFA,	0x4FF2,	0x2FF4, 0x27E4,	0x1188,
	0x0C30,	0x03C0,	0x0000 };
static MFDB n_8_16 = {blk_1_8_16, 16, 16, 1, 0, 1, 0, 0, 0};
static MFDB s_8_16 = {blk_2_8_16, 16, 16, 1, 0, 1, 0, 0, 0};

static int blk_1_7_14[] = { 0x0000, 0x0000, 0x03E0, 0x0410, 0x0808, 
  0x1004, 0x1004, 0x1004, 0x1004, 
  0x1004, 0x0808, 0x0410, 0x03E0, 
  0x0000 };
static int blk_2_7_14[] = { 0x0000, 0x0000, 0x03E0, 0x0410, 0x0BE8, 
  0x17F4, 0x17F4, 0x17F4, 0x17F4, 
  0x17F4, 0x0BE8, 0x0410, 0x03E0, 
  0x0000 };
static MFDB n_7_14 = {blk_1_7_14, 16, 14, 1, 0, 1, 0, 0, 0};
static MFDB s_7_14 = {blk_2_7_14, 16, 14, 1, 0, 1, 0, 0, 0};

static int blk_1_8_8[] = { 0x07E0, 0x381C, 0x4002, 0x8001,
	0x8001,0x4002, 0x381C, 0x07E0 };
static int blk_2_8_8[] = { 0x07E0, 0x381C, 0x47E2, 0xBFFD,
	0xBFFD, 0x47E2, 0x381C, 0x07E0 };
static MFDB n_8_8 = {blk_1_8_8, 16, 8, 1, 0, 1, 0, 0, 0};
static MFDB s_8_8 = {blk_2_8_8, 16, 8, 1, 0, 1, 0, 0, 0};

static cyc_8_16[] = {0x0000, 0x0000, 0x13C0, 0x1C30, 0x1C08,
	0x0008,	0x2004,	0x2004,	0x2004,	0x2004,	0x1000,	0x1038,
	0x0C38,	0x03C8,	0x0000,	0x0000 };
static cyc_7_14[] = {0x0000, 0x0000, 0x0BC0, 0x0C20, 
  0x0E10, 0x1008, 0x1008, 0x1008, 
  0x1008, 0x0870, 0x0430, 0x03D0, 
  0x0000, 0x0000 };
static cyc_8_8[] = {0x27E0, 0x381C, 0x3000, 0x0000, 0x0000,
	0x000C, 0x381C, 0x07E4 };

static MFDB c_8_16 = {cyc_8_16, 16, 16, 1, 0, 1, 0, 0, 0};
static MFDB c_7_14 = {cyc_7_14, 16, 14, 1, 0, 1, 0, 0, 0};
static MFDB c_8_8 = {cyc_8_8, 16, 8, 1, 0, 1, 0, 0, 0};

#if 0
static ud_8_16[] = {0x0000, 0x0180, 0x03C0, 0x07E0, 0x0FF0,
	0x1FF8, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x1FF8, 0x0FF0, 
	0x07E0, 0x03C0, 0x0180, 0x0000 };
static ud_8_8[] = {0x27E0, 0x381C, 0x3000, 0x0000, 0x0000,
	0x000C, 0x381C, 0x07E4 };

static MFDB u_8_16 = {ud_8_16, 16, 16, 1, 0, 1, 0, 0, 0};
static MFDB u_8_8 = {ud_8_8, 16, 8, 1, 0, 1, 0, 0, 0};
#endif

static CIRCDESC circs[] = 
{
	&n_8_8, &s_8_8, 8, 8,
	&n_7_14, &s_7_14, 7, 14,
	&n_8_16, &s_8_16, 8, 16,
};

static CIRCDESC cycles[] =
{
	&c_8_8, &c_8_8, 8, 8,
	&c_7_14, &c_7_14, 7, 14,
	&c_8_16, &c_8_16, 8, 16,
};

#ifdef UPDOWN
static CIRCDESC updowns[] =
{
	&u_8_8, &u_8_8, 8, 8,
	&u_8_16, &u_8_16, 8, 16,
};
#endif

static CIRCDESC thecirc, thecycle, theupdown;

typedef void (*radiofun)(int, int, int, int, int);

static USERBLK CycleBlock = {NULL, 0L};
static USERBLK UpdownBlock = {NULL, 0L};
static USERBLK EditableFrameBlock = {NULL, 0L};

static radiofun __doradio;

static void
blitcirc (int x, int y, int ow, int oh, int state)
{
	int xy[8];
	static int col[] = {1,0}; 
	int w = thecirc.fdb1->fd_w;
	int h = thecirc.fdb1->fd_h;
	
	RectAES2VDI (0, 0, w, h, xy);
	RectAES2VDI (x + ((ow - w) >> 1), y + ((oh - h) >> 1), w, h,
		xy + 4);
	vrt_cpyfm (DialWk, 1, xy, state & SELECTED ? thecirc.fdb2
		: thecirc.fdb1, &RastScreenMFDB, col);
}

static void
normradio (int x, int y, int w, int h, int state)
{
	int xy[4];
	
	RectAES2VDI (x,y,w,h,xy);

	WkFillColor (BLACK);
	WkFillPerimeter (1);
	WkFillInterior (FIS_HOLLOW);

	v_bar (DialWk, xy);

	RectAES2VDI (x+2,y+2,w-4,h-4,xy);

	WkFillInterior (state & SELECTED ? FIS_SOLID : FIS_HOLLOW);
	
	v_bar (DialWk, xy);
}



/* Routinen f�r USERDEF-Objekte */

typedef struct blkentry
{
	long	owner;
	union
	{
		USERBLK	ublk;
		struct blkentry *next;
	} m;
	int		type;
} BLKENTRY;


#define GRANULARITY 		50

static BLKENTRY *blkpool = NULL;

static int
makebigger (void)
{
	BLKENTRY *newblock;
	
	newblock = dialmalloc (GRANULARITY * sizeof(BLKENTRY));

	if (!newblock)
		return FALSE;

	memset (newblock, 0, GRANULARITY * sizeof (BLKENTRY));
	newblock[GRANULARITY - 1].owner = -1L;
	newblock[GRANULARITY - 1].m.next = blkpool;

	blkpool = newblock;

	return TRUE;
}


/* Zeiger auf n�chsten Block holen. Linkage wird ber�cksichtigt */

static BLKENTRY *
nextblk (BLKENTRY *von_wo)
{
	if (von_wo->owner == -1L)
		return (von_wo->m.next);
	else
		return ++von_wo;
}

static BLKENTRY *
getone (void)
{
	BLKENTRY *b = blkpool;
	
	while (b)
	{
		if (!b->owner)
			return b;
		
		b = nextblk (b);
	}
	
	return NULL;
}

static BLKENTRY *
_getblk (OBJECT *ownerTree)
{
	BLKENTRY *b;
	long	 owner;

#if __MSDOS__
	owner = _farptr_tolong(ownerTree);
#else
	owner = (long)ownerTree;
#endif

	b = getone();
	
	if (!b)
	{
		if (!makebigger())
			return NULL;		
		
		b = getone();
	}

	b->owner = owner;
	return b;
}

static void
_clearblks (OBJECT *ownerTree)
{
	long owner;
	BLKENTRY *b = blkpool;

#if __MSDOS__
	owner = _farptr_tolong(ownerTree);
#else
	owner = (long)ownerTree;
#endif

	while (b)
	{
		if (b->owner == owner)
			b->owner = 0L;
		
		b = nextblk (b);
	}
}

void
ObjcExit (void)
{
	BLKENTRY *b = blkpool;
	BLKENTRY *hold;
	
	hold = b;
	
	while (b)
	{
		if (b->owner == -1L)
		{
			b = b->m.next;
			dialfree (hold);
			hold = b;
		}
		else
			b++;
	}
	
	blkpool = NULL;
}


static int cdecl
_UnderString (PARMBLK *p)
{
	if (p->pb_prevstate == p->pb_currstate)
	{
		int xy[4];
		int ext[8];

		HandClip (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc, TRUE);

		WkWritingMode (MD_TRANS);
		v_gtext (DialWk, p->pb_x, p->pb_y, (char *)p->pb_parm);
		vsl_type (DialWk, 1);

		vqt_extent (DialWk, (char *)p->pb_parm, ext);
		RectAES2VDI (p->pb_x, p->pb_y + p->pb_h - 1, ext[2], 1, xy);
/*		RectAES2VDI (p->pb_x, p->pb_y + p->pb_h - 1, p->pb_w, 1, xy); */
		v_pline (DialWk, 2, xy);

/*		vsl_color (DialWk, 1); */
/* 3d
		xy[1] += 1; xy[3] += 1;
		vsl_color (DialWk, 0);
		v_pline (DialWk, 2, xy);
		vsl_color (DialWk, 1);
3d */
		HandClip (0, 0, 0, 0, FALSE);
	}
	UDreturn (p->pb_currstate);
}


/* Callbacks f�r EditableFrame */

typedef int cdecl (*editfun) (OBJECT *, int, int, int, int *, int);

static int cdecl
edit_frame (OBJECT *tree, int ob, int chr, int shift, int *idx, int kind)
{
	(void) idx; (void) shift; (void) chr;

	switch (kind)
	{
		case ED_INIT:
		case ED_END:
			{
				GRECT g;
				
				ObjcXywh (tree, ob, &g);
				
				WkWritingMode (MD_XOR);

				vsl_color (DialWk, 1);
				GrafMouse (M_OFF, NULL);
				RastDrawRect (DialWk, g.g_x - 4, g.g_y - 4,
					g.g_w + 8, g.g_h + 8);
				RastDrawRect (DialWk, g.g_x - 3, g.g_y - 3,
					g.g_w + 6, g.g_h + 6);
				GrafMouse (M_ON, NULL);
			}
			break;
		
		case ED_CHAR:
			return 0;
	}

	return 1;
}

static int cdecl
editable_frame (PARMBLK *p)
{
	if (p->pb_tree == NULL)
	{
		p->pb_tree = (OBJECT *)edit_frame;
		return 0;
	}

	UDreturn (p->pb_currstate);
}


/* Hilfsfunktion f�r wndl.c */

int
ObjcIsUnderlined (OBJECT *tree, short obnum)
{
	if ((tree[obnum].ob_type & 0xff) != G_USERDEF) return FALSE;
		
	return tree[obnum].ob_spec.userblk->ub_code == _UnderString;
}

/* Defaultroutine f�r ANIMBTIBLK-Callback */

int cdecl
ObjcDefaultCallback (ANIMBITBLK *p)
{
	(void) p; return 0;
}


int cdecl
ObjcAnimImage (PARMBLK *p)
{
	int xy[8];
	ANIMBITBLK *A = (ANIMBITBLK *)p->pb_parm;
	BITBLK *B;
	static MFDB im;
	static int col[] = {1,0}; 
	
	HandClip (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc, TRUE);

	A->current += 1;
	if (! A->images[A->current]) A->current = 0;

	B = A->images[A->current];
	im.fd_addr = B->bi_pdata;
	im.fd_w = B->bi_wb << 3;
	im.fd_h = B->bi_hl;
	im.fd_wdwidth = B->bi_wb >> 1;
	im.fd_stand = FALSE;
	im.fd_nplanes = 1;
	col[0] = B->bi_color;
	
	RectAES2VDI (B->bi_x, B->bi_y, B->bi_wb << 3, B->bi_hl, xy);
	RectAES2VDI (p->pb_x, p->pb_y, B->bi_wb << 3, B->bi_hl, xy + 4);
	vrt_cpyfm (DialWk, 1, xy, &im, &RastScreenMFDB, col);

	HandClip (0, 0, 0, 0, FALSE);
	UDreturn (0);
}




#if __MSDOS__
static void cdecl _TitleBox (void) { PARMBLK *p = fardr_start();
#else
static int cdecl _TitleBox (PARMBLK *p) {
#endif
	if (p->pb_prevstate == p->pb_currstate)
	{
		HandClip (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc, TRUE);

		WkWritingMode (MD_REPLACE);

		vsl_color (DialWk, 1);
		RastDrawRect (DialWk, p->pb_x, p->pb_y + (HandYSize	>> 1),
			p->pb_w, p->pb_h - (HandYSize >> 1));
/* 3d
		vsl_color (DialWk, 0);
		RastDrawRect (DialWk, 1 + p->pb_x, 1 + p->pb_y + (HandYSize	>> 1),
			p->pb_w, p->pb_h - (HandYSize >> 1));
		vsl_color (DialWk, 1);
3d */			
		v_gtext (DialWk, p->pb_x + HandXSize, p->pb_y,
			(char *)p->pb_parm);
		HandClip (0, 0, 0, 0, FALSE);
	}
	UDreturn (p->pb_currstate);
}

static int
ParseString (char *thestring)
{
	int index = -1;
	char *cp;
		
	for (cp = thestring; *cp; cp++)
	if (*cp == '[')			/* wir haben ein Klammer */
	{
		strcpy (cp, cp+1);	/* l�sche sie */
		if (*cp != '[')		/* sie war ein Shortcut */
			index = (int)((long)cp - (long)thestring);
		
		cp++;				/* n�chste Zeichen ist egal */
	}
	return index;
}

static void
OutString (int x, int y, char *thestring, int index, int state)
{
	char under[2];

/*	if (state & SELECTED) vst_color (DialWk, 0); */

	vst_effects (DialWk, state & DISABLED ? 2 : 0);
	v_gtext (DialWk, x, y, thestring);

	if (state & CHECKED)
		v_gtext (DialWk, x, y,"\008");		/* Checkmark */

	if (index != -1)
	{
		int ext[8];
		
		under[0] = thestring[index];
		thestring[index] = under[1] = 0;
		
		vst_effects (DialWk, state & DISABLED ? 10 : 8);
		vqt_extent (DialWk, thestring, ext);
		thestring[index] = under[0];
		
		v_gtext (DialWk, x + ext[2], y, under);
	}

#if 0
	if (index != -1)
	{
		under[0] = thestring[index];
		under[1] = 0;

		vst_effects (DialWk, state & DISABLED ? 10 : 8);
		v_gtext (DialWk, x + index * HandXSize, y, under);
	}
#endif

	
/*	if (state & SELECTED)
		vst_color (DialWk, 1); */
}

#if __MSDOS__
void cdecl
ObjcMyButton(void)
{
	PARMBLK *p = fardr_start();
#else
int cdecl
ObjcMyButton (PARMBLK *p)
{
#endif
	int x,y,w,h;		/* tats�chliche Objektposition */
	int sr=0,er=0;		/* Startrand, Endrand */
	int sx,sy;			/* Position f�r den Text */
	int r;
	int flags,state;
	int thepos;
	char thetext[80];

	HandClip (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc, TRUE);

	flags = p->pb_tree[p->pb_obj].ob_flags;
	state = p->pb_currstate;
	x = p->pb_x;
	y = p->pb_y;
	w = p->pb_w;
	h = p->pb_h;
	
	if ((state & CROSSED) && (state != p->pb_prevstate)) /* Tristate? */
	{
		if (! (state & (SELECTED|CHECKED)))
			p->pb_tree[p->pb_obj].ob_state = (state |= CHECKED);
		else
		{
			if ((SELECTED|CHECKED) == (state & (SELECTED|CHECKED)))
				p->pb_tree[p->pb_obj].ob_state =
					(state &= (~(CHECKED|SELECTED)));
		}					
	}

	WkWritingMode (MD_REPLACE);

	strcpy (thetext, (char *)p->pb_parm);
	thepos = ParseString (thetext);

	/* Wenn ich mich recht erinnere, dann ist dies der Sonderfall
	f�r Textobjekte mit Shortcuts */
	
	if (HIGHBYTE (p->pb_tree[p->pb_obj].ob_type) != G_BUTTON)
	{
		OutString (x, y, thetext, thepos, state);	/* XXX */
		vst_effects (DialWk, 0);

		if (state & SELECTED)		/* einfach invertieren */
		{
			int xy[4];
	
			RectAES2VDI (x, y, w, h, xy);
			
			WkWritingMode (MD_XOR);
			
			WkFillColor (BLACK);
			WkFillInterior (FIS_SOLID);
			WkFillPerimeter (0);

			v_bar (DialWk, xy);
		}

		HandClip (0, 0, 0, 0, FALSE);
		UDreturn (0);
	}

	sy = y + ((h - HandYSize) >> 1);

	if (! (flags & EXIT))	/* kein Exit-Button? */
	{
		w = HandXSize << 1;
		sx = x + w + HandXSize;

		w--;
		h--;
	}
	else
	{
		int ext[8];
		
		vqt_extent (DialWk, thetext, ext);
		sx = x + ((w - ext[2]) >> 1);
/*		sx = x + ((w - (HandXSize * (int) strlen (thetext))) >> 1); */
	}

	/* Hintergrund ausgeben */
	{
		int xy[4];
		int DialButtonBGColor = 0;

		RectAES2VDI (x, y, w, h, xy);

		WkFillColor (state & CHECKED ? BLACK : DialButtonBGColor);
		WkFillInterior (state & CHECKED ? FIS_PATTERN : FIS_SOLID);
		WkFillStyle (1);

		v_bar (DialWk, xy);
	}

/*	vswr_mode (DialWk, MD_TRANS);	XXX */
	OutString (sx, sy, thetext, thepos, flags & RBUTTON ? 0 : state);

	/* Umrandung ausgeben */

	if (flags & DEFAULT) er++;
	if (flags & EXIT) er++; else er = sr = 0;
	if (state & OUTLINED) er = sr = 2;

	if ((state & DISABLED) && (!(flags & (EXIT|RBUTTON))))
	{
		vsl_udsty (DialWk, 0x5555);
		vsl_type (DialWk, 7);
	}

	if (!(flags & RBUTTON))
		for (r=sr; r<=er; r++)
			RastDrawRect (DialWk, x-r, y-r, w+(r<<1), h+(r<<1));

	if (!(flags & EXIT))	/* nur bei NICHT-EXIT */
	{
		if (flags & RBUTTON)
			__doradio (x, y, w, h, state);
		else
		{
			if (state & SELECTED)
			{
				int xy[4];
				int swap;				

				RectAES2VDI (x+1, y+1, w-2, h-2, xy);
				v_pline (DialWk, 2, xy);
				swap = xy[0]; xy[0] = xy[2]; xy[2] = swap;
				v_pline (DialWk, 2, xy);
			}
		}
	}
	else /* es ist ein EXIT-Objekt... */
	{
		if (state & SELECTED)		/* einfach invertieren */
		{
			int xy[4];
	
			RectAES2VDI (x+1,y+1,w-2,h-2,xy);
			
			WkWritingMode (MD_XOR);

			WkFillColor (BLACK);
			WkFillInterior (FIS_SOLID);
			WkFillPerimeter (0);

			v_bar (DialWk, xy);
		}
	}
	vst_effects (DialWk, 0);
	HandClip (0, 0, 0, 0, FALSE);

	if ((state & DISABLED) && (!(flags & (EXIT|RBUTTON))))
		vsl_type (DialWk, 1);
	
	if (flags & RBUTTON)
		UDreturn (state & DISABLED);
	else
		UDreturn (0);
}

#if __MSDOS__
static void cdecl ObjcEselsOhr(void) {PARMBLK *p = fardr_start();
#else
static int cdecl ObjcEselsOhr (PARMBLK *p) {
#endif
	int xy[8];
	int x,y,w,h;
	extern char _fdconf_use3d;
	
	HandClip (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc, TRUE);

	WkWritingMode (MD_REPLACE);

	if (!_fdconf_use3d) {
		x = p->pb_x - 2;
		y = p->pb_y - 2;
		w = p->pb_w + 4;
		h = p->pb_h + 4;
			
		RectAES2VDI (x, y, w, h, xy);
	
		vsl_color (DialWk, WHITE);
		RastDrawRect (DialWk, x+1, y+1, w-2, h-2);
		vsl_color (DialWk, BLACK);
		RastDrawRect (DialWk, x+3, y+3, w-6, h-6);
		RastDrawRect (DialWk, x, y-1, w+1, h+1);
		v_pline (DialWk, 2, xy);
	} else {
		x = p->pb_x - 1;
		y = p->pb_y - 1;
		w = p->pb_w + 2;
		h = p->pb_h + 2;
			
		RectAES2VDI (x, y, w, h, xy);

		WkFillColor (LWHITE);		
		v_bar (DialWk, xy);	

		vsl_color (DialWk, LBLACK);
		v_pline (DialWk, 2, xy);
		xy[1] = xy[3] += 1;
		xy[0] += 1;
		v_pline (DialWk, 2, xy);

		vsl_color (DialWk, BLACK);
	}

	HandClip (0, 0, 0, 0, FALSE);
	UDreturn (0);
}

#if __MSDOS__
static void cdecl _3dframe (void) {PARMBLK *p = fardr_start();
#else
static int cdecl _3dframe (PARMBLK *p) {
#endif
	int xy[4];
	extern char _fdconf_use3d;
	
	HandClip (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc, TRUE);

	WkWritingMode (MD_REPLACE);
	WkFillColor (WHITE);

	RectAES2VDI (p->pb_x - 2, p->pb_y - 2, p->pb_w + 4,
		p->pb_h + 4, xy);
	v_bar (DialWk, xy);

	vsl_color (DialWk, BLACK);
	RastDrawRect (DialWk, p->pb_x - 3, p->pb_y - 3,
		p->pb_w + 6, p->pb_h + 6);
	RastDrawRect (DialWk, p->pb_x + 2, p->pb_y + 2,
		p->pb_w - 4, p->pb_h - 4);
	
	if (!_fdconf_use3d) {
		RastDrawRect (DialWk, p->pb_x + 1, p->pb_y + 1,
			p->pb_w - 2, p->pb_h - 2);
	} else {
		int xy[6];
		
		vsl_color (DialWk, LWHITE);
		RastDrawRect (DialWk, p->pb_x - 1, p->pb_y - 1,
			p->pb_w + 2, p->pb_h + 2);
		RastDrawRect (DialWk, p->pb_x, p->pb_y,
			p->pb_w, p->pb_h);

		vsl_color (DialWk, LBLACK);

		xy[0] = p->pb_x - 1; xy[1] = p->pb_y + p->pb_h + 1;
		xy[2] = p->pb_x + p->pb_w + 1; xy[3] = xy[1];
		xy[4] = xy[2]; xy[5] = p->pb_y - 1;
		v_pline (DialWk, 3, xy);

		xy[0] = p->pb_x + 1; xy[1] = p->pb_y + p->pb_h - 3;
		xy[2] = xy[0]; xy[3] = p->pb_y + 1;
		xy[4] = p->pb_x + p->pb_w - 3; xy[5] = xy[3];
		v_pline (DialWk, 3, xy);
	}

	vsl_color (DialWk, BLACK);
	
	HandClip (0, 0, 0, 0, FALSE);
	UDreturn (0);
}


#if __MSDOS__
static void cdecl blitcycle(void) {PARMBLK *p = fardr_start();
#else
static int cdecl blitcycle (PARMBLK *p) {
#endif
	int xy[8];
	static int norm_col[] = {1,0}; 
	static int sel_col[] = {0,1}; 
	int w = thecycle.fdb1->fd_w;
	int h = thecycle.fdb1->fd_h;
	
	HandClip (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc, TRUE);

	RectAES2VDI (p->pb_x - 1, p->pb_y - 1, p->pb_w + 2, p->pb_h + 2,
		xy);

	WkFillColor (BLACK);

	v_bar (DialWk, xy);
	
/*	RectAES2VDI (p->pb_x + ((HandXSize * 2 - w) >> 1),
		p->pb_y + ((HandYSize - h) >> 1), w, h, xy + 4); */

	RectAES2VDI ((w - HandXSize * 2) >> 1,
		(h - HandYSize) >> 1, 2 * HandXSize, HandYSize, xy);
	RectAES2VDI (p->pb_x, p->pb_y, p->pb_w, p->pb_h, xy + 4);
	vrt_cpyfm (DialWk, 1, xy, thecycle.fdb1, &RastScreenMFDB,
		p->pb_currstate & SELECTED ? sel_col : norm_col);
	HandClip (0, 0, 0, 0, FALSE);
	UDreturn (p->pb_currstate & DISABLED);
}

#if __MSDOS__
static void cdecl blitupdown(void) {PARMBLK *p = fardr_start();
#else
static int cdecl blitupdown (PARMBLK *p) {
#endif
	int xy[8];
/*	static int norm_col[] = {1,0}; 
	static int sel_col[] = {0,1}; 
	int w = theupdown.fdb1->fd_w;
	int h = theupdown.fdb1->fd_h;
*/	int cx, cy, cw, ch;
	
	HandClip (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc, TRUE);

	RectAES2VDI (p->pb_x - 1, p->pb_y - 1, p->pb_w + 2, p->pb_h + 2,
		xy);

	WkFillColor (BLACK);

	v_bar (DialWk, xy);
	RectAES2VDI (p->pb_x, p->pb_y, p->pb_w, p->pb_h, xy);

	WkFillColor (WHITE);

	v_bar (DialWk, xy);

	RectInter (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc,
		p->pb_x, p->pb_y, p->pb_w, p->pb_h >> 1, &cx, &cy,
		&cw, &ch);
	HandClip (cx, cy, cw, ch, TRUE);
	v_gtext (DialWk, p->pb_x + ((p->pb_w - HandXSize) >> 1),
		p->pb_y - 1, "\001");
	RectInter (p->pb_xc, p->pb_yc, p->pb_wc, p->pb_hc,
		p->pb_x, p->pb_y + (p->pb_h >> 1), p->pb_w, p->pb_h >> 1,
		&cx, &cy, &cw, &ch);
	HandClip (cx, cy, cw, ch, TRUE);
	v_gtext (DialWk, p->pb_x + ((p->pb_w - HandXSize) >> 1),
		p->pb_y + 1, "\002");
	
	
/*	RectAES2VDI (p->pb_x, p->pb_y, p->pb_w, p->pb_h, &xy[4]);
	RectAES2VDI (0, 0, w, h, xy);
	vrt_cpyfm (DialWk, 1, xy, theupdown.fdb1, &Screen,
		p->pb_currstate & SELECTED ? sel_col : norm_col);
	HandClip (0, 0, 0, 0, FALSE);
*/	UDreturn (p->pb_currstate & DISABLED);
}


extern int DialWk;
static int _ObjectDrawCount = 0;

void ObjcInit (void)
{
	int i;

	makebigger();

	__doradio = normradio;
	
	for (i=0; i < ((sizeof(circs)/sizeof(CIRCDESC))); i++)
	{
		if ((HandXSize >= circs[i].fontw) && (HandYSize >= 
			circs[i].fonth))
		{	
			thecirc = circs[i];
			RastTrans (thecirc.fdb1->fd_addr, thecirc.fdb1->fd_wdwidth << 1,
				thecirc.fdb1->fd_h, DialWk);
			RastTrans (thecirc.fdb2->fd_addr, thecirc.fdb2->fd_wdwidth << 1,
				thecirc.fdb2->fd_h, DialWk);
			__doradio = blitcirc;
		}
	}
	
	for (i = 0; i < ((sizeof (cycles) / sizeof (CIRCDESC))); i++)
	{
			if ((HandXSize >= cycles[i].fontw) && (HandYSize >= 
			cycles[i].fonth))
		{	
			thecycle = cycles[i];
			RastTrans (thecycle.fdb1->fd_addr, thecycle.fdb1->fd_wdwidth << 1,
				thecycle.fdb1->fd_h, DialWk);
			CycleBlock.ub_code = blitcycle;
		}
	}
#ifdef UPDOWN
	for (i = 0; i < ((sizeof (updowns) / sizeof (CIRCDESC))); i++)
	{
			if ((HandXSize == updowns[i].fontw) && (HandYSize == 
			updowns[i].fonth))
		{	
			theupdown = updowns[i];
			RastTrans (theupdown.fdb1->fd_addr,
				theupdown.fdb1->fd_wdwidth << 1,
				theupdown.fdb1->fd_h, DialWk);
			UpdownBlock.ub_code = blitupdown;
		}
	}
#endif UPDOWN
}

/* Initialisiert den Tree (Eintragung der Userdefs etc) */

int
ObjcTreeInit (OBJECT *t)
{
	int i = 0;
	
	while (1)
	{
		/* Shortcut-Objekte: bei Nicht-Exit die Breite anpassen,
		sonst die Objektgr��e ver�ndern */
	
		if (HIGHBYTE (t[i].ob_type) == FD_SHORTCUT)
		{
			if (! (t[i].ob_flags & EXIT))
			{
				size_t len = strlen (t[i].ob_spec.free_string);

				if (NULL != strchr (t[i].ob_spec.free_string, '['))
					len --;

				if ((t[i].ob_type & 0xff) == G_BUTTON)
					len += 3;

				t[i].ob_width = HandXSize * (int) len;
			}
			else
			{
				t[i].ob_y -= 2;
				t[i].ob_height += 4;
			}
		}
		
		/* Shortcut-Objekte: Userdef eintragen und ggfs auf
		Landessprache umsetzen */
		
		if (HIGHBYTE (t[i].ob_type) == FD_SHORTCUT)
		{
			BLKENTRY *U = _getblk(t);
			extern char *ok_string, *abbruch_string;
			
			if (!U) return FALSE;

			if (!strcmp (t[i].ob_spec.free_string, "[OK"))
				t[i].ob_spec.free_string = ok_string;
			else if (!strcmp (t[i].ob_spec.free_string, "[Abbruch"))
				t[i].ob_spec.free_string = abbruch_string;
			
			U->m.ublk.ub_parm = t[i].ob_spec.index;
			U->m.ublk.ub_code = ObjcMyButton;
			U->type = t[i].ob_type;
			t[i].ob_type <<= 8;	/* wird in ObjcMyButton gebraucht */
			t[i].ob_type &= 0xff00;
			t[i].ob_type |= G_USERDEF;
			t[i].ob_spec.userblk = &(U->m.ublk);
		}

		/* Underline-Objekte: Userdef eintragen */

		if (HIGHBYTE (t[i].ob_type) == FD_UNDERLINE)
		{
			BLKENTRY *U = _getblk(t);
			
			if (!U) return FALSE;
			
			t[i].ob_height += 1;
			U->m.ublk.ub_parm = t[i].ob_spec.index;
			U->m.ublk.ub_code = _UnderString;
			U->type = t[i].ob_type;
			t[i].ob_type &= 0xff00;
			t[i].ob_type |= G_USERDEF;
			t[i].ob_spec.userblk = &(U->m.ublk);
		}

		/* Titleline-Objekte: Userdef eintragen */

		if (HIGHBYTE (t[i].ob_type) == FD_TITLELINE)
		{
			BLKENTRY *U = _getblk(t);
			
			if (!U) return FALSE;
			
			U->m.ublk.ub_parm = t[i].ob_spec.index;
			U->m.ublk.ub_code = _TitleBox;
			U->type = t[i].ob_type;
			t[i].ob_type &= 0xff00;
			t[i].ob_type |= G_USERDEF;
			t[i].ob_spec.userblk = &U->m.ublk;
		}

		/* Cycle-Objekte: Objektgr��e anpassen */

		if ((HIGHBYTE (t[i].ob_type) == FD_CYCLEFRAME)
			&& CycleBlock.ub_code)
		{
			if ((t[i].ob_type & 0xff) == G_BOXCHAR)
			{
				t[i].ob_type &= 0xff00;
				t[i].ob_type |= G_USERDEF;
				t[i].ob_spec.userblk = &CycleBlock;
			}
			else
				t[i].ob_width--;
		}
		else
			if (t[i].ob_type == ((FD_CYCLEFRAME << 8)|G_BOXCHAR))
				t[i].ob_spec.obspec.character = 2;

#if UPDOWN
		if ((HIGHBYTE (t[i].ob_type) == 0x18) && UpdownBlock.ub_code)
		{
			if ((t[i].ob_type & 0xff) == G_BOXCHAR)
			{
				t[i].ob_type &= 0xff00;
				t[i].ob_type |= G_USERDEF;
				t[i].ob_spec.userblk = &UpdownBlock;
			}
			else
				t[i].ob_width--;
		}
		else
			if (t[i].ob_type == (0x1800|G_BOXCHAR))
				t[i].ob_spec.obspec.character = 3;
#endif

		/* Stretch: Boxinhalt um 1,5 strecken */

		if (HIGHBYTE (t[i].ob_type) == FD_VSTRETCH)
		{
			t[i].ob_type &= 0x00ff;
			ObjcVStretch (t, i, 3, 2);
		}
		
#if defined(__TOS__) || defined(__atarist__)
		/* Animierte Icons */

		if ((t[i].ob_type & 0xff) == G_ANIMIMAGE)
		{
			BLKENTRY *U = _getblk(t);
			
			if (!U) return FALSE;
			
			U->m.ublk.ub_parm = t[i].ob_spec.index;
			U->m.ublk.ub_code = ObjcAnimImage;
			U->type = t[i].ob_type;
			t[i].ob_type &= 0xff00;
			t[i].ob_type |= G_USERDEF;
			t[i].ob_spec.userblk = &(U->m.ublk);
		}
#endif

		/* Umgeklappte Ecke */

		if (t[i].ob_type == ((FD_ESELSOHR << 8) | G_IBOX))
			if ((t[i].ob_state&(CROSSED|OUTLINED))==(CROSSED|OUTLINED))
			{
				BLKENTRY *U = _getblk(t);
				
				if (!U) return FALSE;
				
				U->m.ublk.ub_parm = t[i].ob_spec.index;
				U->m.ublk.ub_code = ObjcEselsOhr;
				U->type = t[i].ob_type;
				t[i].ob_type &= 0xff00;
				t[i].ob_type |= G_USERDEF;
				t[i].ob_spec.userblk = &(U->m.ublk);
			}

		/* 3D-Rand */

		if (t[i].ob_type == ((FD_3DFRAME << 8) | G_BOX) &&
			t[i].ob_state & OUTLINED)
		{
			BLKENTRY *U = _getblk(t);
				
			if (!U) return FALSE;
				
			U->m.ublk.ub_parm = t[i].ob_spec.index;
			U->m.ublk.ub_code = _3dframe;
			U->type = t[i].ob_type;
			t[i].ob_type &= 0xff00;
			t[i].ob_type |= G_USERDEF;
			t[i].ob_spec.userblk = &(U->m.ublk);
		}

		/* editierbare Rahmenobjekte */

		if (HIGHBYTE (t[i].ob_type) == FD_EDITFRAME)
		{
			t[i].ob_type &= 0xff00;
			t[i].ob_type |= G_USERDEF;
			t[i].ob_spec.userblk = &EditableFrameBlock;
			t[i].ob_flags |= EDITABLE;
			EditableFrameBlock.ub_code = editable_frame;
		}
				
		if (t[i].ob_flags & LASTOB) return TRUE;
		i++;
	}
/*	return TRUE;
*/
}

int
ObjcRemoveTree (OBJECT *t)
{
	int i = 0;
	
	_clearblks(t);
	while (1)
	{
		if ((t[i].ob_type & 0xff) == G_USERDEF)
			if (t[i].ob_spec.userblk->ub_code == ObjcEselsOhr)
			{
				t[i].ob_spec.index = t[i].ob_spec.userblk->ub_parm;
				t[i].ob_type = 0x1119;
			}

		if ((t[i].ob_type & 0xff) == G_USERDEF)
			if (t[i].ob_spec.userblk->ub_code == _TitleBox)
			{
				t[i].ob_spec.index = t[i].ob_spec.userblk->ub_parm;
				t[i].ob_type = (FD_TITLELINE << 8) | G_BUTTON;
			}
		
		if ((t[i].ob_type & 0xff) == G_USERDEF)
			if (t[i].ob_spec.userblk->ub_code == _3dframe)
			{
				t[i].ob_spec.index = t[i].ob_spec.userblk->ub_parm;
				t[i].ob_type = (FD_3DFRAME << 8) | G_BOX;
			}
		
		if ((t[i].ob_type & 0xff) == G_USERDEF)
			if (t[i].ob_spec.userblk->ub_code == ObjcMyButton)
			{
				t[i].ob_spec.index = t[i].ob_spec.userblk->ub_parm;
				t[i].ob_type = (FD_SHORTCUT << 8) | (t[i].ob_type >> 8);	
				
				if (t[i].ob_flags & EXIT)
				{
					t[i].ob_height -= 4;
					t[i].ob_y += 2;
				}
			}

		if ((t[i].ob_type & 0xff) == G_USERDEF)
			if (t[i].ob_spec.userblk->ub_code == _UnderString)
			{
				t[i].ob_spec.index = t[i].ob_spec.userblk->ub_parm;
				t[i].ob_type = 0x1312;
				t[i].ob_height--;
			}

		if (HIGHBYTE (t[i].ob_type) == FD_CYCLEFRAME)
			if ((t[i].ob_type & 0xff) != G_BOXCHAR)
				t[i].ob_width++;

		if (t[i].ob_flags & LASTOB ) return 1;
		i++;
	}
}


int ObjcDeepDraw (void)
{
	return _ObjectDrawCount;
}

int ObjcGParent (OBJECT *Tree, int Obj)
{
	int pobj;

	if (Obj == -1)
		return -1;

	pobj = Tree[Obj].ob_next;

	if (pobj != -1)
	{
		while (Tree[pobj].ob_tail != Obj) 
		{
			Obj = pobj;
			pobj = Tree[Obj].ob_next;
		}
	}
	return (pobj);
} 

int ObjcOffset (OBJECT *Tree, int Num, int *Xoff, int *Yoff)
{
	int sumx = 0, sumy = 0;
	
	do
	{
		sumx += Tree[Num].ob_x;
		sumy += Tree[Num].ob_y;
		Num = ObjcGParent (Tree, Num);
	} while (Num != -1);
	
	*Xoff = sumx;
	*Yoff = sumy;

	return 1;
}

#if THOSE_WERE_THE_DAYS
static void _ObjcOut (OBJECT *tree, int where, int xclip,
				int yclip, int wclip, int hclip)
{
	OBJECT *tt;
	int TheType;

	tt = &(tree[where]);
	TheType = LOWBYTE(tt->ob_type);

	if (( TheType != G_BUTTON) &&
		( TheType != G_ANIMIMAGE))
		objc_draw (tree,where,0,xclip,yclip,wclip,hclip);
	else
	{
		ANIMBITBLK *A = (ANIMBITBLK *)(tt->ob_spec.index);

		A->Current += 1;
		if(A->Images[A->Current] == NULL)
			A->Current = 0;
		tt->ob_spec.index = (long)A->Images[A->Current];
		tt->ob_type = G_IMAGE;
		objc_draw(tree,where,0,xclip,yclip,wclip,hclip);
		tt->ob_spec.index = (long)A;
		tt->ob_type = G_ANIMIMAGE|(FD_SHORTCUT << 8);
	}
}
#endif

int ObjcDraw (OBJECT *tree, int startob, signed int depth, int xclip,
				int yclip, int wclip, int hclip)
{
	return objc_draw (tree, startob, depth, xclip, yclip, wclip, hclip);

#if THOSE_WERE_THE_DAYS
	int where,ow;
	signed d = 0;


	_ObjectDrawCount++;

	v_hide_c (HandAES);
	HandClip (xclip, yclip, wclip, hclip, 1);
	where = startob;
	do
	{		
		ow = where;

		if(tree[where].ob_flags & HIDETREE) goto pommes;
				/* von Arnd getauft */

		if( HIGHBYTE(tree[where].ob_type) != SHORTCUT)
			objc_draw(tree,where,0,xclip,yclip,wclip,hclip);
		else
			_ObjcOut(tree,where,xclip,yclip,wclip,hclip);

		if(tree[where].ob_head != -1)		/* Unterbaum da? */
		{
			d++;							/* Tiefe mitz�hlen */
			where = tree[where].ob_head;
		}
		else
pommes:		
			where = tree[where].ob_next;	/* sonst beim n�chsten weiter */

		while (tree[where].ob_tail == ow)
		{
			d--;
			ow = where;
			where = tree[where].ob_next;
		}
	} while ( (!(tree[ow].ob_flags&LASTOB)) && (depth>d) && (d>0) );
	v_show_c(HandAES,1);
	
	_ObjectDrawCount--;

	HandClip (xclip, yclip, wclip, hclip, 0);
	return 1;
#endif
}

void ObjcXywh (OBJECT *tree, int obj, GRECT *p)
{
        ObjcOffset (tree, obj, &p->g_x, &p->g_y);
        p->g_w = tree[obj].ob_width;
        p->g_h = tree[obj].ob_height;
}

int ObjcChange (OBJECT *tree, int obj, int resvd, int cx, int cy,

	int cw, int ch, int newstate, int redraw)
{
	return objc_change (tree, obj, resvd, cx, cy, cw, ch, newstate, redraw);

#if THOSE_WERE_THE_DAYS
	int Changed;
	int InvertNow = 0;
	int InvertAft = 0;
	
	Changed = (tree[obj].ob_state != newstate);

	if (((tree[obj].ob_type & 0xff) == G_STRING) ||
		((tree[obj].ob_type & 0xff) == G_IMAGE))
	{
		InvertNow = ((tree[obj].ob_state & SELECTED) && 
			(!(newstate & SELECTED))) &&
			(!(tree[obj].ob_flags & HIDETREE));
		InvertAft = ((!(tree[obj].ob_state & SELECTED)) &&
			(newstate & SELECTED)) &&
			(!(tree[obj].ob_flags & HIDETREE));
	}

	if (InvertNow && redraw)
	{
		int xy[4];
		int xoff,yoff;

		ObjcOffset (tree, obj, &xoff, &yoff);
		RectAES2VDI(xoff,yoff,tree[obj].ob_width,
			tree[obj].ob_height,xy);
		v_hide_c (HandAES);

		WkFillColor (BLAC);
		WkWritingMode (MD_XOR);

		vr_recfl(DialWk,xy);
		v_show_c (HandAES, 1);
		newstate &= (~SELECTED);
	}

	if (InvertAft)
		newstate &= (~SELECTED);

	tree[obj].ob_state = newstate;

	if (redraw && Changed)
		ObjcDraw(tree,obj,0,cx,cy,cw,ch);

	if (InvertAft && redraw)
	{
		int xy[4];
		int xoff,yoff;

		ObjcOffset (tree, obj, &xoff, &yoff);
		RectAES2VDI(xoff,yoff,tree[obj].ob_width,
			tree[obj].ob_height,xy);
		v_hide_c (HandAES);
		
		WkFillColor (BLACK);
		WkWritingMode (MD_XOR);

		vr_recfl(DialWk,xy);
		v_show_c (HandAES, 1);
		tree[obj].ob_state |= SELECTED;
	}

	return (resvd&0);		/* Warning verhindern */
#endif
}


void
ObjcWChange (int whandle, OBJECT *tree, int obj, int resvd,
	int cx, int cy, int cw, int ch, int newstate, int redraw)
{
	int old_state;
	int x = 0, y = 0, w = 32767, h = 32767, x1, y1, w1, h1;
	(void) resvd;
	
	old_state = tree[obj].ob_state;
	
	if (whandle >= 0)
		wind_get (whandle, WF_FIRSTXYWH, &x, &y, &w, &h);
	
	while (w && h)
	{
		if (RectInter (x, y, w, h, cx, cy, cw, ch, &x1, &y1, &w1, &h1))
		{
			tree[obj].ob_state = old_state;
			objc_change(tree, obj, 0, x1, y1, w1, h1, newstate, redraw);
		}

		if (whandle >= 0)
			wind_get (whandle, WF_NEXTXYWH, &x, &y, &w, &h);
		else
			w = h = 0;
	}
}


void
ObjcToggle (OBJECT *tree, int obj)
{
	GRECT root;

	ObjcXywh(tree, ROOT, &root);
	ObjcChange(tree, obj, 0, root.g_x, root.g_y, 
                root.g_w, root.g_h, tree[obj].ob_state ^ SELECTED, 1);
}

void ObjcWToggle (int whandle, OBJECT *tree, int obj)
{
	GRECT root;

	ObjcXywh (tree, ROOT, &root);
	ObjcWChange (whandle, tree, obj, 0, root.g_x, root.g_y, 
                root.g_w, root.g_h, tree[obj].ob_state ^ SELECTED, 1);

}

void ObjcWSel (int whandle, OBJECT *tree, int obj)
{
	if (! (tree[obj].ob_state & SELECTED))
		ObjcWToggle (whandle, tree, obj);
}

void ObjcSel (OBJECT *tree, int obj)
{
	if (! (tree[obj].ob_state & SELECTED))
		ObjcToggle (tree, obj);
}

void ObjcWDsel (int whandle, OBJECT *tree, int obj)
{
	if (tree[obj].ob_state & SELECTED)
		ObjcWToggle (whandle, tree, obj);
}

void ObjcDsel (OBJECT *tree, int obj)
{
	if (tree[obj].ob_state & SELECTED)
		ObjcToggle (tree, obj);
}


void ObjcVStretch (OBJECT *tree, int ob, int a, int b)
{
	int obj;
		
	obj = ob;
	ob = tree[ob].ob_head;

	do
	{
		tree[ob].ob_y *= a;
		tree[ob].ob_y /= b;
		ob = tree[ob].ob_next;
	}
	while(ob!=obj);
}

OBSPEC *ObjcGetObspec (OBJECT *tree, int index)
{
	if ((tree[index].ob_type & 0xff) != G_USERDEF)
		return &(tree[index].ob_spec);
	else
		return (OBSPEC *)&(tree[index].ob_spec.userblk->ub_parm);
}
