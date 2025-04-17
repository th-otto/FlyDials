/*
	@(#)FlyDial/clip.c
	
	Julian F. Reschke, 27. Mai 1995
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "flydial/flydial.h"

static void
appendslash (char *fn)
{
	if (fn[strlen(fn)-1] != '\\')
		strcat (fn, "\\");
}

static void
removeslash (char *fn)
{
	int len = (int)strlen (fn);

	if (fn[len-1] == '\\')
		fn[len-1] = 0;
}

static int
dirpresent (const char *dirname)
{
	int ret;
	DTA *oldDTA = Fgetdta (), myDTA;
	
	Fsetdta (&myDTA);
	ret = Fsfirst (dirname, 0x17);
	Fsetdta (oldDTA);
	
	if (ret) return 0;
	
	return myDTA.d_attrib & FA_SUBDIR;
}


/* Boot-Device abfragen */

static long
get_boot_device (void)
{
	long bootdev = *((int *)(0x446));
	
	return 	bootdev & 0xff;
}


/*
	FindClipFile

	RÅckgabewert:	== 0: Fehler
					!= 0: Erfolg
	Extension:		Namenserweiterung der gesuchten
					Datei
	Filename:		Zeiger auf Char-Array, in das der
                    vollstÑndige Name eingetragen wird
*/

int
ClipFindFile (const char *xt, char *filename)
{
	scrp_read (filename);		/* Pfad lesen */

	if (filename[0] == '\0')
	{
		/* Clipboard nicht gesetzt */
	
		char *env_clip;
		int ret;
		int mnum;
		MFORM mform;
		
		env_clip = getenv ("CLIPBRD");

		if (env_clip)
			strcpy (filename, env_clip); 
		else
		{
			char default_dir[] = "#:\\clipbrd\\";
			
			default_dir[0] = 'A' + (int) Supexec (get_boot_device);
			strcpy (filename, default_dir);
		}


		/* Erzeuge das Directory */
		GrafGetForm (&mnum, &mform);
		GrafMouse (HOURGLASS, NULL);			

		removeslash (filename);

		if (!dirpresent (filename))
		{
			ret = Dcreate (filename);
	
			if (ret < 0) filename[0] = '\0';	
		}
		
		appendslash (filename);
		GrafMouse (mnum, &mform);

		/* Setze es */
		scrp_write (filename);

	}
	
	appendslash (filename);
		
	strcat (filename, "scrap.");
	strcat (filename, xt);
	return 1;
} 

/*
	ClearClip

	Lîscht alle SCRAP-Dateien
*/

int ClipClear (char *not)
{
	char thename[128];

	if (ClipFindFile ("*", thename))
	{
		DTA myDTA, *oldDTA = Fgetdta ();
		int fsret;
		char *where;

		Fsetdta (&myDTA);
		where = thename + strlen(thename) - sizeof ("*");		

		fsret = Fsfirst (thename, 0x17);
		while (!fsret)
		{
			char *ext = myDTA.d_fname + strlen ("scrap");
		
			strcpy (where, ext);
			
			if (!not || stricmp (ext + 1, not))
				Fdelete (thename);
			
			fsret = Fsnext();
		}

		Fsetdta (oldDTA);
		return 1;
	}
	
	return 0;
}


/* Finde heraus, was die Systemshell ist */

static int
get_shell (void)
{
	char buffer[20];
	int type;
	static int system_shell = -1;

	if (system_shell == -1 && _GemParBlk.global[0] >= 0x0399)
	{
		if (! appl_search (2, buffer, &type, &system_shell))
			system_shell = -2;
	}

	return system_shell;
}

static void
send_to_all (int *buffer)
{
	int dum, val = 0;

	buffer[1] = _GemParBlk.global[2];
	
	if (0 != appl_xgetinfo (10, &val, &dum, &dum, &dum)
		&& ((val & 0xff) >= 7))
		shel_write (7, 0, 0, (char *)buffer, NULL);
	else
	{
		int to = get_shell ();

		if (to >= 0)
			appl_write (to, 16, buffer);
	}
}

void
ClipChanged (void)
{
	int mbuf[8];
#if 0	
	char thename[128];
	
	ClipFindFile ("txt", thename);
	if (thename[0] && thename[1] == ':')
		drv = thename[0];
	else
		drv = 'A' + Dgetdrv ();

	memset (mbuf, 0, sizeof (mbuf));
#define SH_WDRAW 72
	mbuf[0] = SH_WDRAW;
	mbuf[3] = drv;

	send_to_all (mbuf);
#endif	

	memset (mbuf, 0, sizeof (mbuf));
#define SC_CHANGED 80

	mbuf[0] = SC_CHANGED;
		
	send_to_all (mbuf);
}

int
ClipWriteString (const char *str, int append)
{
	int success = 0;
	char name[128];
	long handle;
	
	ClipClear ("txt");
	ClipFindFile ("txt", name);
	
	/* nicht Shift: Datei lîschen */
	if (!append) Fdelete (name);
	
	handle = Fopen (name, 1);
	if (handle < 0) handle = Fcreate (name, 0);
	if (handle >= 0) {
		Fseek (0, (int) handle, 2);
		if (strlen (str) == Fwrite ((int) handle, strlen (str), str))
			success = 1;
		Fclose ((int) handle);
	}
	
	ClipChanged ();
	
	return success;
}
