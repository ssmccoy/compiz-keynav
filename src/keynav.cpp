#include <iostream>
#include <string>
#include "keynav.h"

#define SCAN_DOWN  0
#define SCAN_LEFT  1
#define SCAN_RIGHT 2
#define SCAN_UP    4
#define KEYNAV_DISPLAY_OPTION_COUNT 4

#define DEBUG true

#ifdef DEBUG
#define DEBUG_LOG(msg) std::cerr << "[keynav] " << msg << std::endl
#else
#define DEBUG_LOG(msg)
#endif

COMPIZ_PLUGIN_20090315 (keynav, KeyboardNavigationVTable);

KeyboardNavigation::KeyboardNavigation (CompScreen *screen) :
    PluginClassHandler <KeyboardNavigation, CompScreen> (screen),
    KeynavOptions()
{
    ScreenInterface::setHandler(screen);

    DEBUG_LOG("Enabled Plugin");

#define setAction(action, type) \
    optionSet##action##Initiate(boost::bind(    \
            &KeyboardNavigation::initiate,      \
            this, _1,_2,_3,type))
    setAction( KeynavFocusDown,  FOCUS_DOWN  );
    setAction( KeynavFocusLeft,  FOCUS_LEFT  );
    setAction( KeynavFocusRight, FOCUS_RIGHT );
    setAction( KeynavFocusUp,    FOCUS_UP    );

    DEBUG_LOG("Set up actions");
}

KeyboardNavigation::NearestWindow::NearestWindow (CompWindow     *window,
                                                  FocusDirection direction) :
source(window),
target(NULL),
targetDistance(-1),
direction(direction)
{
    DEBUG_LOG("finding window nearest " << window->id() << " " <<
              direction << ": " <<
              window->x() << "x" << window->y());

    /*
    source    = window;
    direction = direction;
    target    = NULL;
    */
}

int
KeyboardNavigation::NearestWindow::distanceFrom (CompWindow *window)
{
    DEBUG_LOG("calculating " << direction << " distance to " << window->id());

    switch (direction) {
        case FOCUS_DOWN:
            return window->y() - source->y();
        case FOCUS_LEFT:
            return source->x() - window->x();
        case FOCUS_RIGHT:
            return window->x() - source->x();
        case FOCUS_UP:
            return source->y() - window->y();
    }

    throw "Illegal State";
}

void
KeyboardNavigation::NearestWindow::inspectWindow (CompWindow *window)
{
    const CompWindow::Geometry &sg = window->serverGeometry();

    /* This is a completely unmaintainable expression */
    if (window->id() == source->id()                            ||
        !window->isFocussable()                                 ||
        !window->isViewable()                                   ||
        !window->isMapped()                                     ||
        window->shaded()                                        ||
        (window->type() & (CompWindowTypeDesktopMask |
                            CompWindowTypeDockMask))            ||
        (window->state() & CompWindowStateSkipTaskbarMask)      ||
        (sg.x() + sg.width()  <= 0                              ||
         sg.y() + sg.height() <= 0                              ||
         sg.x() >= (int) ::screen->width()                      ||
         sg.y() >= (int) ::screen->height())                    ||
        !window->onCurrentDesktop())
    {
        DEBUG_LOG("Ignoring window " << window->id());
        return;
    }

    int distance = distanceFrom(window);

    DEBUG_LOG("Inspected window " << window->id() << " has distance " <<
              distance);

    if (distance >= 0) {
        if (target == NULL) {
            target         = window;
            targetDistance = distance;
        }
        else if (distance < targetDistance) {
            target         = window;
            targetDistance = distance;
        }
    }
}

CompWindow *
KeyboardNavigation::NearestWindow::result (void) {
    return target;
}

bool
KeyboardNavigation::initiate (CompAction         *action,
                              CompAction::State  state,
                              CompOption::Vector &option,
                              FocusDirection     direction)
{
    CompWindow *window = screen->findWindow( screen->activeWindow() );

    if (window) {
        /* Don't allow focus change for certain special windows */
        if (window->overrideRedirect() ||
            (window->type() & (CompWindowTypeDesktopMask |
                               CompWindowTypeDockMask)))
        {
            return false;
        }

        DEBUG_LOG("Initiating search for nearest window: " << direction);

        NearestWindow nearestWindow (window, direction);

        screen->forEachWindow( boost::bind(
                &KeyboardNavigation::NearestWindow::inspectWindow,
                &nearestWindow, _1
                ) );

        CompWindow *focusWindow = nearestWindow.result();

        if (focusWindow != NULL) {
            DEBUG_LOG("Found focusWindow: " << focusWindow->id() << " " << 
                      focusWindow->x() << "x" << focusWindow->y());

            focusWindow->moveInputFocusTo();
            focusWindow->activate();

            return true;
        }
        else {
            DEBUG_LOG("Unable to find focus window!");
        }
    }

    return false;
}


bool KeyboardNavigationVTable::init () {
    DEBUG_LOG("Initializaing plugin");

    if (!CompPlugin::checkPluginABI("core", CORE_ABIVERSION))
        return false;

    return true;
}
