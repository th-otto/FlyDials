/*
	@(#)FlyDial/graf.c
	
	Julian F. Reschke, 5. Dezember 1995
	
	Graf-Functions
*/

#include <stddef.h>

#ifdef ATARIGEM
#include <aes.h>
#endif

#ifdef __MSDOS__
#include "aes.h"
#include "vdi.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <flydial\evntevnt.h>
#include <flydial\flydial.h>



static int actnum = ARROW;
static MFORM actbuf;	/* ist sowieso null, sacht Stief */

void GrafGetForm (int *formnum, MFORM *formbuf)
{
	*formnum = actnum;
	*formbuf = actbuf;
}

void GrafMouse (int formnum, MFORM *formbuf)
{
	static mhid = 0;
	
	switch (formnum)
	{
		case M_ON:
			if (mhid==1)
				graf_mouse (M_ON, NULL);
			if (mhid) mhid--;			
			break;

		case M_OFF:
			if (!mhid)
				graf_mouse (M_OFF, NULL);
			mhid++;
			break;

		case USER_DEF:			/* follthru */
			actbuf = *formbuf;
		default:
			actnum = formnum;
			graf_mouse (formnum, formbuf);
			break;
	}	
}

int
GrafMouseIsOver (OBJECT *tree, int obj, int btmask)
{
	MEVENT E;

	E.e_flags = MU_TIMER;
	E.e_time = 0L;
	
	if (evnt_event (&E) & MU_TIMER)
	{
		return ((obj == objc_find (tree, 0, 10, E.e_mx, E.e_my))
			&& (btmask & E.e_mb));
	}
	else
		return 0;
}

#define EXTERN extern
#define BYTE unsigned char
#include "uhr.rh"
#include "uhr.rsh"

void
GrafNextBusy (int reset)
{
	static int cnt = 0;
	int sec_cnt, min_cnt;
	int i;
	
	MFORM mf;
	static int secs[] = { SEC00, SEC05, SEC10, SEC15, SEC20,
		SEC25, SEC30, SEC35, SEC40, SEC45, SEC50, SEC55};
	static int mins[] = { MIN00, MIN05, MIN10, MIN15, MIN20,
		MIN25, MIN30, MIN35, MIN40, MIN45, MIN50, MIN55};
	
	if (reset) cnt = 0;
	
	mf.mf_xhot = mf.mf_yhot = 8;
	mf.mf_nplanes = 1; mf.mf_fg = 0; mf.mf_bg = 1;

	sec_cnt = (cnt % 60) / 5;
	min_cnt = ((cnt / 60) / 5) % 12;
	
	memcpy (mf.mf_mask, rs_object[UHRBG].ob_spec.bitblk->bi_pdata, 32);
	memcpy (mf.mf_data, rs_object[secs[sec_cnt]].ob_spec.bitblk->bi_pdata, 32);
	for (i = 0; i < 16; i++)
		mf.mf_data[i] |= rs_object[mins[min_cnt]].ob_spec.bitblk->bi_pdata[i];

	GrafMouse (USER_DEF, &mf);

	cnt += 1;
}
