/*
	@(#)FlyDial/textprn.c
	
	Julian F. Reschke, 30. April 1996
	Textausgabe in Dialogen
*/

#include <string.h>

#include <aes.h>
#include <vdi.h>

#include <flydial/flydial.h>
#include <flydial/textprn.h>
#include <flydial/wk.h>

void
TextPrint (OBJECT *tree, int ob, const char *string)
{
	char outline[256], *cp, c;
	int dummy, cw, ch, sw, sh;
	int xpos, ypos;
	int outy;
	
	vst_font (DialWk, HandSIId);
	vst_height (DialWk, HandSIHeight, &dummy, &dummy, &cw, &ch);

	objc_offset (tree, ob, &xpos, &ypos);
	sw = tree[ob].ob_width;
	sh = tree[ob].ob_height;
	
	outy = ypos + sh - ch;
	
	HandClip (xpos, ypos, sw, sh, 1);
	WkWritingMode (MD_REPLACE);

	outline[0] = '\0';
	
	cp = outline;
	
	while (0 != (c = *string++))
	{
		if (c == 10) /* linefeed */
		{
			int xy[4];

			RastBufCopy (xpos, ypos + ch, sw, sh - ch, xpos, ypos,
				&RastScreenMFDB);

			RectAES2VDI (xpos, outy, sw, ch, xy);
			
			WkFillMode (WHITE, FIS_SOLID, 0, 8);
			
			vr_recfl (DialWk, xy);
		}
		else if (c == 13) /* carriage return */
		{
			v_gtext (DialWk, xpos, outy, (char *) outline);
			cp = outline;
			*cp = '\0';
		}
		else
		{
			*cp++ = c;
			*cp = '\0';			
		}		
	}

	if (*outline)
		v_gtext (DialWk, xpos, outy, (char *) outline);
	
	HandClip (xpos, ypos, sw, sh, 0);
	
	vst_font (DialWk, HandSFId);
	vst_height (DialWk, HandSFHeight, &dummy, &dummy, &dummy, &dummy);
}
