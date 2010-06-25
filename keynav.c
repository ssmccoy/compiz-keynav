#include <stdlib.h>
#include <compiz-core.h>
#include "keynav_options.h"

#define DEBUG(message) fprintf(stderr, "%s\n", message)

#define SCAN_DOWN  0
#define SCAN_LEFT  1
#define SCAN_RIGHT 2
#define SCAN_UP    4
#define KEYNAV_DISPLAY_OPTION_COUNT 4

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

static int getDistance (int direction, int start, CompWindow *window) {
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

    fprintf(stderr, "Unreachable statement reached");
    return -1;
}

static int getStartPoint (int direction, CompWindow *window) {
    switch (direction) {
        case SCAN_RIGHT:
        case SCAN_LEFT:
            return window->serverX;
        case SCAN_UP:
        case SCAN_DOWN:
            return window->serverY;
    }

    fprintf(stderr, "Unreachable statement reached");
    return -1;
}

static Bool isFocusableWindow (CompWindow *window) {
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

static Bool withinBound (int direction, 
                         CompWindow *active, 
                         CompWindow *window)
{
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
             windowRangeEnd <= activeRangeEnd)) ||
           ((activeRangeStart >= windowRangeStart &&
             activeRangeStart <= windowRangeEnd) ||
            (activeRangeEnd >= windowRangeStart &&
             activeRangeEnd <= windowRangeEnd));
}

#define EXACT_AREA_MATCH(active, window) \
   (active->serverX      == window->serverX && \
    active->serverY      == window->serverY && \
    active->serverWidth  == window->serverWidth && \
    active->serverHeight == window->serverHeight)

static Bool sendFocus (CompDisplay *display, CompOption *option, int nOption,
        int direction) {
    int distance, selectedDistance, start = 0;
    Bool seen = FALSE;
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
            if (window == activeWindow) {
                seen = TRUE;
                continue;
            }

            if (isFocusableWindow(window) &&
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

static Bool
keynavInitDisplay (CompPlugin *plugin, CompDisplay *display) {
    if (!checkPluginABI ("core", CORE_ABIVERSION)) {
        return FALSE;
    }

    keynavSetKeynavFocusUpInitiate(    display, keynavFocusUp    );
    keynavSetKeynavFocusDownInitiate(  display, keynavFocusDown  );
    keynavSetKeynavFocusLeftInitiate(  display, keynavFocusLeft  );
    keynavSetKeynavFocusRightInitiate( display, keynavFocusRight );

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

static CompPluginVTable keynavVTable = {
    "keynav",
    0,
    0,
    0,
    keynavInitObject,
    0,
    0,
    0
};

CompPluginVTable *
getCompPluginInfo (void)
{
    return &keynavVTable;
}
