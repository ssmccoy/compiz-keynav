#include <stdlib.h>
#include <compiz-core.h>

#define DEBUG(message) fprintf(stderr, "%s\n", message)

#define SCAN_DOWN  0
#define SCAN_LEFT  1
#define SCAN_RIGHT 2
#define SCAN_UP    4
#define KEYNAV_DISPLAY_OPTION_COUNT 4

static int keynavDisplayPrivateIndex;
static CompMetadata keynavMetadata;

#define KEYNAV_DISPLAY(display) \
    (KeynavDisplay *) (display)->base.privates[keynavDisplayPrivateIndex].ptr

typedef enum {
    DOWN_KEY,
    LEFT_KEY,
    RIGHT_KEY,
    UP_KEY
} FocusDirectionKeys;

typedef struct _KeynavDisplay {
    int privateIndex;
    CompOption      options[KEYNAV_DISPLAY_OPTION_COUNT];

    KeyCode         downKey,
                    leftKey,
                    rightKey,
                    upKey;
} KeynavDisplay;

int getDistance (int direction, int start, CompWindow *window) {
    switch (direction) {
        case SCAN_LEFT:
            return start - window->serverX;
        case SCAN_RIGHT:
            return window->serverX - start;
        case SCAN_UP:
            return start - window->serverY;
        case SCAN_DOWN:
            return window->serverY - start;
    }
}

int getStartPoint (int direction, CompWindow *window) {
    switch (direction) {
        case SCAN_RIGHT:
        case SCAN_LEFT:
            return window->serverX;
        case SCAN_UP:
        case SCAN_DOWN:
            return window->serverY;
    }
}

Bool isFocusableWindow (CompWindow *window) {
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
    if (window->attrib.map_state != IsViewable) {
        return FALSE;
    }

    return TRUE;
}

Bool withinBound (int direction, CompWindow *active, CompWindow *window) {
    int activeRangeStart = 0,
        activeRangeEnd   = 0,
        windowRangeStart = 0,
        windowRangeEnd   = 0;

    switch (direction) {
        case SCAN_LEFT:
        case SCAN_RIGHT:
            activeRangeStart = active->serverY;
            activeRangeEnd   = active->serverY + active->serverHeight;

            windowRangeStart = window->serverY;
            windowRangeEnd   = window->serverY + window->serverHeight;
            break;

        case SCAN_UP:
        case SCAN_DOWN:
            activeRangeStart = active->serverX;
            activeRangeEnd   = active->serverX + active->serverWidth;

            windowRangeStart = window->serverX;
            windowRangeEnd   = window->serverX + window->serverWidth;
    }

    return ((windowRangeStart >= activeRangeStart &&
             windowRangeStart <= activeRangeEnd) ||
            (windowRangeEnd >= activeRangeStart &&
             windowRangeEnd <= activeRangeEnd));
}

static Bool sendFocus (CompDisplay *display, CompOption *option, int nOption,
        int direction) {
    int i = 0, distance, selectedDistance, start = 0;
    CompScreen *screen;
    CompWindow *activeWindow;
    CompWindow *selectedWindow = NULL;
    CompWindow *window;

    Window xid = getIntOptionNamed(option, nOption, "root", 0);

    screen = findScreenAtDisplay(display, xid);

    if (!screen) {
        return FALSE;
    }

    selectedDistance = screen->width;

    activeWindow = findWindowAtScreen(screen, display->activeWindow);

    if (activeWindow) {
        start = getStartPoint(direction, activeWindow);

        for (window = screen->windows; window != NULL; window = window->next) {

            if (window != activeWindow && 
                isFocusableWindow(window) &&
                withinBound(direction, activeWindow, window)) 
            {
                distance = getDistance(direction, start, window);

                if (distance > 0 && distance < selectedDistance) {
                    selectedDistance = distance;
                    selectedWindow   = window;
                }
            }
        }

        if (selectedWindow != NULL) {
            addWindowDamage(selectedWindow);
            raiseWindow(selectedWindow);
            moveInputFocusToWindow(selectedWindow);
        }
    }

    return FALSE;
}

static Bool keynavFocusDown (CompDisplay *display, CompAction *action,
        CompActionState state, CompOption *option, int nOption) {
    return sendFocus(display, option, nOption, SCAN_DOWN);
}

static Bool keynavFocusLeft (CompDisplay *display, CompAction *action,
        CompActionState state, CompOption *option, int nOption) {
    return sendFocus(display, option, nOption, SCAN_LEFT);
}

static Bool keynavFocusRight (CompDisplay *display, CompAction *action,
        CompActionState state, CompOption *option, int nOption) {
    return sendFocus(display, option, nOption, SCAN_RIGHT);
}

static Bool keynavFocusUp (CompDisplay *display, CompAction *action,
        CompActionState state, CompOption *option, int nOption) {
    return sendFocus(display, option, nOption, SCAN_UP);
}

static const CompMetadataOptionInfo keynavOptionInfo[] = {
    { "keynav_focus_down",  "key", 0, keynavFocusDown,  0 },
    { "keynav_focus_left",  "key", 0, keynavFocusLeft,  0 },
    { "keynav_focus_right", "key", 0, keynavFocusRight, 0 },
    { "keynav_focus_up",    "key", 0, keynavFocusUp,    0 },
};


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
    
    if (keynavDisplayPrivateIndex < 0) {
	compFiniMetadata (&keynavMetadata);
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

    display->base.privates[keynavDisplayPrivateIndex].ptr = keynavDisplay;

    return TRUE;
}

static Bool
keynavInitObject (CompPlugin *p, CompObject *o)
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
keynavFiniObject (CompPlugin *p,
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
    return CORE_ABIVERSION;
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
        return compSetDisplayOption(display, option, value);
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

    *count = KEYNAV_DISPLAY_OPTION_COUNT;

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

static Bool keynavInit (CompPlugin *plugin) {
    if (!compInitPluginMetadataFromInfo(&keynavMetadata,
                plugin->vTable->name, 0, 0,
                keynavOptionInfo,
                KEYNAV_DISPLAY_OPTION_COUNT))
        return FALSE;

    compAddMetadataFromFile(&keynavMetadata, plugin->vTable->name);

    return TRUE;
}

static void keynavFini (CompPlugin *plugin) {
    compFiniMetadata(&keynavMetadata);
}

static CompPluginVTable keynavVTable = {
    "keynav",
    keynavGetMetadata,
    keynavInit,
    keynavFini,
    keynavInitObject,
    keynavFiniObject,
    keynavGetObjectOptions,
    keynavSetObjectOption
};

CompPluginVTable *
getCompPluginInfo (void)
{
    return &keynavVTable;
}

CompPluginVTable *
getCompPluginInfo20070830 (void)
{
    return getCompPluginInfo();
}
