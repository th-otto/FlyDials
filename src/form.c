/*
	@(#)FlyDial/form.c
	
	Julian F. Reschke, 20. Juli 1998
*/


#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "flydial/flydial.h"
#include "flydial/clip.h"

#define FMD_BACKWARD	-1
#define	FMD_FORWARD		-2
#define FMD_DEFLT		-3

#define HIGHBYTE(a) ((int)((char *)(&a))[0])
#define LOWBYTE(a)	((int)((char *)(&a))[1])

extern char _fdconf_gem_edit_focus;

int _invert_edit;

#if __MSDOS__
static char altKeys[128]=
	{
	0,0,'1','2','3','4','5','6','7','8',			/* 0-9 		*/
	'9','0',0,0,0,0,'Q','W','E','R',				/* 10-19 	*/
	'T','Y','U','I','O','P',0,0,0,0,				/* 20-29 	*/
	'A','S','D','F','G','H','J','K','L',0,			/* 30-39 	*/
	0,0,0,0,'Z','X','C','V','B','N',				/* 40-49 	*/
	'M',0,0,0,0,0,0,0,0,0,							/* 50-59 	*/
	0,0,0,0,0,0,0,0,0,0,							/* 60-69 	*/
	0,0,0,0,0,0,0,0,0,0,							/* 70-79 	*/
	0,0,0,0,0,0,0,0,0,0,							/* 80-89 	*/
	0,0,0,0,0,0,0,0,0,0,							/* 90-99 	*/
	0,0,0,0,0,0,0,0,0,0,							/* 100-109 	*/
	0,0,0,0,0,0,0,0,0,0,							/* 110-119 	*/
	0,0,0,0,0,0,0,0									/* 120-127	*/
	};
#endif

#if defined(__TOS__) || defined(__atarist__)
static long
tosversion (void)
{
	SYSHDR *Sys;

	Sys = *((SYSHDR **)(0x4f2L));
	return Sys->os_version;
}
#endif

static const char *validators = NULL;
static VALFUN *valfuns = NULL;

void FormSetValidator (const char *valchars, VALFUN *funs)
{
	validators = valchars;
	valfuns = funs;
}

typedef int cdecl (*editfun) (OBJECT *, int, int, int, int *, int);


/* Ein Zeichen validieren. Fr die GEM-Templates wird z Zt nur
   ' ' untersttzt */

static int
validate_a_character (OBJECT *tree, int object, int chr, int shift,
	int *idx)
{
	static char blanks_allowed[] = "AaNnX";
	char validat;
	const char *c;
	
	if (validators)
	{
		c = strchr (validators, chr & 0xff);
		
		if (c)
		{
			int fieldnum = (int)(c - validators);
			
			/* Zeichen validieren */
			return valfuns[fieldnum](tree, object, &chr, &shift,
				*idx);
		}
	}

	validat = tree[object].ob_spec.tedinfo->te_pvalid[*idx];
	c = strchr (blanks_allowed, validat);
	
	return c && ' ' == (chr & 0xff);
}


static int
feed_input (OBJECT *tree, int object, int chr, int shift, int *idx,
	int kind, int previously_selected)
{
	if (validators)
	{
		int ac = LOWBYTE (chr);
		char *valstr = tree[object].ob_spec.tedinfo->te_pvalid;
		char *str = tree[object].ob_spec.tedinfo->te_ptext;
		const char *wo;
		char validat;
		int oldpos = *idx;
		
		validat = valstr[oldpos];
		if (!validat) {
			oldpos -= 1;
			validat = valstr[oldpos];
		}
		
		wo = strchr (validators, validat);
		
		if (wo)
		{
			int fieldnum = (int)(wo - validators);
			
			if (previously_selected)
				objc_edit (tree, object, 0x11b, idx, kind);
			
			/* Zeichen validieren */
			if (valfuns[fieldnum](tree, object, &chr, &shift,
				*idx))
			{
				int ret;
				
				/* das Zeichen kann eingegeben werden. Setze als
				   validator X und laž es vom AES machen */
				
				valstr[oldpos] = 'X';
				ret = objc_edit (tree, object, chr, idx, kind);
				valstr[oldpos] = validat;
				return ret;
			}
			else	/* Feldtrenner? */
			{
				char *template = tree[object].ob_spec.tedinfo->te_ptmplt;
				char *adv = template;
				int i;
				
				for (i = 0; i < *idx; i++) {
					while (*adv != '_') adv++;
					adv++;
				}

				wo = strchr (adv, ac);
				
				if (wo)
				{
					int dif = (int)(wo - template);
					int cnt = 0;

					for (i = 0; i < dif; i++)
						if (template[i] == '_') cnt++;
					
					str[*idx] = 0;
					while (strlen (str) < cnt)
					{
						int i = *idx;
						char tmp = valstr[i];
						
						valstr[i] = 'X';
						objc_edit (tree, object, ' ', idx, ED_CHAR);
						valstr[i] = tmp;
					}
					
					return 1;
				}
				else
					chr = 0;
			}
		}
	}

	if (previously_selected)
	{
		switch (chr >> 8)
		{
			case 72:	/* up */
			case 75:	/* left */
			case 77:	/* right */
			case 80:	/* down */
			case 83:	/* delete */
				/* nothing */
				break;
			default:
				objc_edit (tree, object, 0x11b, idx, ED_CHAR);
				break;
		}
	}

	return objc_edit (tree, object, chr, idx, kind);
}


static int
myedit (OBJECT *tree, int object, int chr, int shift, int *idx,
	int kind)
{
	OBJECT *O = tree + object;
	int sc = HIGHBYTE (chr);
	int ac = LOWBYTE (chr);
	char *str;
	int previously_selected = 0;
	
	if (!(O->ob_flags & EDITABLE)) return 1;

	/* Ist es ein Userdef? */
	
	if (LOWBYTE (O->ob_type) == G_USERDEF)
	{
		PARMBLK P;
		editfun E;
		
		P.pb_tree = NULL;
		O->ob_spec.userblk->ub_code (&P);
		E = (editfun)P.pb_tree;
		return E (tree, object, chr, shift, idx, kind);
	}
	
	/* kein Zeichen einfgen? */
	if (kind != ED_CHAR)
		return objc_edit (tree, object, chr, idx, kind);
	
	str = O->ob_spec.tedinfo->te_ptext;

	/* Clipboardbehandlung */
	if (ac == 1 + 'C' - 'A') /* Ctrl-C */
	{
		char tmp[128];
		char *tmplate = O->ob_spec.tedinfo->te_ptmplt;
		char *valid = O->ob_spec.tedinfo->te_pvalid;
		char *text = O->ob_spec.tedinfo->te_ptext;
		char *c = tmp;
		int starting = 1;
		
		*c = '\0';

		while (*tmplate)
		{
			if (!starting || *tmplate == '_')
			{
				starting = 0;		
			
				if (*tmplate != '_')
					*c++ = *tmplate;
				else
				{
					if (*text != ' ' ||
						validate_a_character (tree, object, ' ', 0, idx))
						*c++ = *text;
					
					text += 1;
					valid += 1;
				}
			}
			
			tmplate += 1;
		}
		
		ClipWriteString (tmp, shift & 3);
		return 1;
	}
	
	/* WENN das Editobjekt selektiert ist UND invert_edit gesetzt
	ist UND wir auf Mac-Style-Editing stehen haben, dann fhrt jede
	Tasteneingabe zur Aufhebung der Selektion */

	if ((tree[object].ob_state & SELECTED) &&
		_invert_edit && !_fdconf_gem_edit_focus)
	{
		objc_change (tree, object, 0, 0, 0, 32767, 32767,
			tree[object].ob_state & ~SELECTED, 1);
		previously_selected = 1;
	}

	if (ac == 1 + 'V' - 'A') /* Ctrl-V */
	{
		char thename[128];
		long handle;
		int ret = 1;
		
		if (previously_selected)
			objc_edit (tree, object, 0x11b, idx, ED_CHAR);

		ClipFindFile ("txt", thename);
		handle = Fopen (thename, 0);
		if (handle >= 0)
		{
			size_t i, len = tree[object].ob_spec.tedinfo->te_txtlen;
			
			if (len > sizeof (thename)) len = sizeof (thename);

			len = Fread ((int) handle, len, thename);
			Fclose ((int) handle);
			
			for (i = 0; i < len && ret != 0; i++)
			{
				if (thename[i] == '\r' || thename[i] == '\n')
					break;
			
				ret = feed_input (tree, object, thename[i], 0, idx, kind, 0);
			}
		}
		
		return ret;
	}

	/* Steuerzeichen behandeln */
	switch (sc)
	{
		case 0x4b:	/* shift-links: Beginn der Zeile */
			if (shift & 3)
			{
				int ret = 1;
			
				while (*idx)
					ret = objc_edit (tree, object, 0x4b00, idx, ED_CHAR);
				
				return ret;
			}
			break;
				
		case 0x4d:	/* shift-rechts: Ende der Zeile */
			if (shift & 3)
			{
				objc_edit (tree, object, 0, idx, ED_END);
				return objc_edit (tree, object, 0, idx, ED_INIT);
			}
			break;

		case 0x73: /* ctrl-links: ein Wort zurck */
			if (shift & 4)
			{
				int ret = 1;
				
				while (*idx)
				{
					ret = objc_edit (tree, object, 0x4b00, idx, ED_CHAR);
				 	if ((str[*idx - 1] == ' ') && (str[*idx] != ' '))
				 		break;
				}
				
				return ret;
			}
			break;
			
		case 0x74: /* ctrl-rechts: ein Wort vorw„rts */
			if (shift & 4)
			{
				int ret = 1;

				while (str[*idx])
				{
					ret = objc_edit (tree, object, 0x4d00, idx, ED_CHAR);
				 	if ((str[*idx - 1] == ' ') && (str[*idx] != ' '))
				 		break;
				}
				
				return ret;
			}
			break;
		
		case 0x53:	/* ctrl-delete: L”schen bis Ende der Zeile */
			if (shift & 4)
			{
				while (str[*idx])
					objc_edit (tree, object, chr, idx, ED_CHAR);

				return 1;
			}
			break;

	}

	return feed_input (tree, object, chr, shift, idx, kind,
		previously_selected);
}

#ifdef THOSE_WERE_THE_DAYS
static void
getalt (int *key)
{
	int c = 0;

	*key = 0;
	do
	{
		c = evnt_keybd ();
		if (c == 0x5230) break;
		*key *= 10;
		*key += ((c & 0xff) - '0');
	} while (TRUE);

	*key &= 0xff;
}
#endif


static int
lastob (OBJECT *tree)
{
	int i = 0;
	
	while (! (tree[i].ob_flags & LASTOB)) i++;
	
	return i;
}

static int
watchbox (int whandle, OBJECT *tree, int obj, int instate,
	int outstate)
{
	int cx, cy, cc, dummy;
	GRECT rec;
	int inside = 0, hold, newstate;
	
	ObjcXywh (tree, obj, &rec);
	cc = 1;
	cx = rec.g_x;
	cy = rec.g_y;

	do
	{
		hold = inside;
		if ((cx>=rec.g_x)&&(cy>=rec.g_y)&&(cx<(rec.g_x+rec.g_w))
					&&(cy<(rec.g_y+rec.g_h)))
			inside = 1;
		else
			inside = 0;

		if (hold != inside)
		{
			if (inside)
				newstate = instate;
			else
				newstate = outstate;

			ObjcWChange (whandle, tree, obj, 0, rec.g_x, rec.g_y, rec.g_w,
				rec.g_h, newstate, 1);
		}		
		graf_mkstate (&cx, &cy, &cc, &dummy);
	} while (cc);
	
	return inside;
}


#if defined(__TOS__) || defined(__atarist__)
static int
getkey (OBJECT *tree, int startob, unsigned int which)
{
	KEYTAB *KB;
	unsigned char TheKey;
	int obj;
	char *text;
	
	if (which == 0x6200)	/* HELP-Taste */
	{
		obj = 0;

		while (obj >= 0)
		{
			if (HIGHBYTE (tree[obj].ob_type) == FD_HELPKEY)
				return obj;

			if (tree[obj].ob_flags & LASTOB)
				obj = -1;
			else
				obj++;
		}
	}
#if 0
	if (LOWBYTE (which) == 3)	/* CTRL-C */
	{
		obj = 0;

		while (obj >= 0)
		{
			if (HIGHBYTE (tree[obj].ob_type) == FD_CUT)
				return obj;

			if (tree[obj].ob_flags & LASTOB)
				obj = -1;
			else
				obj++;
		}
	}
#endif

	which = ((which>>8)&0xff);	
	KB = Keytbl ((void *)-1L,(void *)-1L,(void *)-1L);

	if (which >= 0x78)
	{
		TheKey = which-0x47;
		if (TheKey == '9'+1) TheKey = '0';
	}	
	else
		TheKey = ((char *)KB->unshift)[which];
	
	TheKey = toupper(TheKey);
	obj = 0;
	while (obj >= 0)
	{
		if (LOWBYTE (tree[obj].ob_type) == G_USERDEF)
		{
			if (tree[obj].ob_spec.userblk->ub_code == ObjcMyButton)
			{
				text = (char *)tree[obj].ob_spec.userblk->ub_parm;
				text = strchr (text, '[');
				if (text && (text[1]!='['))
					if (toupper(text[1]) == TheKey)
						return obj;
			}
		}
		if (tree[obj].ob_flags & LASTOB)
			obj = -1;
		else
			obj++;
	}
	return startob;
}
#else
static int getkey (OBJECT *tree, int startob, unsigned int which)
{
	unsigned char TheKey;
	int obj;
	char *text;

	if (which == 0x6200)	/* HELP-Taste */
	{
		obj = 0;
		while (obj >= 0)
		{
			if( ((tree[obj].ob_type)&0xff00) == 0x1500)
				 return obj;

			if (tree[obj].ob_flags & LASTOB)
				obj = -1;
			else
				obj++;
		}
	}

	TheKey = which >> 8;
	if (TheKey <= 127)
		{
		TheKey = altKeys[TheKey];
		TheKey = toupper(TheKey);
		}
	else
		TheKey = 0;

	obj = 0;
	while (obj >= 0)
	{
		if ((tree[obj].ob_type & 0xff) == G_USERDEF)
		{
			if (tree[obj].ob_spec.userblk->ub_code == ObjcMyButton)
			{
				text = (char *)tree[obj].ob_spec.userblk->ub_parm;
				text = strchr (text, '[');
				if (text && (text[1]!='['))
					if (toupper(text[1]) == TheKey)
						return obj;
			}
		}
		if (tree[obj].ob_flags & LASTOB)
			obj = -1;
		else
			obj++;
	}
	return (startob);
}
#endif


#if defined(__TOS__) || defined(__atarist__)
static int
findanim (OBJECT *tree)
{
	int i = 0;
	
	do
	{
		if (tree[i].ob_flags & LASTOB)
			return -1;

		if ((tree[i].ob_type & 0xff) == G_USERDEF)
			if (tree[i].ob_spec.userblk->ub_code == ObjcAnimImage)
				return i;

		i++;
	} while(1);
}
#endif

static int
findobj (OBJECT *tree, int startob, int which)
{
	int obj, theflag, flag, inc;
	
	obj = 0; inc = 1; flag = EDITABLE;
	
	switch (which)
	{
		case FMD_BACKWARD:
			inc = -1;	/* kein Break: Absicht */

		case FMD_FORWARD:
			obj = startob + inc;
			
			/* jr: wenn startob schon LASTOB ist, mssen wir hier abbrechen!!! */
			
			if (inc > 0)
				if (tree[startob].ob_flags & LASTOB) return startob;
			
			break;

		case FMD_DEFLT:
			flag = DEFAULT;
			break;
	}	

	while (obj >= 0)
	{
		theflag = tree[obj].ob_flags;
		
		if (theflag & flag) return obj;

		if (theflag & LASTOB)
			obj = -1;
		else
			obj += inc;
	}
	
	return startob;
}


static int
inifld (OBJECT *tree, int startfld)
{
	if (!startfld) startfld = findobj (tree, 0, FMD_FORWARD);
	return startfld;
}

static void
doradio (int whandle, OBJECT *tree, int obj)
{
	int sobj;
	int pobj = ObjcGParent (tree, obj);

	for (sobj = tree[pobj].ob_head; sobj != pobj; sobj = tree[sobj].ob_next)
	{
		if (sobj != obj)
			if (tree[sobj].ob_flags & RBUTTON)
				ObjcWDsel (whandle, tree, sobj);
	}

	ObjcWSel (whandle, tree, obj);
}

int
FormWButton (int whandle, OBJECT *tree, int obj, int clicks,
	int *nextobj)
{
	int flags, state, texit, slbe, dsbld, edit, hibit;

	flags = tree[obj].ob_flags;
	state = tree[obj].ob_state;
	texit = flags & TOUCHEXIT;
	slbe = flags & SELECTABLE;
	dsbld = state & DISABLED;
	edit = flags & EDITABLE;
	
	if (!texit && (!slbe || dsbld) && !edit) {
		*nextobj = 0;
		return 1;
	}

	if (texit && clicks == 2)
		hibit = 0x8000;
	else
		hibit = 0x0;

	if (slbe && !dsbld)					/* Selectable und nicht */
	{									/* disabled... */
		if (flags & RBUTTON)
			doradio (whandle, tree, obj);
		else							/* kein Radio-Button */
			if (!texit)
			{
				if (!watchbox (whandle, tree, obj, state^SELECTED, state))
				{
					*nextobj = 0;
					return 1;
				} 
			}
			else
				ObjcWToggle (whandle, tree, obj);
	}

	if (texit || (flags & EXIT))
	{
		*nextobj = obj|hibit;
		return 0;
	}
	else
		if (!edit) *nextobj = 0;

	return 1;
}

int
FormButton (OBJECT *tree, int obj, int clicks, int *nextobj)
{
	return FormWButton (-1, tree, obj, clicks, nextobj);
}



int
FormKeybd (OBJECT *tree, int edit_obj, int next_obj, int kr, int shift,
	int *onext_obj, int *okr)
{
	int co,tmp;
	int dummy = next_obj;
	int sc = (kr & 0xff00) >> 8;

	if (sc == 0x72 || sc == 0x1C)		/* Return */
	{
		co = findobj (tree, edit_obj, FMD_DEFLT);
		*onext_obj = co;
		*okr = 0;

		if (edit_obj != co)
		{
			FormButton (tree, co, 1, &dummy);
			return 0;
		}
		sc = 15;	/* als Tab nehmen */
	}

	if (sc == 0x47)	/* ClrHome */
	{
		if (shift & 3)
		{
			int merkob = lastob (tree);
			
			co = findobj (tree, merkob, FMD_BACKWARD);
			
			/* falls nix gefunden... */
			
			if (co == merkob) co = *onext_obj;
		}
		else
			co = findobj (tree, 0, FMD_FORWARD);
		
		*onext_obj = co;
		*okr = 0;
		return 1;
	}

	/* Tab? */
	if ( (sc == 15 && !(shift & 7)) ||
		 (_fdconf_gem_edit_focus && !(shift & 7) && sc == 0x50))
	{
		co = findobj (tree, edit_obj, FMD_FORWARD);
		*onext_obj = co;
		*okr = 0;
		return 1;
	}

	/* Backtab? */
	if ( sc == 15 ||
		(_fdconf_gem_edit_focus && !(shift & 7) && sc == 0x48))
	{
		co = findobj (tree, edit_obj, FMD_BACKWARD);
		*onext_obj = co;
		*okr = 0;
		return 1;
	}

	if (kr & 0xff)	/* NICHT ALTERNATE */
	{
		*okr = kr;
		return 1; 
	}

	co = getkey (tree, edit_obj, kr);
	*onext_obj = co;
	*okr = 0;
	if (edit_obj == co)
	{
		*okr = kr;
		return 1;
	}
	
	tmp = FormButton (tree, co, 1, &dummy);
	if (tmp)
	{
		*onext_obj = edit_obj;
		*okr = 0;
	}							
	return tmp;
}

static FORMKEYFUNC _TheKey = FormKeybd;
static FORMHELPFUN _TheHelp;
static int _TheHelpDelay = 0;

void FormSetFormKeybd (FORMKEYFUNC fun)
{
	_TheKey = fun;
}

FORMKEYFUNC FormGetFormKeybd (void)
{
	return _TheKey;
}

void FormSetHelpFun (FORMHELPFUN fun, int delay)
{
	_TheHelp = fun;
	_TheHelpDelay = delay;
}


int
FormDo (OBJECT *tree, int startfld)
{
	int sf = startfld;
	
	return FormXDo (tree, &sf, NULL, NULL, NULL, NULL, NULL);
}

int
FormXDo (OBJECT *tree, int *startfld, int *pks,
	int *pkr, int *pmx, int *pmy, int *pmb)
{
	int edit_obj;
	int next_obj, which, cont, idx, mx, my, mb, ks, kr, br;
	int evtype = MU_BUTTON|MU_KEYBD;
	int time;
	static long tv = 0;
	ANIMBITBLK *A;
	int anim;
	int ax, ay, aw, ah;

	if (!tv) tv = Supexec (tosversion);
	if (pkr) *pkr = 0;

	WindUpdate (BEG_UPDATE);

	anim = findanim (tree);

	if (anim != -1) {	
		A = (ANIMBITBLK *)(tree[anim].ob_spec.userblk->ub_parm);
		evtype |= MU_TIMER;
		time = A->durations[A->current];
		objc_offset (tree, anim, &ax, &ay);
		aw = A->images[0]->bi_wb << 3;
		ah = A->images[0]->bi_hl;
	}

	next_obj = inifld (tree, *startfld);
	edit_obj = 0;
	cont = 1;
	
	while (cont)
	{
		if (next_obj && edit_obj != next_obj) {
			edit_obj = next_obj;
			next_obj = 0;
			myedit (tree, edit_obj, 0, 0, &idx, ED_INIT);
			if (_invert_edit && !_fdconf_gem_edit_focus)
				objc_change (tree, edit_obj, 0, 0, 0, 32767, 32767,
					tree[edit_obj].ob_state | SELECTED, 1);
		}

		which = evnt_multi (evtype, 256|2, 3, 0,
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							0x0L, time, 0, &mx, &my, &mb, &ks,
							&kr, &br);

		if (pks) *pks = ks;
		if (pmx) *pmx = mx;
		if (pmy) *pmy = my;
		if (pmb) *pmb = mb;

		if (which & MU_TIMER) {
			ObjcDraw (tree, 0, 10, ax, ay, aw, ah);
		 	time = A->durations[A->current];
			if (A->callback) A->callback (A);
		}
		
		if (which & MU_KEYBD)
		{
			if (pkr) *pkr = kr;

			cont = _TheKey (tree, edit_obj, next_obj, kr, ks,
				&next_obj, &kr);

			if (kr)
			{
				/* Patch fuer TOS 1.0 */

				if (tv < 0x102L)
				{
					if (tree[edit_obj].ob_flags & EDITABLE)
						if (!(tree[edit_obj].ob_spec.tedinfo->te_pvalid[idx]
							== '9') || ((kr&0xff) != '_') )
								cont = myedit (tree, edit_obj, kr, ks, &idx, ED_CHAR );
				}
				else
				{
					cont = myedit (tree, edit_obj, kr, ks, &idx, ED_CHAR);
				}
				
				if (!cont)
					next_obj = edit_obj;
			}
		}
		
		
		if (which & MU_BUTTON)
		{
			next_obj = objc_find (tree, ROOT, MAX_DEPTH, mx, my);

			if (mb == 1)
			{
				if (next_obj == -1)
				{
					next_obj = 0;
					Bconout (2, 7);
				}
				else
					cont = FormButton (tree, next_obj, br, &next_obj);
			}
			else if (mb == 2 && _TheHelp)
				_TheHelp (tree, next_obj, mx, my);
				
		}
		
		if (!cont || (next_obj && next_obj != edit_obj))
		{
			myedit (tree, edit_obj, 0, 0, &idx, ED_END);
			if (_invert_edit && !_fdconf_gem_edit_focus)
				objc_change (tree, edit_obj, 0, 0, 0, 32767, 32767,
					tree[edit_obj].ob_state & ~SELECTED, 1);
			
		}
	}

	WindUpdate (END_UPDATE);
	_TheKey = FormKeybd;
	_TheHelp = NULL;
	*startfld = edit_obj;

	if ((tree[next_obj & 0x7fff].ob_flags & EXIT) && (which & MU_KEYBD))
		evnt_timer (100, 0);

	return next_obj;
}
