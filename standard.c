#include <stdlib.h>
#include <compiz.h>

static int displayPrivateIndex;

typedef struct _StandardDisplay {
    int screenPrivateIndex;
} StandardDisplay;

typedef struct _StandardScreen {
    int nothing;
} StandardScreen;

#define GET_STANDARD_DISPLAY(d)					 \
    ((StandardDisplay *) (d)->privates[displayPrivateIndex].ptr)

#define STANDARD_DISPLAY(d)			   \
    StandardDisplay *sd = GET_STANDARD_DISPLAY (d)

#define GET_STANDARD_SCREEN(s, sd)				     \
    ((StandardScreen *) (s)->privates[(sd)->screenPrivateIndex].ptr)

#define STANDARD_SCREEN(s)					   \
    StandardScreen *ss =					   \
	GET_STANDARD_SCREEN (s, GET_STANDARD_DISPLAY (s->display))

static Bool
standardInitDisplay (CompPlugin  *p,
		     CompDisplay *d)
{
    StandardDisplay *sd;

    sd = malloc (sizeof (StandardDisplay));
    if (!sd)
	return FALSE;

    sd->screenPrivateIndex = allocateScreenPrivateIndex (d);
    if (sd->screenPrivateIndex < 0)
    {
	free (sd);
	return FALSE;
    }

    d->privates[displayPrivateIndex].ptr = sd;

    return TRUE;
}

static void
standardFiniDisplay (CompPlugin  *p,
		     CompDisplay *d)
{
    STANDARD_DISPLAY (d);

    freeScreenPrivateIndex (d, sd->screenPrivateIndex);

    free (sd);
}

static Bool
standardInitScreen (CompPlugin *p,
		    CompScreen *s)
{
    StandardScreen *ss;

    STANDARD_DISPLAY (s->display);

    ss = malloc (sizeof (StandardScreen));
    if (!ss)
	return FALSE;

    s->privates[sd->screenPrivateIndex].ptr = ss;

    return TRUE;
}

static void
standardFiniScreen (CompPlugin *p,
		    CompScreen *s)
{
    STANDARD_SCREEN (s);

    free (ss);
}

static Bool
standardInit (CompPlugin *p)
{
    displayPrivateIndex = allocateDisplayPrivateIndex ();
    if (displayPrivateIndex < 0)
	return FALSE;

    return TRUE;
}

static void
standardFini (CompPlugin *p)
{
    if (displayPrivateIndex >= 0)
	freeDisplayPrivateIndex (displayPrivateIndex);
}

static int
standardGetVersion (CompPlugin *plugin,
		    int	       version)
{
    return ABIVERSION;
}

static CompPluginVTable standardVTable = {
    "standard",
    N_("standard"),
    N_("standard plugin"),
    standardGetVersion,
    standardInit,
    standardFini,
    standardInitDisplay,
    standardFiniDisplay,
    standardInitScreen,
    standardFiniScreen,
    0, /* InitWindow */
    0, /* FiniWindow */
    0, /* GetDisplayOptions */
    0, /* SetDisplayOption */
    0, /* GetScreenOptions */
    0, /* SetScreenOption */
    NULL,
    0
};

CompPluginVTable *
getCompPluginInfo (void)
{
    return &standardVTable;
}
