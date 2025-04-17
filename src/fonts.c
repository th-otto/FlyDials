/*
	@(#)FlyDial/font.c
	
	Julian F. Reschke & Stefan Eissing
	13. Juli 1993
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flydial/flydial.h"

/* Defines fÅr Speedo */

#define  FH_FMVER    0      /* U   D4.0 CR LF NULL NULL  8 bytes            */
#define  FH_FNTSZ    8      /* U   Font size (bytes) 4 bytes                */
#define  FH_FBFSZ   12      /* U   Min font buffer size (bytes) 4 bytes     */
#define  FH_CBFSZ   16      /* U   Min char buffer size (bytes) 2 bytes     */
#define  FH_HEDSZ   18      /* U   Header size (bytes) 2 bytes              */
#define  FH_FNTID   20      /* U   Source Font ID  2 bytes                  */
#define  FH_SFVNR   22      /* U   Source Font Version Number  2 bytes      */
#define  FH_FNTNM   24      /* U   Source Font Name  70 bytes               */
#define  FH_MDATE   94      /* U   Manufacturing Date  10 bytes             */
#define  FH_LAYNM  104      /* U   Layout Name  70 bytes                    */
#define  FH_CPYRT  174      /* U   Copyright Notice  78 bytes               */
#define  FH_NCHRL  252      /* U   Number of Chars in Layout  2 bytes       */
#define  FH_NCHRF  254      /* U   Total Number of Chars in Font  2 bytes   */
#define  FH_FCHRF  256      /* U   Index of first char in Font  2 bytes     */
#define  FH_NKTKS  258      /* U   Number of kerning tracks in font 2 bytes */
#define  FH_NKPRS  260      /* U   Number of kerning pairs in font 2 bytes  */
#define  FH_FLAGS  262      /* U   Font flags 1 byte:                       */
                            /*       Bit 0: Extended font                   */
                            /*       Bit 1: not used                        */
                            /*       Bit 2: not used                        */
                            /*       Bit 3: not used                        */
                            /*       Bit 4: not used                        */
                            /*       Bit 5: not used                        */
                            /*       Bit 6: not used                        */
                            /*       Bit 7: not used                        */
#define  FH_CLFGS  263      /* U   Classification flags 1 byte:             */
                            /*       Bit 0: Italic                          */
                            /*       Bit 1: Monospace                       */
                            /*       Bit 2: Serif                           */
                            /*       Bit 3: Display                         */
                            /*       Bit 4: not used                        */
                            /*       Bit 5: not used                        */
                            /*       Bit 6: not used                        */
                            /*       Bit 7: not used                        */
#define  FH_FAMCL  264      /* U   Family Classification 1 byte:            */
                            /*       0:  Don't care                         */
                            /*       1:  Serif                              */
                            /*       2:  Sans serif                         */
                            /*       3:  Monospace                          */
                            /*       4:  Script or calligraphic             */
                            /*       5:  Decorative                         */
                            /*       6-255: not used                        */
#define  FH_FRMCL  265      /* U   Font form Classification 1 byte:         */
                            /*       Bits 0-3 (width type):                 */
                            /*         0-3:   not used                      */
                            /*         4:     Condensed                     */
                            /*         5:     not used                      */
                            /*         6:     Semi-condensed                */
                            /*         7:     not used                      */
                            /*         8:     Normal                        */
                            /*         9:     not used                      */
                            /*        10:     Semi-expanded                 */
                            /*        11:     not used                      */
                            /*        12:     Expanded                      */
                            /*        13-15:  not used                      */
                            /*       Bits 4-7 (Weight):                     */
                            /*         0:   not used                        */
                            /*         1:   Thin                            */
                            /*         2:   Ultralight                      */
                            /*         3:   Extralight                      */
                            /*         4:   Light                           */
                            /*         5:   Book                            */
                            /*         6:   Normal                          */
                            /*         7:   Medium                          */
                            /*         8:   Semibold                        */
                            /*         9:   Demibold                        */
                            /*         10:  Bold                            */
                            /*         11:  Extrabold                       */
                            /*         12:  Ultrabold                       */
                            /*         13:  Heavy                           */
                            /*         14:  Black                           */
                            /*         15-16: not used                      */
#define  FH_SFNTN  266      /* U   Short Font Name  32 bytes                */
#define  FH_SFACN  298      /* U   Short Face Name  16 bytes                */
#define  FH_FNTFM  314      /* U   Font form 14 bytes                       */
#define  FH_ITANG  328      /* U   Italic angle 2 bytes (1/256th deg)       */
#define  FH_ORUPM  330      /* U   Number of ORUs per em  2 bytes           */
#define  FH_WDWTH  332      /* U   Width of Wordspace  2 bytes              */
#define  FH_EMWTH  334      /* U   Width of Emspace  2 bytes                */
#define  FH_ENWTH  336      /* U   Width of Enspace  2 bytes                */
#define  FH_TNWTH  338      /* U   Width of Thinspace  2 bytes              */
#define  FH_FGWTH  340      /* U   Width of Figspace  2 bytes               */
#define  FH_FXMIN  342      /* U   Font-wide min X value  2 bytes           */
#define  FH_FYMIN  344      /* U   Font-wide min Y value  2 bytes           */
#define  FH_FXMAX  346      /* U   Font-wide max X value  2 bytes           */
#define  FH_FYMAX  348      /* U   Font-wide max Y value  2 bytes           */
#define  FH_ULPOS  350      /* U   Underline position 2 bytes               */
#define  FH_ULTHK  352      /* U   Underline thickness 2 bytes              */
#define  FH_SMCTR  354      /* U   Small caps transformation 6 bytes        */
#define  FH_DPSTR  360      /* U   Display sups transformation 6 bytes      */
#define  FH_FNSTR  366      /* U   Footnote sups transformation 6 bytes     */
#define  FH_ALSTR  372      /* U   Alpha sups transformation 6 bytes        */
#define  FH_CMITR  378      /* U   Chemical infs transformation 6 bytes     */
#define  FH_SNMTR  384      /* U   Small nums transformation 6 bytes        */
#define  FH_SDNTR  390      /* U   Small denoms transformation 6 bytes      */
#define  FH_MNMTR  396      /* U   Medium nums transformation 6 bytes       */
#define  FH_MDNTR  402      /* U   Medium denoms transformation 6 bytes     */
#define  FH_LNMTR  408      /* U   Large nums transformation 6 bytes        */
#define  FH_LDNTR  414      /* U   Large denoms transformation 6 bytes      */
                            /*     Transformation data format:              */
                            /*       Y position 2 bytes                     */
                            /*       X scale 2 bytes (1/4096ths)            */
                            /*       Y scale 2 bytes (1/4096ths)            */
#define  SIZE_FW FH_LDNTR + 6  /* size of nominal font header */
#define  EXP_FH_METRES SIZE_FW /* offset to expansion field metric resolution (optional) */

static int
_vqt_name (int handle, int num, char *name, int *scalable, int *type)
{
	int i;
	static VDIPB V = {_VDIParBlk.contrl, _VDIParBlk.intin,
		_VDIParBlk.ptsin ,_VDIParBlk.intout, _VDIParBlk.ptsout};

	_VDIParBlk.contrl[0] = 130;
	_VDIParBlk.contrl[1] = 0;
	_VDIParBlk.contrl[3] = 2;
	_VDIParBlk.contrl[5] = 1;
	_VDIParBlk.contrl[6] = handle;

	_VDIParBlk.intin[0] = num;
	
	vdi (&V);

	for (i = 0; i < 32; i++)
		name[i] = _VDIParBlk.intout[i+1];
	
	*scalable = *type = 0;
	if (_VDIParBlk.contrl[4] >= 34) *scalable = _VDIParBlk.intout[33];
	if (_VDIParBlk.contrl[4] >= 35) *type = _VDIParBlk.intout[34];

	return _VDIParBlk.intout[0];
}

void _vqt_fontheader (int handle, void *buffer, char *pathname)
{
	int i;
	static VDIPB V = {_VDIParBlk.contrl, _VDIParBlk.intin,
		_VDIParBlk.ptsin,_VDIParBlk.intout, _VDIParBlk.ptsout};

	_VDIParBlk.contrl[6] = handle;
	*((char **)(&_VDIParBlk.intin[0])) = buffer;
	_VDIParBlk.contrl[0] = 232;
	_VDIParBlk.contrl[1] = 	_VDIParBlk.contrl[2] = 0;
	_VDIParBlk.contrl[3] = 2;
	vdi (&V);

	for (i = 0; i <= _VDIParBlk.contrl[4]; i++)
	{
		pathname[i] = (char)_VDIParBlk.intout[i];
		pathname[i+1] = 0;
	}
}

/* strstr mit case independance */

static char
*_stristr (char *str, char *src)
{
	char tmp[256];
	char *c;
	
	strcpy (tmp, str);
	strlwr (tmp);
	c = strstr (tmp, src);
	
	if (!c)
		return NULL;
	else
		return str + (c - tmp);	
}


static void
remove_trailing_blanks (char *str)
{
	char *cp = str;
	
	while (*cp) cp += 1;
	cp = &cp[-1];
	while (*cp == ' ')
		*cp-- = 0;
}


/* Aus langem Fontnamen und optionaler Form-Angabe die einzelnen
   Namen ermitteln */
   
static int
font_parse (const char *fullname, const char *optform,
			char **fontname, char **facename, char **formname)
{
	char *suffixes[] = {
		" (tm)", " - swa", " -swa"
		};
	char *styles[] = {
		" italic", " monospace", " serif", " display",
		" thin", " ultralight", " extralight", " light", " book",
		" normal", " medium", " semibold", " demibold", " bold",
		" extrabold", " ultrabold", " heavy", " black",
		" oblique", " demi", " roman", " kursiv",
		};
	char tmp[256];
	char facen[256], formn[256];
	char *cp;
	int i;
		
	facen[0] = 0;
	formn[0] = 0;
	
	strcpy (tmp, fullname);
	
	/* 1. Schritt: MÅll am Ende entfernen */
	
	for (i = 0; i < sizeof (suffixes) / sizeof (suffixes[0]); i++)
	{
		cp = _stristr (tmp, suffixes[i]);
		if (cp) *cp = 0;
	}
	
	/* 2. Schritt: 6x6 bla ummappen */	

	if (!strcmp (tmp, "6x6 system font"))
		strcpy (tmp, "System Font");
		
	/* 3. Schritt: ÅberflÅssige Blanks entfernen */
	
	while (NULL != (cp = strstr (tmp, "  ")))
		strcpy (cp, cp+1);	

	remove_trailing_blanks (tmp);

	/* Form abtrennen */

	strcpy (facen, tmp);
	
	/* erstmal die optionale probieren */
	
	if (optform && NULL != (cp = strstr (facen, optform)))
	{
		strcpy (formn, cp);
		*cp = 0;		
	}
	else if (!stricmp (optform, "regular") || !stricmp (optform, "normal"))
	{
		strcpy (formn, optform);
	}
	else
	{
		long bestmatch = 257;

		for (i = 0; i < sizeof (styles) / sizeof (styles[0]); i++)
		{
			long pos;
			char *cp = _stristr (facen, styles[i]);
			
			if (cp)
			{
				/* nur gÅltig, wenn das folgende Zeichen 
				' ' oder '\0' ist! */
			
				pos = cp - facen + strlen (styles[i]);

				if (facen[pos] == '\0' || facen[pos] == ' ')
				{
					pos = cp - facen;
					if (pos < bestmatch) bestmatch = pos;
				}
			}
		}
		
		if (bestmatch != 257)
		{
			strcpy (formn, &facen[bestmatch + 1]);
			facen[bestmatch] = 0;
		}
	}
	
	/* ggfs. formn ergÑnzen */
	
	if (! formn[0] && optform) strcpy (formn, optform);
	
	remove_trailing_blanks (formn);
	remove_trailing_blanks (facen);
	
	/* Strings allozieren */
	
	*fontname = dialmalloc (2 + strlen (tmp));
	
	if (*fontname)
	{
		strcpy (*fontname, tmp);
		
		*facename = dialmalloc (2 + strlen (facen));
		
		if (*facename)
		{
			strcpy (*facename, facen);
		
			*formname = dialmalloc (2 + strlen (formn));
		
			if (*formname)
			{
				strcpy (*formname, formn);
				return 1;
		
			}
		}
	}

	return 0;
}


/* Aus formname das Weigth ermitteln */
   
static int
simulate_weight (char *formname)
{
	struct { char *name; int weight; } styles[] = {
		"thin", 1,
		"ultralight", 2,
		"extralight", 3,
		"light", 4,
		"book", 5,
		"normal", 6,
		"medium", 7, 
		"semibold", 8, 
		"demibold", 9,
		"bold", 10,
		"fed", 10,
		"extrabold", 11,
		"ultrabold", 12,
		"heavy", 13, 
		"black", 14
		};
	int i;
	
	for (i = 0; i < sizeof (styles)/sizeof (styles[0]); i++)
		if (_stristr (formname, styles[i].name))
			return styles[i].weight;

	return 6;
}




static int
isprop (int handle, int index)
{
	int attrib[10];
	int minADE, maxADE;
	int distances[5], effects[3];
	int i, prop, dummy, width;
	
	vqt_attributes (handle, attrib);
	vst_font (handle, index);
	vst_point (handle, 20, &dummy, &dummy, &dummy, &dummy);
	
	vqt_fontinfo (handle, &minADE, &maxADE, distances, &dummy, effects);
	
	prop = 0;
	width = -1;

	/*	vqt_extent geht nicht mit dem Null-Zeichen! */
	
	if (!minADE) minADE = 1;

	
	for (i = minADE; (i <= maxADE) && (!prop); i++)
	{
		int wid;

		vqt_width (handle, i, &wid, &dummy, &dummy);

		if (wid)
		{
			if (width == -1)
				width = wid;
			else	
				prop = (width != wid);
		}
	}
	
	vst_font (handle, attrib[0]);
	vst_point (handle, attrib[7], &dummy, &dummy, &dummy, &dummy);

	return prop;
}

void
FontLoad (FONTWORK *fwork)
{
	if (fwork->loaded)
		return;
	
	if ((_GemParBlk.global[0] == 0x210) || vq_gdos())
		fwork->addfonts = vst_load_fonts (fwork->handle, 0);

	fwork->loaded = 1;
	fwork->list = NULL;
}

void
FontUnLoad (FONTWORK *fwork)
{
	if (!fwork->loaded)
		return;
	
	if ((_GemParBlk.global[0] == 0x210) || vq_gdos())
		vst_unload_fonts (fwork->handle, 0);

	fwork->loaded = 0;
	if (fwork->list)
	{
		int i = 0;
		
		while (fwork->list[i].id != -1)
		{
			dialfree (fwork->list[i].fullname);
			dialfree (fwork->list[i].fontname);
			dialfree (fwork->list[i].facename);
			dialfree (fwork->list[i].formname);

			i++;
		}

		dialfree (fwork->list);
		fwork->list = NULL;
	}
}

static int
fntcmp (FONTINFO *f1, FONTINFO *f2)
{
	int r;
	r = strcmp (f1->facename, f2->facename);
	if (r)
		return r;
	else if (f1->weight != f2->weight)
		return (f1->weight - f2->weight);
	else if (f1->classification != f2->classification)
		return (f1->classification - f2->classification);
	else
		return strcmp (f1->formname, f2->formname);
}		

int
FontGetList (FONTWORK *fwork, int proptest, int fsmtest)
{
	int cnt;
	int i;
	FONTINFO *fp;
	(void) fsmtest;
	
	if (!fwork->loaded)
		return 0;
	
	if (fwork->list)
		return 1;
	
	cnt = fwork->sysfonts + fwork->addfonts;
	if (!cnt) return 0;

	fp = dialmalloc ((cnt+1) * sizeof(FONTINFO));
	if (!fp) return 0;
	memset (fp, 0, (cnt+1) * sizeof(FONTINFO));
	for (i = 0; i < cnt; i++)
	{
		char fullname[72];
		char formname[16];
		char buffer[422];
		char path[256];
		int scalable, type;

		fp[i].base_id = -1;

		_VDIParBlk.intout[33] = 0;
		fp[i].fontname = fp[i].facename = fp[i].formname = NULL;
		fp[i].id = _vqt_name (fwork->handle, i+1, fullname, &scalable, &type);

		fp[i].flags.isfsm = scalable != 0;
		fullname[32] = 0;
		formname[0] = 0;

		buffer[FH_FNTNM] = 0;
		buffer[FH_FNTFM] = 0;
		buffer[FH_CLFGS] = 0;
		buffer[FH_FRMCL] = 0;

		if (fp[i].flags.isfsm)
		{
			vst_font (fwork->handle, fp[i].id);
			_vqt_fontheader (fwork->handle, buffer, path);
		}
		
		fp[i].weight = ((unsigned char)buffer[FH_FRMCL]) >> 4;
		fp[i].classification = buffer[FH_CLFGS];

		if (strlen (&buffer[FH_FNTNM]))
		{
			buffer[FH_FNTNM + 70] = 0;
			strcpy (fullname, &buffer[FH_FNTNM]);
		}
		
		if (strlen (&buffer[FH_FNTFM]))
		{
			buffer[FH_FNTFM + 14] = 0;
			strcpy (formname, &buffer[FH_FNTFM]);
		}

		if (strlen (&buffer[FH_FNTNM]))
			if (!strlen (&buffer[FH_FNTFM]))
				printf ("\aFont %s: keine FNTFM\n", fullname);

/*		printf ("%s - %s\n", fullname, formname); */
		
		fp[i].flags.isprop = TRUE;
		if (proptest)
		{
			/* wenn kein Type geliefert wurde... */
			if (type == 0)
				fp[i].flags.isprop = isprop (fwork->handle, fp[i].id);
			else
				fp[i].flags.isprop = !(type & 0x100);
		}
		
		fp[i].fullname = dialmalloc (2 + strlen (fullname));

		if (! fp[i].fullname || 
			! font_parse (fullname, formname, &fp[i].fontname,
			&fp[i].facename, &fp[i].formname))
		{
			int j;
			
			for (j = 0; j <= i; j++)
			{
				dialfree (fp[j].fullname);
				dialfree (fp[j].fontname);
			}
			
			return 0;
		}
		
		if (! fp[i].weight)
			fp[i].weight = simulate_weight (fp[i].formname);
		
		strcpy (fp[i].fullname, fullname);
	}

	fp[cnt].id = -1;	/* Listenende !*/

	qsort (fp, cnt, sizeof (FONTINFO), fntcmp);

	/* Familien aufbauen */
	
	for (i = 0; i < cnt; i++)
	{
		int j;
		int last = i;
		
		for (j = i + 1; j < cnt; j++)
		{
			if (!strcmp (fp[i].facename, fp[j].facename))
			{
				if (fp[i].base_id == -1)
					fp[j].base_id = i;
				else
					fp[j].base_id = fp[i].base_id;
					
	
				fp[j].flags.famentry = 1;
				fp[last].nextform = j;
				last = j;
			}
		}	
	}
	

	fwork->list = fp;		/* Liste merken */
	return 1;
}

int
FontSetPoint (FONTWORK *F, int handle, int id, int point,
	int *cw, int *ch, int *lw, int *lh)
{
	int i = 0;
	FONTINFO *f = F->list;

	while (f[i].id != -1)
	{
		if (f[i].id == id)
		{
			if (f[i].flags.isfsm)
				return vst_arbpt (handle, point, cw, ch, lw, lh);
			else
				return vst_point (handle, point, cw, ch, lw, lh);
		}

		i++;
	}
	return 0;
}

FONTINFO *
FontGetEntry (FONTWORK *F, int id)
{
	int i = 0;
	FONTINFO *f = F->list;

	while (f[i].id != -1)
	{
		if (f[i].id == id)
			return &f[i];

		i++;
	}
	return NULL;
}

int
FontIsFSM (FONTWORK *F, int id)
{
	FONTINFO *f = FontGetEntry (F, id);

	if (f)
		return f->flags.isfsm;
	else
		return 0;
}
