===============================================================================
Compiz Focus Switcher Plugin
===============================================================================

This plugin for the compiz window manager allows a user to send the window
focus in a specific direction by scanning for windows with lateral collisions
with the present focus window, and focusing on and raising the nearest window
in the given direction.

This means if you have two tiled windows, directly next to one vertically
maximized window, you can easily chose which window to focus on without
considering any other windows or application in that application area.

+-------------------+-------------------+
|                   |                   |
|   Left Vertical   |     Right Top     |
|                   |                   |
|    (Focused)      +-------------------+
|                   |                   |
|                   |    Right Bottom   |
|                   |                   |
+-------------------+-------------------+

If you desire to focus on the group to right, you can press (for example)
Super+Right, or, if you wish to send the focus back to the window on the left,
you can simply press Super+Left.

After pressing Super+Right, your focus will (likely) be:

+-------------------+-------------------+
|                   |                   |
|   Left Vertical   |     Right Top     |
|                   |     (Focused)     |
|                   +-------------------+
|                   |                   |
|                   |    Right Bottom   |
|                   |                   |
+-------------------+-------------------+

Following this with Super+Down, will result in the following:

+-------------------+-------------------+
|                   |                   |
|   Left Vertical   |     Right Top     |
|                   |                   |
|                   +-------------------+
|                   |                   |
|                   |    Right Bottom   |
|                   |     (Focused)     |
+-------------------+-------------------+
