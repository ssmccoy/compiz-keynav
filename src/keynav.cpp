#include <iostream>
#include <string>
#include <memory>
#include "keynav.h"

#define SCAN_DOWN  0
#define SCAN_LEFT  1
#define SCAN_RIGHT 2
#define SCAN_UP    4
#define KEYNAV_DISPLAY_OPTION_COUNT 4

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
start( window->x() + (window->width() / 2),
       window->y() + (window->height() / 2) ),
targetDistance(-1),
direction(direction)
{
    DEBUG_LOG("finding window nearest " << window->id() << " " <<
              direction << ": " <<
              window->x() << "x" << window->y());
}

int
KeyboardNavigation::NearestWindow::distanceFrom (CompWindow *window)
{
    DEBUG_LOG("calculating " << direction << " distance to " << window->id());

    CompPoint center ( window->x() + (window->width() / 2),
                       window->y() + (window->height() / 2) );

    switch (direction) {
        case FOCUS_DOWN:
            return center.y() - start.y();
        case FOCUS_LEFT:
            return start.x() - center.x();
        case FOCUS_RIGHT:
            return center.x() - start.x();
        case FOCUS_UP:
            return start.y() - center.y();
    }

    throw "Illegal State";
}

bool
KeyboardNavigation::NearestWindow::lateralCollision (CompWindow *window)
{
    CompPoint center ( window->x() + (window->width() / 2),
                       window->y() + (window->height() / 2) );

    switch (direction) {
        case FOCUS_UP:
        case FOCUS_DOWN:
            return center.x() >= source->x() &&
                   center.x() <= source->x() + source->width();
        case FOCUS_LEFT:
        case FOCUS_RIGHT:
            return center.y() >= source->y() &&
                   center.y() <= source->y() + source->height();
    }

    throw "Illegal State";
}

#define UNDESIRABLE_WINDOW(window, sg) \
        !window->isFocussable()                                 || \
        !window->isViewable()                                   || \
        !window->isMapped()                                     || \
        window->shaded()                                        || \
        window->minimized()                                     || \
        (window->type() & (CompWindowTypeDesktopMask |             \
                            CompWindowTypeDockMask))            || \
        (window->state() & CompWindowStateSkipTaskbarMask)      || \
        (sg.x() + sg.width()  <= 0                              || \
         sg.y() + sg.height() <= 0                              || \
         sg.x() >= (int) ::screen->width()                      || \
         sg.y() >= (int) ::screen->height())                    || \
        !window->onCurrentDesktop()

void
KeyboardNavigation::NearestWindow::inspectWindow (CompWindow *window)
{
    const CompWindow::Geometry &sg = window->serverGeometry();

    /* This is a completely unmaintainable expression */
    if (window->id() == source->id() || UNDESIRABLE_WINDOW(window, sg)) {
        DEBUG_LOG("Ignoring window " << window->id());
        return;
    }

    if (!lateralCollision(window)) {
        return;
    }

    int distance = distanceFrom(window);

    DEBUG_LOG("Inspected window " << window->id() << " has distance " <<
              distance);

    if (distance == 0) {
        if (target == NULL) {
            target = window;
        }
    }
    else if (distance > 0) {
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
