#include <stdlib.h>
#include <compiz.h>

#define SCAN_DOWN  0
#define SCAN_LEFT  1
#define SCAN_RIGHT 2
#define SCAN_UP    4
#define KEYNAV_DISPLAY_OPTION_COUNT 4

static int keynavDisplayPrivateIndex;
static CompMetadata keynavMetadata;

#define KEYNAV_DISPLAY(display) \
    (KeynavDisplay) display->privates[keynavDisplayPrivateIndex].ptr

typedef enum {
    DOWN_KEY,
    LEFT_KEY,
    RIGHT_KEY,
    UP_KEY
} FocusDirectionKeys;

static const CompMetadataOptionInfo keynavOptionInfo[] = {
    { "keynav_focus_down",  "key", 0, keynavFocusDown,  0 },
    { "keynav_focus_left",  "key", 0, keynavFocusLeft,  0 },
    { "keynav_focus_right", "key", 0, keynavFocusRight, 0 },
    { "keynav_focus_up",    "key", 0, keynavFocusUp,    0 },
};

typedef struct _KeynavDisplay {
    int privateIndex;
    CompOption      options[KEYNAV_DISPLAY_OPTION_COUNT];

    KeyCode         downKey,
                    leftKey,
                    rightKey,
                    upKey;
} KeynavDisplay;

int getDistance (int direction, int start, CompWindow window) {
    switch (direction) {
        case SCAN_RIGHT:
            return window->serverX - start;
        case SCAN_LEFT:
            return (window->serverX + window->serverWidth) - start;
        case SCAN_UP:
            return (window->serverY + window->serverHeight) - start;
        case SCAN_DOWN:
            return window->serverY - start;
    }
}

int getStartPoint (int direction, CompWindow window) {
    switch (direction) {
        case SCAN_RIGHT:
            return window->serverX + window->serverWidth;
        case SCAN_LEFT:
            return window->serverX;
        case SCAN_UP:
            return window->serverY;
        case SCAN_DOWN:
            return window->serverY + window->serverHeight;
    }
}

Bool isFocusableWindow (CompWindow window) {
    if (window->attrib.override_redirect) {
        return FALSE;
    }
    if (window->wmType & (CompWindowTypeDockMask | CompWindowTypeDesktopMask)) {
        return FALSE;
    }
    if (window->state & CompWindowStateSkipPagerMask) {
        return FALSE;
    }
    if (window->state & CompWindowStateShadedMask) {
        return FALSE;
    }
    if (window->mapNum || window->attrib.map_state != IsViewable) {
        return FALSE;
    }

    return TRUE;
}

static Bool sendFocus (CompDisplay *display, int direction) {
    int i = 0, selectedDistance = 0, start = 0;
    CompScreen *screen;
    CompWindow *activeWindow;
    CompWindow *selectedWindow = NULL;
    CompWindow *window;

    screen = findScreenAtDisplay(display, display->activeWindow);
    activeWindow = findWindowAtDisplay(display, display->activeWindow);

    if (activeWindow) {
        start = getStartPoint(direction, activeWindow);

        for (window = screen->windows; window != NULL; window = window->next) {
            if (window != activeWindow && isFocusableWindow(window)) {
                distance = getDistance(direction, start, window);

                if (distance > 0 && distance < selectedDistance) {
                    selectedDistance = distance;
                    selectedWindow   = window;
                }
            }
        }

        if (selectedWindow != NULL) {
            raiseWindow(selectedWindow);
            focusWindow(selectedWindow);

            return TRUE;
        }
    }

    return FALSE;
}

static Bool
keynavFocusDown (CompDisplay *display, CompAction *action, CompActionState state,
        CompOption *option, int nOption) {
    return sendFocus(display, SCAN_DOWN);
}
static Bool
keynavFocusLeft (CompDisplay *display, CompAction *action, CompActionState state,
        CompOption *option, int nOption) {
    return sendFocus(display, SCAN_LEFT);
}
static Bool
keynavFocusRight (CompDisplay *display, CompAction *action, CompActionState state,
        CompOption *option, int nOption) {
    return sendFocus(display, SCAN_RIGHT);
}

static Bool
keynavFocusUp (CompDisplay *display, CompAction *action, CompActionState state,
        CompOption *option, int nOption) {
    return sendFocus(display, SCAN_UP);
}

static Bool
keynavInitDisplay (CompPlugin *plugin, CompDisplay *display) {
    KeynavDisplay *keynavDisplay;

    if (!checkPluginABI ("core", CORE_ABIVERSION)) {
        return FALSE;
    }

    keynavDisplay = malloc(sizeof *keynavDisplay);

    if (!keynavDisplay) {
        return FALSE;
    }

    keynavDisplayPrivateIndex = allocateDisplayPrivateIndex();
    
    if (scaleDisplayPrivateIndex < 0) {
	compFiniMetadata (&scaleMetadata);
	return FALSE;
    }


    if (!compInitDisplayOptionsFromMetadata(display,
                &keynavMetadata, keynavOptionInfo, keynavDisplay->options,
                KEYNAV_DISPLAY_OPTION_COUNT)) {
        free(keynavDisplay);

        return FALSE;
    }

    keynavDisplay->downKey  = XKeysymToKeycode(display->display,
            XStringToKeysym("Down"));
    keynavDisplay->leftKey  = XKeysymToKeycode(display->display,
            XStringToKeysym("Left"));
    keynavDisplay->rightKey = XKeysymToKeycode(display->display,
            XStringToKeysym("Right"));
    keynavDisplay->upKey    = XKeysymToKeycode(display->display,
            XStringToKeysym("Up"));

    return TRUE;
}

static Bool
keynavInit (CompPlugin *p, CompObject *o)
{
    static InitPluginObjectProc dispTab[] = {
	(InitPluginObjectProc) 0, /* InitCore */
	(InitPluginObjectProc) keynavInitDisplay,
	0, 
	0 
    };

    RETURN_DISPATCH (o, dispTab, ARRAY_SIZE (dispTab), TRUE, (p, o));
}

static void keynavFiniDisplay (CompPlugin *p, CompDisplay *d) {
    if (keynavDisplayPrivateIndex >= 0)
        freeDisplayPrivateIndex(keynavDisplayPrivateIndex);

    compFiniMetadata(&keynavMetadata);
}

static void
keynavFini (CompPlugin *p,
            CompObject *o)
{
    static FiniPluginObjectProc dispTab[] = {
	(FiniPluginObjectProc) 0, /* FiniCore */
	(FiniPluginObjectProc) keynavFiniDisplay, 
	0, 
	0 
    };

    DISPATCH (o, dispTab, ARRAY_SIZE (dispTab), (p, o));
}

static int
keynavGetVersion (CompPlugin *plugin,
		    int	       version)
{
    return ABIVERSION;
}

static CompMetadata * keynavGetMetadata (CompPlugin *plugin) {
    return &keynavMetadata;
}

static Bool
keynavSetDisplayOption (CompPlugin      *plugin,
                        CompDisplay     *display,
                        const char      *name,
                        CompOptionValue *value)
{
    int index;
    CompOption *option;
    KeynavDisplay *keynavDisplay = KEYNAV_DISPLAY(display);

    option = compFindOption(keynavDisplay->options, KEYNAV_DISPLAY_OPTION_COUNT,
            name, &index);

    if (option) {
        return compSetDisplayOption(display, option, value)
    }

    return FALSE;
}

static Bool
keynavSetObjectOption (CompPlugin      *plugin,
		      CompObject      *object,
		      const char      *name,
		      CompOptionValue *value)
{
    static SetPluginObjectOptionProc dispTab[] = {
	(SetPluginObjectOptionProc) 0, /* SetCoreOption */
	(SetPluginObjectOptionProc) keynavSetDisplayOption,
	(SetPluginObjectOptionProc) 0
    };

    RETURN_DISPATCH (object, dispTab, ARRAY_SIZE (dispTab), FALSE,
		     (plugin, object, name, value));
}

static CompOption *
keynavGetDisplayOptions (CompPlugin *plugin, CompDisplay *display, int *count) 
{
    KeynavDisplay *keynavDisplay = KEYNAV_DISPLAY(display);

    *count = KEYNAV_DISPLAY_OPTION_COUNT

    return keynavDisplay->options;
}

static CompOption *
keynavGetObjectOptions (CompPlugin *plugin, CompObject *object, int *count)
{
    static GetPluginObjectOptionsProc dispTab[] = {
        (GetPluginObjectOptionsProc) 0, /* GetCoreOptions */
        (GetPluginObjectOptionsProc) keynavGetDisplayOptions,
        (GetPluginObjectOptionsProc) 0
    };

    RETURN_DISPATCH (object, dispTab, ARRAY_SIZE (dispTab),
                     (void *) (*count = 0), (plugin, object, count));
}

static CompPluginVTable keynavVTable = {
    "keynav",
    keynavGetMetadata,
    keynavInit,
    keynavFini,
    keynavInitDisplay,
    keynavFiniDisplay,
    keynavGetObjectOptions,
    keynavSetObjectOption
};

CompPluginVTable *
getCompPluginInfo (void)
{
    return &keynavVTable;
}
