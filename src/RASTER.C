/*
	@(#)FlyDial/raster.c
	
	Julian F. Reschke, 20. April 1996
	Raster-Functions

	AB:
	- RastInit, RastTerm, RastDiskSave, RastDiskRestore added
*/

#include <flydial\flydial.h>

extern int DialWk;
MFDB RastScreenMFDB;

#ifdef __MSDOS__
#	define	RAST_BUF_SIZE	4000
#	include <io.h>
#	include <dos.h>
#	include <string.h>
#	include <stdio.h>
#	include <sys\stat.h>
#endif


/*
	Ben”tigten Speicherplatz fr Bildschirmausschnitt
	bestimmen
*/

unsigned long RastSize (int w, int h, MFDB *p)
{
	int work_out[57];
	int WdWord;

	vq_extnd (DialWk, 1, work_out);
	WdWord = (w+15)>>4;
	p->fd_w = w;
	p->fd_h = h;
	p->fd_wdwidth = WdWord;
	p->fd_nplanes = work_out[4];
	p->fd_stand = 0;
		
	return (	((long)h) *			/* Bildschirmzeilen */
				((long)WdWord<<1) *	/* Bytes pro Zeile */
				((long)work_out[4]) ); 	/* Planes */
}


/* Bildschirmausschnitt in Buffer speichern */

void
RastSave (int x, int y, int w, int h, int dx, int dy, MFDB *src)
{
	int xy[8];

	RectAES2VDI (x, y, w, h, xy);
	RectAES2VDI (dx, dy, w, h, xy + 4);	
	vro_cpyfm (DialWk, 3, xy, &RastScreenMFDB, src);
}


/* Bildschirmausschnitt aus Buffer zurckholen */

void
RastRestore (int x, int y, int w, int h, int sx, int sy, MFDB *src)
{
	int xy[8];

	RectAES2VDI (sx, sy, w, h, xy);
	RectAES2VDI (x, y, w, h, xy + 4);
	vro_cpyfm (DialWk, 3, xy, src, &RastScreenMFDB);
}

#ifdef __MSDOS__

static int 	_rastLevel = 0;
static char	_rastPath[129];
static char	_rastDefFilename[14];
static int	_rastDefHandle;
static void	*_rastBufAddr;
static int	_rastBufSize;	

void RastGetQSB(void **qsbAddr, int *qsbSize)
	{
	int w1,w2,w3,w4;

	wind_get(0,17,&w1,&w2,&w3,&w4);
	*qsbSize = (((long)(unsigned int)w4) << 16) + (long)(unsigned int)w3;
	if (*qsbSize >= 32750)
		*qsbSize = 32750;

	*qsbAddr = MKFP(w2,w1);
	}


int RastInit(char *tempdir)
	{
	int 	len;
	char	fullname[129];
	char	name[20];
	int		handle;

	RastGetQSB(&_rastBufAddr,&_rastBufSize);

	strcpy(_rastPath,tempdir);
	len = (int)strlen(_rastPath);
	if (len>0)
		{
		if (_rastPath[len-1] != '\\')
			strcat(_rastPath,"\\");
		}
	
	strcpy(fullname,_rastPath);
	strcat(fullname,tmpnam(name));

	handle = dos_creat(fullname,0);
	strcpy(_rastDefFilename,name);

	_rastDefHandle = handle;
	_rastLevel = 0;
		
	return (handle != -1);
	}


void RastTerm(void)
 	{
	char	fullname[129];

	if (_rastDefHandle != -1)
		{
		close(_rastDefHandle);
		strcpy(fullname,_rastPath);
		strcat(fullname,_rastDefFilename);
		unlink(fullname);
		_rastLevel = 0;
		}
	}


int RastDiskSave(RDB *rdb, int x, int y, int w, int h)
	{
	char	stackBuffer[RAST_BUF_SIZE];
	char	fullname[129];
	char	name[14];
	void	*sliceBuffer;
	int		bufSize, bufHeight;
	int 	xy[8];
	int		i;
	int		yk;
	int		wordWidth;
	MFDB	dest;
	int		handle;
	int		sliceBytes;
	int		bytes;

	RastGetQSB(&_rastBufAddr,&_rastBufSize);
	if (_rastBufSize > 0)
		{
		sliceBuffer = _rastBufAddr;
		bufSize = _rastBufSize;
		}
	else
		{
		sliceBuffer = stackBuffer;
		bufSize = RAST_BUF_SIZE;
		}

	wordWidth = (w + 15) >> 4;
	rdb->scanlineSize = 2 * wordWidth * rdb->planes;
	if (rdb->scanlineSize > bufSize) return FALSE;

	/* if this is a nested call, open a new tempfile */
	if (_rastLevel > 0)
		{
		strcpy(fullname,_rastPath);
		strcat(fullname,tmpnam(name));

		handle = dos_creat(fullname,0);
		if (handle == -1) return FALSE;
		
		strcpy(rdb->filename,name);
		rdb->handle = handle;
		}
	else
		{
		strcpy(rdb->filename,_rastDefFilename);
		rdb->handle = _rastDefHandle;
		if (rdb->handle == -1) return FALSE;
		}

	rdb->saved = FALSE;

	rdb->sliceHeight = bufSize / rdb->scanlineSize;
	rdb->fullSlices = h / rdb->sliceHeight;
	rdb->lastSliceHeight = h % rdb->sliceHeight;
	if (rdb->lastSliceHeight > 0)
		rdb->fullSlices++;

	if (rdb->sliceHeight > rdb->lastSliceHeight)
		bufHeight = rdb->sliceHeight;
	else
		bufHeight = rdb->lastSliceHeight;

	dest.fd_addr = sliceBuffer;
	dest.fd_w = w;
	dest.fd_h = bufHeight;
	dest.fd_wdwidth = wordWidth;
	dest.fd_nplanes = rdb->planes;
	dest.fd_stand = 0;

	yk = y;
	for (i=0; i<rdb->fullSlices; i++)
		{
		int height;

		if ((i==rdb->fullSlices-1) && (rdb->lastSliceHeight > 0)) 
			height = rdb->lastSliceHeight;
		else
			height = rdb->sliceHeight;

		RectAES2VDI(x,yk,w,height,xy);
		RectAES2VDI(0,0,w,height,&(xy[4]));

		dest.fd_h = height;
		vro_cpyfm (DialWk,3,xy,&Screen,&dest);

		sliceBytes = height*rdb->scanlineSize;
		bytes = write(rdb->handle,sliceBuffer,sliceBytes);
		if (bytes == -1) goto error;

		yk += height;
		}

	rdb->saved = TRUE;
	_rastLevel++;
	return TRUE;

error:
	if (_rastLevel > 1)
		{
		close(handle);
		unlink(fullname);
		}
	return FALSE;
	}


int RastDiskRestore(RDB *rdb, int x, int y, int w, int h)
	{
	char	stackBuffer[RAST_BUF_SIZE];
	void	*sliceBuffer;
	int 	xy[8];
	int		i;
	int		yk;
	int		wordWidth;
	int		sliceBytes;
	int		bufHeight;
	MFDB	src;
	char	fullname[129];

	_rastLevel--;
	if (rdb->handle == -1) return FALSE;

	strcpy(fullname,_rastPath);
	strcat(fullname,rdb->filename);

	RastGetQSB(&_rastBufAddr,&_rastBufSize);
	if (_rastBufSize > 0)
		sliceBuffer = _rastBufAddr;
	else
		sliceBuffer = stackBuffer;

	wordWidth = (w + 15) >> 4;
	if (rdb->sliceHeight > rdb->lastSliceHeight)
		bufHeight = rdb->sliceHeight;
	else
		bufHeight = rdb->lastSliceHeight;

	src.fd_addr = sliceBuffer;
	src.fd_w = w;
	src.fd_h = bufHeight;
	src.fd_wdwidth = wordWidth;
	src.fd_nplanes = rdb->planes;
	src.fd_stand = 0;

	/* seek to the start of the file */
	lseek(rdb->handle,0L,SEEK_SET);

	yk = y;
	for (i=0; i<rdb->fullSlices; i++)
		{
		int height;

		if ((i == rdb->fullSlices-1) && (rdb->lastSliceHeight > 0))
			height = rdb->lastSliceHeight;
		else
			height = rdb->sliceHeight;

		sliceBytes = height * rdb->scanlineSize;
		if (read(rdb->handle,sliceBuffer,sliceBytes) == -1) goto error;

		src.fd_h = height;
		RectAES2VDI(0,0,w,height,xy);
		RectAES2VDI(x,yk,w,height,&(xy[4]));
		vro_cpyfm (DialWk,3,xy,&src,&Screen);

		yk += height;
		}

	if (_rastLevel >= 1)
		{
		close(rdb->handle);
		unlink(fullname);
		}
	else
		{
		/* seek to the start of the file */
		lseek(rdb->handle,0L,SEEK_SET);
		}

	return TRUE;

error:
	if (_rastLevel >= 1)
		{
		close(rdb->handle);
		unlink(fullname);
		}

	return FALSE;
	}


#endif

#if !__MSDOS__	
void RastBufCopy (int sx, int sy, int w, int h, int dx, int dy, MFDB *TheBuf )
{
	int xy[8];

	RectAES2VDI(sx,sy,w,h,xy);
	RectAES2VDI(dx,dy,w,h,&(xy[4]));
	vro_cpyfm (DialWk,3,xy,TheBuf,TheBuf);
}
#endif

void RastDrawRect (int ha, int x, int y, int w, int h)
{
	int xy[10];
	int *c;
	int x2,y2;
	
	c = xy;
	x2 = x+w-1; y2 = y+h-1;
	
	*c++ = x; *c++ = y;
	*c++ = x; *c++ = y2;
	*c++ = x2; *c++ = y2;
	*c++ = x2; *c++ = y;
	*c++ = x+1; *c = y;
	v_pline(ha,5,xy);
}

#if !__MSDOS__
void RastSetDotStyle (int ha, int *xy)
{
	unsigned int selection = 0xaaaa;
	
	if ((xy[0] + xy[1]) & 1) selection = 0x5555;
	
	if (xy[0] == xy[2])
		selection ^= 0xffff;
	else
		if (!(xy[0] & 1)) selection ^= 0xffff;

	vsl_udsty (ha, selection);
}

void RastDotRect (int ha, int x, int y, int w, int h)
{
	int xy[4];
	
	vsl_type (ha, 7);
	
	xy[0] = x;
	xy[1] = y;
	xy[2] = x+w-1;
	xy[3] = y;
	
	RastSetDotStyle (ha, xy);
	v_pline (ha, 2, xy);
	
	xy[1] += (h-1);
	xy[3] = xy[1];

	RastSetDotStyle (ha, xy);
	v_pline (ha, 2, xy);

	xy[0] = xy[2];
	xy[1] = y+1;
	xy[3] = y+h-2;

	RastSetDotStyle (ha, xy);
	v_pline (ha, 2, xy);

	xy[0] = xy[2] = x;

	RastSetDotStyle (ha, xy);
	v_pline (ha, 2, xy);
	
	vsl_type (ha, 1);
}
#endif

static void vdifix (MFDB *pfd, void *theaddr, int wb, int h)
{
	pfd->fd_wdwidth = wb >> 1;
	pfd->fd_w = wb << 3;
	pfd->fd_h = h;
	pfd->fd_nplanes = 1; /* monochrom */
	pfd->fd_addr = theaddr;
}


void RastTrans (void *saddr, int swb, int h, int handle)
{
	MFDB src,dst;

	vdifix (&src,saddr,swb,h);
	src.fd_stand = 1;	/* Standardformat */
	vdifix (&dst,saddr,swb,h);
	dst.fd_stand = 0;
	vr_trnfm (handle,&src,&dst);
}
