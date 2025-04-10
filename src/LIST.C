/*
	Beispiel fÅr FormSetFormKeybd
*/


static int _listob;

static int _findselected (OBJECT *tree, int ob)
{
	int i;

	i = tree[ob].ob_head;		
	
	while (!(tree[i].STATE & SELECTED))
		i = tree[i].ob_next;

	return i;
}

static int _UpDown (OBJECT *tree, int object, int obnext, 
					int ochar, int *nxtobject, int *nxtchar)
{
	if (ochar == 0x4800)
	{
		int tmp,selob,ob;

		/* einen nach oben */
		
		ob = _findselected (tree, _listob);
		selob = ObjcGParent (tree, ob);
		selob = tree[selob].ob_head;

		if (selob != ob) /* war nicht schon am Anfang */
		{
			while (tree[selob].ob_next != ob)
				selob = tree[selob].ob_next;

			tmp = FormButton (tree, selob, 1, nxtobject);
			*nxtchar = 0;
			return tmp;
		}
	}

	if (ochar == 0x5000)
	{
		int tmp,selob;
		int ob;

		ob = _findselected (tree, _listob);
		selob = tree[ob].ob_next;

		/* schon am Ende? */

		if ((tree[selob].ob_tail != ob) && 
			(!(tree[selob].FLAGS & HIDETREE)))
		{
			tmp = FormButton (tree, selob, 1, nxtobject);
			*nxtchar = 0;
			return tmp;
		}
	}
	
	return FormKeybd (tree, object, obnext, ochar, nxtobject, nxtchar);
}

/*
	Normales DialDo mit Unterschied:
	
	in einer normalen Liste (also Parent-Object mit vertikal Åbereinander
	angeordneten Radio-Objekten kann mit den Cursor-Tasten gescrollt
	werden.
	
	Beispiel: Wildcardselection in Gemini, alle Listen in SCSI-Tool
*/


int DialDoList (DIALINFO *D, int startob, int listob)
{
	_listob = listob;
	FormSetFormKeybd (_UpDown);
	return DialDo (D, startob);
}