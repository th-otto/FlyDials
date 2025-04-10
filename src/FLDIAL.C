/*
	@(#)FlyDial/fldial.c
	@(#)Julian F. Reschke, 31. M„rz 1991
*/

#include <aes.h>
#include <stdio.h>
#include <flydial\flydial.h>

#include "flytest.h"

int main (void)
{
	DIALINFO DD;
	OBJECT *T, *T1;
	int but;

	appl_init();
	DialInit (Malloc, Mfree);
	rsrc_load("flytest.rsc");

	GrafMouse (ARROW, NULL);

	rsrc_gaddr(0,0,&T);
	rsrc_gaddr(0,SECSIZES,&T1);

	DialAlert (ImInfo(), "Dies ist eine kleine Alert-Box mit drei Buttons!"
	, 0, "[OK|[Cancel|[Weiter112");

	DialAlert (ImInfo(), "Hier ist ein|Zeilenumbruch erzwungen.", 0, "[Ok");

	DialAlert (ImInfo(), ALCENTER"Zentrierer Titel|Und weiterer Text", 0,
		"[Ok");

	DialAlert (ImInfo(), ALRIGHT "Rechtsjustiert|geht es auch.", 0, "[Ok");


	DialCenter(T);
	ObjcTreeInit (T);
	DialStart(T,&DD);
	
	T[FCSECSIZ].ob_spec = T1[1].ob_spec;

	DialDraw(&DD);
	do
	{
		long newspec;
	
		newspec = T[FCSECSIZ].ob_spec.index;

		but = DialDo (&DD, 0);
	

		switch (but)
		{
			case HUGO:
			case FCSECSIZ:
				JazzSelect (T, FCSECSIZ, T1, 1, 0, &newspec);
				break;

			case FCCYCLE:
				JazzSelect (T, FCSECSIZ, T1, 1, -2, &newspec);
				break;
		}
		
		if ((newspec != T[FCSECSIZ].ob_spec.index) && (newspec))
		{
			int x, y;

			ObjcOffset (T, FCSECSIZ, &x, &y);
			T[FCSECSIZ].ob_spec.index = newspec;
			objc_draw (T, FCSECSIZ, 1, x + 1, y + 1,
				T[FCSECSIZ].ob_width - 2, T[FCSECSIZ].ob_height - 2);
		}
		
	} while (!(T[but & 0x7fff].ob_flags & EXIT));
	DialEnd(&DD);

	DialExit();
	appl_exit();
	return 0;
}