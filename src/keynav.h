/**
 * Copyright (c) 2014 Scott S. McCoy <tag@cpan.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <vector>
#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>

#include "keynav_options.h"

typedef enum {
    FOCUS_DOWN,
    FOCUS_LEFT,
    FOCUS_RIGHT,
    FOCUS_UP
} FocusDirection;

class KeyboardNavigation :
    public PluginClassHandler <KeyboardNavigation, CompScreen>,
    public KeynavOptions,
    public ScreenInterface
{
    private:
	/**
	 * Simple utility to search the window list and find the nearst one.
	 */
	class NearestWindow {
	    private:
                std::vector<CompWindow*> collisions;
		CompWindow     *source;
		CompWindow     *target;
		int            targetDistance;
		FocusDirection direction;

		int distanceFrom (CompWindow *window);

	    public:
		NearestWindow (CompWindow *window, FocusDirection direction);

		/**
		 * Inspect the window, and see if it's closer in the given
		 * direction.
		 *
		 * <p>Given a window, check it's coordinates to see if it's
		 * closer than the present candidate for the closest
		 * window.  If it is, select it.</p>
		 *
		 * @param window The window to inspect.
		 */
		void inspectWindow (CompWindow *window);
		
		/**
		 * Return the currently selected closest window.
		 *
		 * @return A window, or NULL.
		 */
		CompWindow * result (void);
	};

    public:
        KeyboardNavigation (CompScreen *screen);

	/**
	 * Find the nearest window in the specified direction.
	 */
        bool initiate (CompAction         *action,
                       CompAction::State  state,
                       CompOption::Vector &option,
		       FocusDirection	  direction);
};

class KeyboardNavigationVTable :
    public CompPlugin::VTableForScreen<KeyboardNavigation>
{
    public:
        bool init ();
};

