/* runtime arguments will override these settings */
const int defaultZoom = 40;         // default preview zoom level
const int defaultPrevX = 400;       // defualt preview x size (pixels)
const int defaultPrevY = 400;       // default preview y size (pixels)
const bool alwaysOneshot = false;   // always start in oneshot mode
const bool alwaysShort = false;     // always print values in short format
const bool unlockAfterSelection = false; // unlock preview if locked after making a selection

static Key keys[] = {
    {0, XK_l, lock, {}},
};
