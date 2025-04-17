/*
	@(#)Flydial/sysfonts.c
	
	Julian F. Reschke, 16. September 1995
	
	IDs und Grîûen der SystemzeichensÑtze erfragen
*/

#include <stdio.h>

#include "flydial/flydial.h"


static int
load_font (int vdihandle, int fontid)
{
	int loaded = 0;

	if (fontid != vst_font (vdihandle, fontid) && vq_gdos ())
	{
		vst_load_fonts (vdihandle, 0);
		loaded = 1;
	}

	return loaded;
}	

#if 0
static int
height_to_pt (int vdihandle, int id, int height)
{
	int set = 99, last_set = 100;
	int current_height = 1000;
	
	vst_font (vdihandle, id);
	
	while (current_height != height)
	{
		int dummy;

/* printf ("\rFont %d current %d height %d setsize %d\n", id,
	current_height, height, last_set); */
		
		last_set = vst_point (vdihandle, set, &dummy,
								&current_height, &dummy, &dummy);
/* printf ("%d -> %d\n", set, last_set); */
		if (last_set <= 0 || last_set > set) break;

		set = last_set - 1;
	}
	
/* printf ("\rFont %d current %d height %d setsize %d\n", id,
	current_height, height, last_set); */

	return last_set;
}
#endif

void
FontAESInfo (int vdihandle, int *pfontsloaded,
	int *pnormid, int *pnormheight,
	int *piconid, int *piconheight)
{
	static int done = 0;
	static int normid, normheight;
	static int iconid, iconheight;
	int fontsloaded = 0;
	
	if (!done)
	{
		int aeshandle, cellwidth, cellheight, dummy;
		int done = 0;

		wind_update (BEG_UPDATE);
	
		aeshandle = graf_handle (&cellwidth, &cellheight, &dummy, &dummy);
	
		if (_GemParBlk.global[0] >= 0x0400 ||
			0 == appl_find ("?AGI\0\0\0\0"))
		{
			/* zunÑchst versuchen wir, die Fontids beim AES zu erfragen */
	
			done = appl_getinfo (0, &normheight, &normid, &dummy, &dummy);
			if (done)
				done = appl_getinfo (1, &iconheight, &iconid, &dummy, &dummy);
		}

		/* Wenn das Ermitteln nicht funktioniert hat...*/
		if (!done)
		{
			/* Hier fragen wir den aktuellen Font der AES-Workstation
			ab. Dies ist ein Hack, aber es funktioniert mit den
			unterschiedlichen Auto-Ordner-Tools und ist eben nur
			bis AES 3.99 nîtig. Auûerdem gehen wir davon aus,
			daû fÅr beide Textgrîûen dieselbe Schrift verwendet wird */
	
			static OBJECT dum_ob = {0, -1, -1, G_BOXCHAR, LASTOB,
				0, 0X20001100L, 0, 0, 8, 16};
			int attrib[10];
	
			objc_draw (&dum_ob, 0, 1, 0, 0, 1, 1);
			vqt_attributes (aeshandle, attrib);
			iconid = normid = attrib[0];
			normheight = attrib[7];
			iconheight = 4;
		}
	
		/* Nun haben wir fÅr beide Fonts die Id und die Pixelgrîûe
		(Parameter fÅr vst_height). Nun sorgen wir dafÅr, daû beide
		Fonts auch wirklich auf der aktuellen virtuellen Workstation
		geladen sind (wir gehen davon, daû sie generell verfÅgbar
		sind, sonst hÑtte sie uns das AES ja nicht melden dÅrfen). */
	
		fontsloaded |= load_font (vdihandle, normid);
		fontsloaded |= load_font (vdihandle, iconid);
	
		/* Systemfont in Standardgrîûe einstellen */
		
		vst_font (vdihandle, normid);
		vst_height (vdihandle, normheight, &dummy, &dummy, &dummy, &dummy);
	
		wind_update (END_UPDATE);
		
		done = 1;
	}

	/* RÅckgabewerte */
		
	if (pnormid) *pnormid = normid;
	if (pnormheight) *pnormheight = normheight;
	if (piconid) *piconid = iconid;
	if (piconheight) *piconheight = iconheight;
	if (pfontsloaded) *pfontsloaded = fontsloaded;
}
