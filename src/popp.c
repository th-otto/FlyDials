/*
	@(#)FlyDial/popp.c
	@(#)Julian F. Reschke, 6. Oktober 1990
*/


#include <stddef.h>
#include <stdlib.h>

#include "flydial/flydial.h"

#define POPUPDELAY 75
#define DIAGDELAY  120

static int PoppWk;
static OBJECT *PoppTree=NULL;
static int PoppObject=-1;

int _PoppXMouse,_PoppYMouse,_PoppButton;
extern int _PoppMot(int,int);
extern int _PoppBt(int);
extern int ObjcDeepDraw (void);

#define MAXDEEP 4

static GRECT DontReach[MAXDEEP];
static GRECT Reach[MAXDEEP];
static GRECT MenuTitle,MenuBar;
static int RectIndex = 0;

static void getdesktitle (OBJECT *menubar, GRECT *d, GRECT *s)
{
	int i;
	
	i = 0;
	while (!(menubar[i].ob_flags & LASTOB))
	{
		if (menubar[i].ob_state & SELECTED)
		{
			d->g_w = menubar[i].ob_width;
			d->g_h = menubar[i].ob_height;
			ObjcOffset (menubar, i, &(d->g_x), &(d->g_y));
			
			s->g_w = menubar[2].ob_width;
			s->g_h = menubar[2].ob_height;
			ObjcOffset (menubar, 2, &(s->g_x), &(s->g_y));
	
			return;
		}
		i++;
	}
	d->g_w = d->g_h = 0;
	return;
}



static long gettimer (void)
{
	return *((long *)0x4ba);
}

static long timepassed (long oldtime)
{
	return (*((long *)0x4ba)) - oldtime;
}

static int _PoppFind(OBJECT *Tree, int startob, int x, int y)
{
	int ob;

	ob = Tree[startob].ob_head;

	do
	{
		int cx,cy;

		ObjcOffset(Tree,ob,&cx,&cy);
		if(RectInside(cx,cy,Tree[ob].ob_width,Tree[ob].ob_height,x,y))
			return ob;
		ob = Tree[ob].ob_next;
	} while(ob!=startob);
	return -1;
}


static void _PoppPrint (OBJECT *Tree, int obnum, int redraw)
{
	int xoff,yoff;
	int parent;
	static int _PoppHandle ( int x, int y, OBJECT *t, int startob, int ex, 
						int ey, int ew, int eh);

	if (!ObjcDeepDraw())	/* von der Menleiste? */
		if (RectIndex == 0)
			getdesktitle (Tree, &MenuTitle, &MenuBar);

	parent = ObjcGParent (Tree, obnum);
	DontReach[RectIndex].g_w = Tree[parent].ob_width;
	DontReach[RectIndex].g_h = Tree[parent].ob_height;
	ObjcOffset (Tree,parent,&(DontReach[RectIndex].g_x),
		&(DontReach[RectIndex].g_y));
	ObjcOffset (Tree,obnum,&xoff,&yoff);
	Reach[RectIndex].g_x = xoff;
	Reach[RectIndex].g_y = yoff;
	Reach[RectIndex].g_w = Tree[obnum].ob_width;
	Reach[RectIndex].g_h = Tree[obnum].ob_height;

	RectIndex++;


	if ((Tree[obnum].ob_type & 0xff) == G_STRING)
	{
		if(redraw)
		{
			vst_effects(PoppWk,Tree[obnum].ob_state & DISABLED ? 2 : 0);
			v_gtext(PoppWk,xoff,yoff,Tree[obnum].ob_spec.free_string);
			
			if (Tree[obnum].ob_state & CHECKED)
				v_gtext(PoppWk,xoff,yoff,"\010\000");	/* H„kchen */
		}
		else
		{
			int xy[4];

			RectAES2VDI(xoff,yoff,Tree[obnum].ob_width,
				Tree[obnum].ob_height,xy);
			vr_recfl(PoppWk,xy);
		}
	}	
	else if ((Tree[obnum].ob_type & 0xff) == G_USERDEF)
	{
		MENUSPEC *me;
		me = (MENUSPEC *)(Tree[obnum].ob_spec.userblk->ub_parm);

		if (me->me_magic == FLYDIALMAGIC)
		{
			int sx,sy,sw,sh,tx,ty;
			int menupos=0;		/* rechts */

			HandScreenSize(&sx,&sy,&sw,&sh);
			tx = xoff+Tree[obnum].ob_width - (HandXSize);			
			ty = yoff-1 /* - (me->me_sub[me->me_obnum].ob_height >> 3) */;

			if (tx + me->me_sub[me->me_obnum].ob_width >= sx+sw)
				tx = sx+sw - me->me_sub[me->me_obnum].ob_width;
	
			if (tx<= xoff + (HandXSize << 2))
			{
				tx = xoff - me->me_sub[me->me_obnum].ob_width + (HandXSize << 1);
				menupos = 1;
			}
	
			if(tx) tx = (((tx+1) & 0xfff8)-1);
	
			if(redraw)
			{
				char Pfeil[2];
	
				Pfeil[0] = 3+menupos; Pfeil[1] = 0;
				vst_effects(PoppWk,Tree[obnum].ob_state & DISABLED ? 2 : 0);
				v_gtext(PoppWk,xoff,yoff,me->me_title);
				v_gtext(PoppWk,menupos?xoff+(HandXSize>>1):
					xoff+Tree[obnum].ob_width-(HandXSize<<1),yoff,Pfeil);
			}
	
			if((Tree[obnum].ob_state & SELECTED)||
				(!redraw))
			{
				int xy[4];
	
				RectAES2VDI(xoff,yoff,Tree[obnum].ob_width,
					Tree[obnum].ob_height,xy);
				vr_recfl(PoppWk,xy);
			}
			
			if(Tree[obnum].ob_state & SELECTED)
				_PoppHandle(tx,ty,me->me_sub,me->me_obnum,xoff,yoff,
							Tree[obnum].ob_width,Tree[obnum].ob_height);
	
		}
		else
		{
			/* Userblk auswerten */

			PARMBLK p;

			p.pb_tree = Tree;
			p.pb_obj = obnum;
			p.pb_parm = Tree[obnum].ob_spec.userblk->ub_parm;	
			p.pb_prevstate = p.pb_currstate = Tree[obnum].ob_state;
			ObjcXywh (Tree, obnum, (GRECT *) &(p.pb_x));
			p.pb_xc = p.pb_yc = p.pb_wc = p.pb_hc = 0;
			p.pb_parm = 0L;
			Tree[obnum].ob_spec.userblk->ub_code (&p);
		}
	}
	RectIndex--;
}


void PoppInit (void)
{
	int workin[]={1,1,1,1,1,1,1,1,1,1,2};
	int workout[56];
	int dummy;

	HandInit();
	PoppWk = HandAES;
	v_opnvwk (workin,&PoppWk,workout);

	vswr_mode (PoppWk,MD_XOR);
	vst_alignment (PoppWk,0,5,&dummy,&dummy);
	vsf_perimeter (PoppWk,1);
	HandScreenSize (&dummy,&dummy,&dummy,&dummy);
}

void PoppExit (void)
{
	v_clsvwk(PoppWk);
}

static int _Dummy ( void)
{
	return 0;
}

typedef int (*intfunptr)(int);

static intfunptr PoppAESBut;


/*
	Eingabe:
				x,y:	wo soll es erscheinen?
				t:		Zeiger auf Objektbaum
*/
static int _PoppHandle ( int x, int y, OBJECT *t, int startob, int ex, 
						int ey, int ew, int eh)
{
	int merkx,merky;
	int width,height;
	int sx,sy,sw,sh;
	MFDB Buffer;
	unsigned long BufSize;
	int obnum,findob,oldob;
	int mx,my,mb,mbold;
	intfunptr savcod,savcod2;
	intfunptr butcod,butcod2;
	int px,py;
	int inpopup,inpopparent,inother;
	long delay;

	/* Mausstatus merken */

	vq_mouse (HandAES,&mb,&mx,&my);
	
	_PoppButton = mbold = mb;
	_PoppXMouse = mx;
	_PoppYMouse = my;
	vex_motv(HandAES,_PoppMot,&savcod);
	vex_butv(HandAES,_PoppBt,&butcod);
	
	if (butcod != _PoppBt)
		PoppAESBut = butcod;

	/* Popup vollst„ndig sichtbar machen */

	width = t[startob].ob_width + 2;		/* wegen Aussenlinie! */
	height = t[startob].ob_height + 2;
	merkx = t[startob].ob_x;
	merky = t[startob].ob_y;

	HandScreenSize(&sx,&sy,&sw,&sh);
		
	if (x<sx) x = sx;
	if (y<sy) y = sy;

	if (x+width > sx+sw)
		x = sx+sw - width;
	if (y+height > sy+sh)
		y = sy+sh - height;

	/* Fehlermeldung, wenn's gar nicht passt */
	if ((height > sh)||(width > sw)) return 0;

	ObjcOffset(t,startob,&px,&py);
	t[startob].ob_x += (x+1 - px);
	t[startob].ob_y += (y+1 - py);

	v_show_c(HandAES,1);

	delay = gettimer();
		
	while((!(RectInside(x,y,width,height,_PoppXMouse,_PoppYMouse))
		&&(RectInside(ex,ey,ew,eh,_PoppXMouse,_PoppYMouse)))
		&&(timepassed(delay)<POPUPDELAY));

	v_hide_c(HandAES);


	if ((!RectInside(x,y,width,height,_PoppXMouse,_PoppYMouse))&&
		(!RectInside(ex,ey,ew,eh,_PoppXMouse,_PoppYMouse)))
		goto BigSkip;

	BufSize = RastSize(width,height,&Buffer);

	Buffer.fd_addr = dialmalloc(BufSize);
	
	/* Fehlermeldung bei zu wenig Speicher */
	if (! Buffer.fd_addr) return 0;
	
	RastSave(x,y,width,height,0,0,&Buffer);

	{
		int xy[4];
		
		vsf_interior(PoppWk,0);
		vsf_style(PoppWk,1);
		vswr_mode(PoppWk,MD_REPLACE);
		RectAES2VDI(x,y,width,height,xy);
		v_bar(PoppWk,xy);
		vswr_mode(PoppWk,MD_XOR);
		vsf_interior(PoppWk,2);
		vsf_style(PoppWk,8);
	}		


	obnum = t[startob].ob_head;
	do
	{
		t[obnum].ob_state &= (~SELECTED);
		if (!(t[obnum].ob_flags & HIDETREE)) _PoppPrint(t,obnum,1);
		obnum = t[obnum].ob_next;
	} while (obnum!=startob);


	v_show_c(HandAES,1);
	oldob = -1;
	mbold = _PoppButton;
	delay = gettimer();
	
	do
	{
		int i;	/* fr die FOR-Schleife unten */
	
		findob = _PoppFind(t,startob,mx,my);
		if (findob != oldob)
		{
			v_hide_c(HandAES);
			if((oldob!=-1)&&(!(t[oldob].ob_state & DISABLED)))
			{
				t[oldob].ob_state ^= SELECTED;
				_PoppPrint(t,oldob,0);
			}
			oldob = findob;
			if((oldob!=-1)&&(!(t[oldob].ob_state & DISABLED)))
			{
				t[oldob].ob_state ^= SELECTED;
				_PoppPrint(t,oldob,0);
			}
			v_show_c(HandAES,1);
		}
		mx = _PoppXMouse;
		my = _PoppYMouse;
		mb = _PoppButton;
		
		inpopparent = inother = 0;
		
		for (i=0; i<RectIndex; i++)
		{
			inother |= RectInside (DontReach[i].g_x, DontReach[i].g_y,
				DontReach[i].g_w, DontReach[i].g_h, mx, my);
			inpopparent |= RectInside (Reach[i].g_x, Reach[i].g_y,
				Reach[i].g_w, Reach[i].g_h, mx, my);
		}
		inpopup = RectInside (x,y,t[startob].ob_width,t[startob].ob_height,mx,my);

		if (!ObjcDeepDraw())
		{
			inpopparent |= RectInside (MenuTitle.g_x, MenuTitle.g_y,
				MenuTitle.g_w, MenuTitle.g_h, mx, my);
			inother |= RectInside (MenuBar.g_x, MenuBar.g_y,
				MenuBar.g_w, MenuBar.g_h, mx, my);
		}

	} while ( ((mb==mbold) && (inpopup||inpopparent||(!inother)))
			|| (mb!=mbold)&& (inpopparent&&(!inpopup))
			|| (timepassed(delay) < DIAGDELAY)
			);

	v_hide_c(HandAES);
	RastRestore(x,y,width,height,0,0,&Buffer);

	if((mb!=mbold)&&(!(t[findob].ob_state & DISABLED)))
	{
		if(PoppObject == -1)
		{
			PoppTree = t;
			PoppObject = findob;
			t[findob].ob_state &= ~(SELECTED);
			PoppAESBut(1);
			PoppAESBut(0);
		}
	}

BigSkip:
	vex_motv(HandAES,savcod,&savcod2);
	vex_butv(HandAES,butcod,&butcod2);
/*	v_show_c(HandAES,1);
*/
	t[startob].ob_x = merkx;
	t[startob].ob_y = merky;

	dialfree (Buffer.fd_addr);
	return (0);	
}

int cdecl PoppDo (PARMBLK *pb)
{

	_PoppPrint(pb->pb_tree,pb->pb_obj,
				pb->pb_prevstate == pb->pb_currstate);
	return 0;
}

void PoppResult(OBJECT **Tree, int *obj)
{
	*Tree = PoppTree;
	*obj = PoppObject;
	PoppTree = (OBJECT *)0L;
	PoppObject = -1;
}


void PoppChain (OBJECT *ParentTree, int ParentOb,
				OBJECT *SubTree, int ChildOb, MENUSPEC *TMe)
{
	MENUSPEC *Me;

	if(!TMe)
		Me = dialmalloc (sizeof(MENUSPEC));
	else
		Me = TMe;

	Me->me_magic = FLYDIALMAGIC;
	Me->me_sub = SubTree;
	Me->me_obnum = ChildOb;
	Me->me_title = ParentTree[ParentOb].ob_spec.free_string;
	Me->me_ublk.ub_code = PoppDo;
	Me->me_ublk.ub_parm = (long)Me;	
	ParentTree[ParentOb].ob_type = G_USERDEF;
	ParentTree[ParentOb].ob_spec.userblk = &(Me->me_ublk);
}


void PoppUp ( OBJECT *Tree, int x, int y, OBJECT **ResTree, int *resob)
{
	int cx,cy,cw,ch;
	DIALINFO TT;
	int DoMove = 1;
	int mx,my,mb,dummy;
	int ob,lastob=-1;

	{
		int sx,sy,sw,sh;

		HandScreenSize(&sx,&sy,&sw,&sh);
		
		if (x<sx) x = sx;
		if (y<sy) y = sy;

		if (x + Tree->ob_width > sx+sw)
			x = sx+sw - Tree->ob_width;
		if (y + Tree->ob_height > sy+sh)
			y = sy+sh - Tree->ob_height;
	}


	Tree->ob_x = x;
	Tree->ob_y = y;
	DialStart(Tree,&TT);
	DialDraw(&TT);
	HandScreenSize(&cx,&cy,&cw,&ch);
	WindUpdate(BEG_UPDATE);
	WindUpdate(BEG_MCTRL);

	do
	{
		graf_mkstate(&mx,&my,&mb,&dummy);
		ob = objc_find(Tree,0,1,mx,my);

		if(lastob!=ob)
		{
			if (( lastob != -1)&&(Tree[lastob].ob_flags & SELECTABLE))
				objc_change(Tree,lastob,0,cx,cy,cw,ch,
					Tree[lastob].ob_state & (~SELECTED),1);
	
			lastob = ob;
			if ((ob != -1)&&(Tree[ob].ob_flags & SELECTABLE))
				objc_change(Tree,lastob,0,cx,cy,cw,ch,
					Tree[lastob].ob_state | SELECTED,1);

		}

		if( mb && (( Tree[ob].ob_type & 0xff00) == 0x1100))
		{
			if(DoMove)
			{
				int oldx,TempNum;
				MFORM TempForm;

				oldx = Tree->ob_x;
				GrafGetForm (&TempNum, &TempForm);
				GrafMouse (FLAT_HAND, NULL);
				DoMove = DialMove (&TT, cx, cy, cw, ch);
				GrafMouse (TempNum, &TempForm);
				
				if (Tree->ob_x != oldx)
					DialDraw(&TT);
			}
		}
		PoppResult(ResTree,resob);

		if (mb && ((Tree[ob].ob_flags & (SELECTABLE|EXIT)) == 
				(SELECTABLE|EXIT)))
		{
			*ResTree = Tree;
			*resob = ob;
		}
	} while ((*resob == -1) && ((!mb)||(ob!=-1)));

	if (lastob != -1)
		objc_change(Tree,lastob,0,cx,cy,cw,ch,Tree[lastob].ob_state & 
			(~SELECTED),0);

	WindUpdate(END_MCTRL);
	WindUpdate(END_UPDATE);

	DialEnd(&TT);
}
