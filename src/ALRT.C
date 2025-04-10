/*
	@(#)FlyDial/alrt.c
	
	Julian F. Reschke, 30. April 1995
	Alert-Functions
*/

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <flydial\flydial.h>


#define MAXBUT 5
#define ALBACK 0
#define ALIM   1
#define ALMOVE 2
#define ALBUT  3

#define STDWIDTH 60

#define DIFF(a,b) 	((int)(a - b))

#define MAXBUTSIZE 100
#define MAXSTRINGSIZE 700
#define MAXLINES 23

const char *continue_string = "[Weiter";
const char *ok_string = "[OK";
const char *abbruch_string = "[Abbruch";


static char *
strtok2 (char **string, char *toks)
{
	char *s1 = *string;
	char *ret;

	if (! *string) return NULL;

	while (s1 == (ret = strpbrk (s1, toks)))
		s1 += 1;
	
	/* Anfang des Tokens gefunden */
	ret = s1;
	
	/* Ende suchen */;
	s1 = strpbrk (ret, toks);

	if (!s1)	/* kein weiteres */
		*string = NULL;
	else
	{
		*s1++ = '\0';
		*string = *s1 ? s1 : NULL;
	}
	
	return ret;
}

/* static char ButString[MAXBUTSIZE]; */
/* static char TheString[MAXSTRINGSIZE]; */
static char *starts[MAXLINES+1];

#if __MSDOS__
static char AlertBuffer[MAXBUTSIZE+MAXSTRINGSIZE];
#endif

static int _Success = 1;
static ANIMBITBLK TheAnim;

static int CPFit, CPLine;
static int ImW;

extern int DialWk;

static OBJECT AlertTree[] = 
{
-1, 1, 2, 0x1800|G_BOX, NONE, OUTLINED, (long)0x21100L, 0, 0, 55, 12,
2, -1,-1, G_IMAGE, NONE, NORMAL, (long)0L, 1, 1, 0, 0,
0, -1,-1, G_IBOX|0x1100, TOUCHEXIT, CROSSED|OUTLINED, (long)0xfe1101L, 0, 0,1,1, 
0, -1,-1, G_BUTTON|0x1200, SELECTABLE|EXIT, NORMAL, (long)0L, 0, 0, 1, 1,
0, -1,-1, G_BUTTON|0x1200, SELECTABLE|EXIT, NORMAL, (long)0L, 0, 0, 0, 0,
0, -1,-1, G_BUTTON|0x1200, SELECTABLE|EXIT, NORMAL, (long)0L, 0, 0, 0, 0,
0, -1,-1, G_BUTTON|0x1200, SELECTABLE|EXIT, NORMAL, (long)0L, 0, 0, 0, 0,
0, -1,-1, G_BUTTON|0x1200, SELECTABLE|EXIT, NORMAL, (long)0L, 0, 0, 0, 0
};

static const char *
findback (const char *a)
{
	const char *s, *t;

	s = a + CPLine;
	t = strchr (a, '|');

	if (t && (DIFF(t,a) < CPLine)) return t;
	if (strlen(a) < CPLine) return NULL;

	while (s != a)
	{
		if (*s == ' ') return s;
		
		/* Sachen wie -36 nicht trennen */
		if (*s == '-' && (s == a && *(s - 1) != ' ')) return s;
		s -= 1;
	}

	return NULL;
}
 
 
/* stelle fest, ob eine Zeichenkette nur als Blanks besteht */

static int
only_blanks (char *c)
{
	while (*c)
	{
		if (*c == 0) return TRUE;
		if (*c != ' ') return FALSE;
		c += 1;
	}
	
	return TRUE;
}

int
DialAlert (BITBLK *Image, const char *String, int Default,
	const char *Buttons)
{
	char *c;
	char *d;
	DIALINFO D;
	int i, ImH;
	int MaxButLen = 0, MaxStringLen = 0, NumLines;
	int LastBut = 1;
	char *TheString, *ButString;
	char *next_tok;

	if (strlen(Buttons) > MAXBUTSIZE) return -1;
	if (strlen(String) > MAXSTRINGSIZE) return -1;

#if __MSDOS__
	TheString = AlertBuffer;
#else
	TheString = dialmalloc ((long)(MAXSTRINGSIZE+MAXBUTSIZE));
	if (!TheString) return -1;
#endif

	ButString = &TheString[MAXSTRINGSIZE];

	WindUpdate (BEG_UPDATE);
	WindUpdate (BEG_MCTRL);

	strcpy (TheString, String);
	strcpy (ButString, Buttons);

	{
		int ddummy, WX;

		HandScreenSize (&ddummy, &ddummy, &WX, &ddummy);
		CPFit = WX/HandXSize;
		CPFit -= 6;
		if (CPFit > STDWIDTH) CPFit = STDWIDTH;
	}

	AlertTree[ALIM].ob_type = G_STRING;
	AlertTree[ALIM].ob_spec.free_string = "";
	AlertTree[ALIM].ob_width = 0;
	AlertTree[ALIM].ob_height = 0;
	AlertTree[ALMOVE].ob_width = 2;
	AlertTree[ALMOVE].ob_height = 1;
	AlertTree[ALMOVE].ob_spec.index = 0xfe1101L;

	ImW = ImH = 0;

	if (Image)
	{
		if (Image == ((BITBLK *)1L))
		{
			AlertTree[ALIM].ob_type = G_ANIMIMAGE;
			AlertTree[ALIM].ob_spec.index = (long)(&TheAnim);
			Image = TheAnim.images[0];
		}
		else
		{	
			AlertTree[ALIM].ob_type = G_IMAGE;
			AlertTree[ALIM].ob_spec.bitblk = Image;	
		}
		ImW = Image->bi_wb << 3;
		ImH = Image->bi_hl;
	}
	AlertTree[ALIM].ob_x = AlertTree[ALIM].ob_y = 1;

	CPLine = CPFit - ImW/HandXSize;

	AlertTree[ALBACK].ob_width = CPFit;
	AlertTree[ALBACK].ob_height = 12;
	AlertTree[ALBACK].ob_x = 0;

	i = ALBUT;
	next_tok = ButString;
	c = strtok2 (&next_tok, "|");
	
	while (c && (i < MAXBUT+ALBUT))
	{
		AlertTree[ALBACK].ob_tail = LastBut = i;
		AlertTree[i-1].ob_next = i;
		AlertTree[i].ob_next = 0;
		AlertTree[i].ob_height = 1;
		AlertTree[i].ob_flags = SELECTABLE|EXIT|((i==(ALBUT+Default))?DEFAULT:NONE);
		
		/* Leere Buttons verstecken */
		if (only_blanks (c) && !(AlertTree[i].ob_flags & DEFAULT))
			AlertTree[i].ob_flags |= HIDETREE;
		
		AlertTree[i].ob_spec.free_string = c;
		AlertTree[i].ob_state = NORMAL;
		
		if (strlen(c) > MaxButLen) MaxButLen = (int)strlen(c);
		c = strtok2 (&next_tok, "|");
		i++;
	}

	if (MaxButLen < 5) MaxButLen = 5;
	
	AlertTree[i-1].ob_flags |= LASTOB;

	/* Text formatieren */

	starts[0] = TheString;
	c = (char *)String;
	
	i = 0;
	while (i < MAXLINES)
	{
		d = (char *)findback(c);
		
		if (!d)
		{
			if (strlen(c) < CPLine)
			{
				strcpy (starts[i], c);
				starts[i+1] = NULL;
				i = MAXLINES+1;
			}
			else
			{
				strncpy (starts[i], c, CPLine-1);
				starts[i][CPLine-1] = 0;
				i++;
				starts[i] = &(starts[i-1][CPLine]);
				c = &c[CPLine-1];
			}
		}
		else
		{
			int dif = DIFF (d, c);
				
			strncpy (starts[i], c, dif);
			if (*d == '-')
				starts[i][dif++] = '-';
			starts[i][dif] = 0;
			c = &d[1];
			i++;
			starts[i] = &(starts[i-1][2+dif]);
		}
	}

	for (i=0; i <= MAXLINES; i++)
	{
		if (!starts[i])
		{
			NumLines = i;
			break;
		}
		else
		{
			int ln;

			if ((ln = (int)strlen(starts[i])) > MaxStringLen)
				MaxStringLen = ln;
		}
	}
/*
	dprintf ("%d\n", NumLines);
*/
	AlertTree[ALBACK].ob_height = NumLines + 5;

	if (((AlertTree[ALBACK].ob_height-2)*HandYSize)<ImH)
		AlertTree[ALBACK].ob_height = 2+(ImH/HandYSize);

	AlertTree[ALBACK].ob_width = (CPFit-CPLine)+MaxStringLen+4;

	/* Breite erh”hen, falls Buttons nicht passen */
	
	{
		int wid = 6;
		int i;
		
		for (i = ALBUT; i <= LastBut; i++)
		{
			if (AlertTree[i].ob_flags & HIDETREE)
				wid += (int) strlen (AlertTree[i].ob_spec.free_string);
			else
				wid += MaxButLen + 3;
		}
		
		if (AlertTree[ALBACK].ob_width < wid)
			AlertTree[ALBACK].ob_width = wid;
	}

	if (AlertTree[ALBACK].ob_width & 1)
		AlertTree[ALBACK].ob_width++; 	/* snappen */

	{
		int lastpos = AlertTree[ALBACK].ob_width - MaxButLen - 3;
	
		for (i = LastBut; i >= ALBUT; i--)
		{
			AlertTree[i].ob_width = MaxButLen + 1;
			AlertTree[i].ob_y = AlertTree[ALBACK].ob_height - 2;

			AlertTree[i].ob_x = lastpos;
			
			if (AlertTree[i].ob_flags & HIDETREE)
				lastpos -= (int) strlen (AlertTree[i].ob_spec.free_string);
			else
				lastpos -= (MaxButLen + 3);
		}
	}
	
	AlertTree[ALMOVE].ob_x = AlertTree[ALBACK].ob_x +
		(AlertTree[ALBACK].ob_width - AlertTree[ALMOVE].ob_width);

	for (i = 0; i < (MAXBUT + 3); i++)
		rsrc_obfix (AlertTree, i);

	AlertTree[ALIM].ob_width = ImW;
	AlertTree[ALIM].ob_height = ImH;

	ObjcTreeInit (AlertTree);
	DialCenter (AlertTree);

	if (!RectOnScreen (AlertTree->ob_x, AlertTree->ob_y,
		AlertTree->ob_width, AlertTree->ob_height))
	{
#if ! __MSDOS__
		dialfree (TheString);
#endif
		WindUpdate (END_MCTRL);
		WindUpdate (END_UPDATE);
		return -1;
	}

	if (!DialStart (AlertTree, &D)) _Success = 0;

#if MSDOS
	AlertTree[ALMOVE].ob_flags |= HIDETREE;
#endif

	DialDraw (&D);

	/* Texte ausgeben */
	
	GrafMouse (M_OFF, NULL);
	
	{
		int tx,ty,ddummy;

		vst_alignment(DialWk,0,0,&ddummy,&ddummy);
	
		tx = AlertTree[ALBACK].ob_x + ImW + (HandXSize<<1);
		ty = AlertTree[ALBACK].ob_y + (HandYSize<<1);

		for (i = 0; i < NumLines; i++)
		{
			switch (starts[i][0])
			{
				case 1:		/* ALCENTER */
					v_gtext(DialWk,tx+((1+MaxStringLen-(int)strlen(starts[i]))
						*HandXSize>>1),ty+i*HandYSize,&(starts[i][1]));
					break;
					
				case 2:		/* ALRIGHT */
					v_gtext(DialWk,tx+((1+MaxStringLen-(int)strlen(starts[i]))
						*HandXSize),ty+i*HandYSize,&(starts[i][1]));
					break;
					
				default:				
					v_gtext(DialWk,tx,ty+i*HandYSize,starts[i]);
					break;		
			}
		}
		vst_alignment (DialWk, 0, 5, &ddummy, &ddummy);
	}
	
	GrafMouse (M_ON, NULL);

	{
		MFORM TempForm;
		int TempNum;
		
		GrafGetForm (&TempNum, &TempForm);
		GrafMouse (ARROW, NULL);
		i = DialDo (&D, NULL); 

	/* i = form_do (AlertTree, 0); */
		GrafMouse (TempNum, &TempForm);
	}
	
	DialEnd(&D);
	ObjcRemoveTree (AlertTree);

#if ! __MSDOS__
	dialfree (TheString);
#endif

	WindUpdate (END_MCTRL);
	WindUpdate (END_UPDATE);
	return (i-ALBUT);
} 



int
DialAnimAlert (BITBLK **Images, int *Durations, char *String,
	int Default, const char *Buttons,
	int cdecl (*callback)(struct _animbitblk *),
	long parm)
{
	TheAnim.current = 0;
	TheAnim.images = Images;
	TheAnim.durations = Durations;
	TheAnim.callback = callback;
	TheAnim.parm = parm;
	return DialAlert ( (BITBLK *)1L, String, Default, Buttons );
}

int DialSuccess (void)
{
	int merk = _Success;
	
	_Success = 1;
	return merk;
}


/* DialAlert mit formatiertem Text */

int
DialAlertf (BITBLK *Bit, const char *but, int def, const char *msg,  ...)
{
	va_list argp;
	int ret;
	char *buffer = dialmalloc (1000);
	
	if (!buffer) return -1; 
	va_start (argp, msg);
	vsprintf (buffer, msg, argp);
	ret = DialAlert (Bit, buffer, def, but);
	va_end (arpg);
	dialfree (buffer);
	return ret;
} 

void
DialSay (const char *what, ...)
{
	va_list argp;
	char *buffer = dialmalloc(1000);
	
	if (!buffer) return;
	va_start (argp, what);
	vsprintf (buffer, what, argp);
	DialAlert (ImExclamation(), buffer, 0, ok_string);
	va_end (argp);
	dialfree (buffer);
}

void
DialWarn (const char *what, ...)
{
	va_list argp;
	char *buffer = dialmalloc(1000);
	
	if (!buffer) return;
	va_start (argp, what);
	vsprintf (buffer, what, argp);
	DialAlert (ImSignStop(), buffer, 0, continue_string);
	va_end (argp);
	dialfree (buffer);
}

void
DialFatal (const char *what, ...)
{
	va_list argp;
	char *buffer = dialmalloc(1000);
	
	if (!buffer) return;
	va_start (argp, what);
	vsprintf (buffer, what, argp);
	DialAlert (ImBomb(), buffer, 0, continue_string);
	va_end (argp);
	dialfree (buffer);
}


int
DialAsk (const char *bt, int def, const char *what, ...)
{
	va_list argp;
	int ret;
	char *buffer = dialmalloc(1000);
	
	if (!buffer) return -1;
	va_start (argp, what);
	vsprintf (buffer, what, argp);
	ret = DialAlert (ImSignQuestion(), buffer, def, bt);
	va_end (argp);
	dialfree (buffer);
	return ret;
}

void
DialSetButtons (const char *cont, const char *abbruch, const char *ok)
{
	abbruch_string = abbruch;
	continue_string = cont;
	ok_string = ok;
}
