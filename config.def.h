/* runtime arguments will override these settings */
const int defaultZoom = 36;         // default preview zoom level
const int defaultPrevX = 400;       // defualt preview x size (pixels)
const int defaultPrevY = 400;       // default preview y size (pixels)
const bool alwaysOneshot = false;   // always start in oneshot mode
const bool alwaysShort = false;     // always print values in short format
const bool unlockOnSelect = false;  // unlock preview if locked after making a selection
const bool verboseZoom = true;      // display a message when zooming

enum { LOCK, UNLOCK, TOGGLE };

static Key keys[] = {
    //mod,   X11 KeySym,  function,           {options}     
    { 0,     XK_l,        lockPreview,        {.i = TOGGLE}}, // l to lock
    { 0,     XK_q,        quit,               {}},            // q to quit
    { 0,     XK_equal,    change_zoom,        {.i = +2}}, // = to zoom in
    { 0,     XK_minus,    change_zoom,        {.i = -2}}, // - to zoom out
    { 0,     XK_0,        change_zoom,        {.i = 0}},  // 0 to reset zoom
};

static Button buttons[] = {
    //mod,   MouseButton, function,           {options}     
    { 0,     1,           select,             {}}, // left-click to select
    { 0,     3,           quit,               {}}, // right-click to quit
};
