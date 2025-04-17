/*
	ST-Fortunes using Jlibs
	(c) Julian F. Reschke 1989
*/


#include "flydial/flydial.h"
#include "flydial/clip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fortic.rsh"

static char *
sccsid (void)
{
	return "@(#)Fortunes 1.26  Copyright (c)  "__DATE__" "
					 "  Julian F. Reschke";
}

#define DIFF(a,b)   ((int)((long)a-(long)b))

char Cookie[1000];


static const char *
FindBack (const char *a)
{
	register const char *s,*t;
	
	s = &(a[70]);

	t = strchr(a,'|');
	if (t && (DIFF(t,a)<70) ) return t;

	if (strlen(a)<70) return NULL;
	while ( s != a )
	{
		if (*s == ' ') return s;
		s = &(s[-1]);
	}
	return NULL;
}

void 
WriteScrap (char *stext)
{
	char SPath[128];
	char *env;
	FILE *handle;
	char tcook[1000],hold;
	char *text = tcook;
	char *rep;

	strcpy(tcook,stext);	

	ClipClear (NULL);

	if (!ClipFindFile("TXT",SPath))
	{
		graf_mouse(ARROW,0L);
		DialAlert(&Clip,"Can't find TXT-file on clipboard...",0,"[Ok");
		return;
	}
			
	graf_mouse(HOURGLASS,0L);

	handle = fopen(SPath,"w+");
	if (!handle)
	{
		graf_mouse(ARROW,0L);
		DialAlert(&Clip,"Can't open TXT-file on clipboard...",0,"[Ok");
		return;
	}

	/* Write buffer */
	
	do
	{
		env = (char *)FindBack(text);
		if(env)
		{
			hold = env[1];
			if (*env == ' ')
				env[1] = 0;
			else 
				env[0] = 0;		
		}

		if(fprintf(handle,"%s\n",text)==EOF)
		{
			graf_mouse(ARROW,0L);
			DialAlert(&Clip,"Can't write to clipboard...",0,"[Ok");
			return;
		}

		if(env) env[1] = hold;
		text = &(env[1]);
	}
	while(env!=NULL);
	fclose(handle);

	strcpy(tcook,stext);	
	text = tcook;

	ClipFindFile("1WP",SPath);

	handle = fopen(SPath,"w+");
	if (!handle)
	{
		graf_mouse(ARROW,0L);
		DialAlert(&Clip,"Can't open 1WP-file on clipboard...",0,"[Ok");
		return;
	}

	fprintf(handle,"%c%c",27,128);	

	/* Write buffer in 1WP-Format*/
	
	do
	{
		env = (char *)FindBack(text);
		if(env)
		{
			hold = env[1];
			if (*env == ' ')
				env[1] = 0;
			else 
				env[0] = 0;		
		}

		while (strchr(text,' '))
		{
			rep = strchr(text,' ');
			*rep = 30;			/* weiches Leerzeichen */	
		}

		if(fprintf(handle,"%s\n",text)==EOF)
		{
			graf_mouse(ARROW,0L);
			DialAlert(&Clip,"Can't write to clipboard...",0,"[Ok");
			return;
		}

		if(env) env[1] = hold;
		text = &(env[1]);
	}
	while(env!=NULL);
	fclose(handle);

	graf_mouse(ARROW,0L);
}


int
DoCookie (void)
{
	int but,done=0;
	char *FileName;
	char TheName[128];
	char FileCnt[128];
	char LineBuf[80];
	FILE *handle;
	long position;
	register char *TheCookie;


	while(!done)
	{
		graf_mouse(HOURGLASS,0);
	
		strcpy(Cookie,"");
		TheCookie = Cookie;
	
		strcpy(TheName,"");
		FileName = getenv("COOKIEDIR");
		if (FileName != NULL) strcpy(TheName,FileName);	
		if (TheName[strlen(TheName)-1] != '\\')
			strcat (TheName, "\\");
			
		strcat(TheName,"COOKIES");
		strcpy(FileCnt,TheName);
		strcat(FileCnt,".CNT");
	
		handle = fopen(FileCnt,"r");
		if (handle == NULL)
			position = 0;
		else
		{
			char string[80];
			
			if(fgets(string,80,handle)==NULL)
				position = 0;
			else
				position = -1+atol(string);
			fclose(handle);
		}
	
		handle = fopen(TheName,"r");
		if(handle == NULL)
		{
			graf_mouse(ARROW,NULL);
			DialAlert(NULL,"Can't find my COOKIES...",0,"[Ok");
			return 0;
		}

		setbuf(handle,NULL);
		fseek(handle,position,SEEK_SET);
		
		do
		{
			int i=0;
	
			fgets(LineBuf,80,handle);
			if(!strcmp(LineBuf,"quit\n"))
			{
				fseek(handle,0L,SEEK_SET);
				fgets(LineBuf,80,handle);
			}
	
			while(LineBuf[i]>=' ')
				*TheCookie++ = LineBuf[i++];	
		}
		while(strcmp(LineBuf,"-\n"));
		TheCookie = &(TheCookie[-1]);
		
		fflush(handle);
		ltoa(ftell(handle),LineBuf,10);
		fclose(handle);
	
		strcat(LineBuf,"\n");
		handle = fopen(FileCnt,"w+");	
		if(handle==NULL) return 1;
		fputs(LineBuf,handle);
		fclose(handle);
	
		*TheCookie = 0;
	
		graf_mouse(ARROW,NULL);
	
Same:
		but = DialAlert(NULL,Cookie,3,"[Again|[Copy|[Info...|[Quit");
		done = (but == 3);


		if(but==2)
		{
			but=DialAlert(&Stop,"BananaSoft-Fortunes 1.22|"
					       "Copyright \275|Julian F. Reschke 7/1989",
						   0,"[Ok");
			goto Same;
		}
	
		if(but==1)
		{
			WriteScrap(Cookie);
			goto Same;
		}
	}

	return 1;
}


int main ( void )
{
	appl_init();
	DialInit(Malloc,Mfree);
	DoCookie();
	DialExit();
	appl_exit();
	return 0;
}
