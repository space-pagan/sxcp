#include <X11/Xlib.h>       // Xlib
#include <X11/Xutil.h>      // XClassHint()
#include <X11/cursorfont.h> // to allow crosshair
#include <X11/keysym.h>     // X11 key symbols
#include <string>           // std::string
#include <vector>           // std::vector
#include <iostream>         // cout, cerr, etc...
#include <chrono>           // std::chrono
#include <thread>           // std::thread
#include <cstring>          // sprintf

static Display* d;
static Window w;
static Window root;
static GC gc;
static int zoom;
static bool shortflag;
static bool oneshotflag;
static bool previewLocked;
static int pixArraySizeX;
static int pixArraySizeY;
static int offsetX;
static int offsetY;
static XColor** pixels = nullptr;

void die(std::string msg);
void setWMClass();
void getCurrentSize(unsigned int& width, unsigned int& height);
void prepareXWindow(int sizeX, int sizeY);
void getPixels(int x, int y);
void getMouseCoordinates(int& x, int& y);
int getInvertedColor(XColor* color);
void prepareMouse();
void prepareKeyboard();
void fixOffset(int mousePosX, int mousePosY, int screenheight, int screenwidth);
void drawPixelPreview();
void drawPixelSelector();
void cleanup();
void fixDimenstonVariables();
void main_loop(int previewSizeX, int previewSizeY);
void parseCLI(int argc, char** argv, int& previewSizeX, int& previewSizeY);

typedef union {
    int i;
    unsigned int ui;
    float f;
    bool b;
    const void* v;
} Arg;

typedef struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(const Arg* arg);
    const Arg arg;
} Key;

typedef struct {
    unsigned int mod;
    unsigned int button;
    void (*func)(const Arg* arg);
    const Arg arg;
} Button;

void lockPreview(const Arg* arg);
void quit(const Arg* arg);
void select(const Arg* arg);
void change_zoom(const Arg* arg);

#include "config.h"         // user constants

void die(std::string msg) {
    std::cerr << msg;
    exit(-1);
}

void setWMClass() {
    XClassHint classhint;
    char hint_name[10] = "colortest";
    char hint_class[10] = "colortest";
    classhint.res_name = hint_name;
    classhint.res_class = hint_class;
    XSetClassHint(d, w, &classhint);
}

void getCurrentSize(unsigned int& width, unsigned int& height) {
    Window rootwin;
    int posx, posy;
    unsigned int border, bitdepth;
    XGetGeometry(d, w, &rootwin, &posx, &posy, &width, &height, &border, &bitdepth);
}

bool isMouseOnWindow(int mousePosX, int mousePosY) {
    Window rootwin;
    int winPosX, winPosY;
    unsigned int width, height, border, bitdepth;
    XGetGeometry(d, w, &rootwin, &winPosX, &winPosY, &width, &height, &border, &bitdepth);
    if ((unsigned int)mousePosX >= (unsigned int)winPosX &&
        (unsigned int)mousePosX <= (unsigned int)(winPosX + width) &&
        (unsigned int)mousePosY >= (unsigned int)winPosY &&
        (unsigned int)mousePosY <= (unsigned int)(winPosY + height))
            return true;
    return false;
}

void prepareXWindow(int sizeX, int sizeY) {
    d = XOpenDisplay(NULL); // open the default display
    root = DefaultRootWindow(d);
    if (d == NULL) die("Unable to connect to display\n");

    int whiteColor = WhitePixel(d, DefaultScreen(d));
    int screenheight = DisplayHeight(d, DefaultScreen(d));

    w = XCreateSimpleWindow(d, root, 0, 0, sizeX, sizeY, 
            0, whiteColor, whiteColor);
    setWMClass();
    
    // Request X to report events. StructureNotifyMask for any change in window structure
    XSelectInput(d, w, StructureNotifyMask);
    XMapWindow(d, w); // Map the window to the screen
    gc = XCreateGC(d, w, 0, nullptr); // create Graphics Context

    // wait for MapNotify event
    for (XEvent e; e.type != MapNotify; XNextEvent(d, &e));

    // reposition window after any windows managers have attempted to relocate 
    XMoveResizeWindow(d, w, 0, screenheight-sizeY-15, sizeX, sizeY);
}

void getPixels(int x, int y) {
    XImage* image = XGetImage(d, root, x, y, pixArraySizeX, pixArraySizeY, 
            AllPlanes, XYPixmap);
    for (int i = 0; i < pixArraySizeX; i++) {
        for (int j = 0; j < pixArraySizeY; j++) {
            pixels[i][j].pixel = XGetPixel(image, i, j);
        }
    }
    XFree(image);
    XQueryColor(d, DefaultColormap(d, DefaultScreen(d)), &pixels[offsetX][offsetY]);
}

void getMouseCoordinates(int& x, int& y) {
    Window root_win, child_win;
    int win_x, win_y;
    unsigned int mask;
    XQueryPointer(d, root, &root_win, &child_win, &x, &y, &win_x, &win_y, &mask);
}

int getInvertedColor(XColor* color) {
    int r = color->red / 256;
    int g = color->green / 256;
    int b = color->blue / 256;
    int ri = 255 - r;
    int gi = 255 - g;
    int bi = 255 - b;
    return ri*65536 + gi*256 + bi;
}

void prepareMouse() {
    Cursor c = XCreateFontCursor(d, XC_crosshair);
    XGrabPointer(d, root, false, ButtonPressMask, GrabModeAsync, 
            GrabModeAsync, None, c, CurrentTime);
}

void prepareKeyboard() {
    XGrabKeyboard(d, root, false, GrabModeAsync, GrabModeAsync, CurrentTime);
}

void fixOffset(int mousePosX, int mousePosY, int screenheight, int screenwidth) {
    offsetX = pixArraySizeX/2;
    offsetY = pixArraySizeY/2;
    if (mousePosX - offsetX < 0) offsetX = mousePosX;
    if (mousePosY - offsetY < 0) offsetY = mousePosY;
    if (mousePosX + offsetX >= screenwidth)
        offsetX = pixArraySizeX - screenwidth + mousePosX;
    if (mousePosY + offsetY >= screenheight)
        offsetY = pixArraySizeY - screenheight + mousePosY;
}

void drawPixelPreview() {
    for (int x = 0; x < pixArraySizeX; x++) {
        for (int y = 0; y < pixArraySizeY; y++) {
            XSetForeground(d, gc, pixels[x][y].pixel);
            XFillRectangle(d, w, gc, x*zoom, y*zoom, zoom, zoom);
        }
    }
}

void drawPixelSelector() {
        int inv = getInvertedColor(&pixels[offsetX][offsetY]);
        XSetForeground(d, gc, inv);
        XDrawRectangle(d, w, gc, offsetX*zoom, offsetY*zoom, zoom, zoom);
}

void cleanup() {
    for (int i = 0; i < pixArraySizeX; i++) {
        delete[] pixels[i];
    }
    delete[] pixels;
    XFree(gc);
    XDestroyWindow(d, w);
    XCloseDisplay(d);
    exit(0);
}

void lockPreview(const Arg* arg) {
    switch(arg->i) {
        case LOCK:
            previewLocked = true;
            break;
        case UNLOCK:
            previewLocked = false;
            break;
        case TOGGLE:
            previewLocked = !previewLocked;
            break;
    }
}

void quit(const Arg* arg) {
    cleanup();
}

void select(const Arg* arg) {
    // assume that arg->v = XColor*
    XColor center = pixels[offsetX][offsetY];
    if (shortflag) {
        printf("#%02x%02x%02x\n", 
                center.red/256, 
                center.green/256, 
                center.blue/256); 
    } else {
        printf("R: %3d, G: %3d, B: %3d | HEX: #%02x%02x%02x\n",
                center.red/256, 
                center.green/256, 
                center.blue/256,
                center.red/256, 
                center.green/256, 
                center.blue/256); 
    }
    if (unlockOnSelect) {
        previewLocked = false;
    }
    if (oneshotflag) {
        cleanup();
    }

}

void fixDimensionVariables() {
    if (pixels != nullptr) {
        for (int i = 0; i < pixArraySizeX; i++) {
            delete[] pixels[i];
        }
        delete[] pixels;
    }
    unsigned int dispwidth, dispheight;
    getCurrentSize(dispwidth, dispheight);
    pixArraySizeX = std::max((int)dispwidth/zoom, 1);
    pixArraySizeY = std::max((int)dispheight/zoom, 1);
    pixels = new XColor*[pixArraySizeX];
    for (int i = 0; i < pixArraySizeX; i++) {
        pixels[i] = new XColor[pixArraySizeY];
    }
}

void change_zoom(const Arg* arg) {
    unsigned int dispwidth, dispheight;
    getCurrentSize(dispwidth, dispheight);
    if (arg->i == 0) {
        zoom = defaultZoom;
    } else {
        int divisor = pixArraySizeX - arg->i;
        if (divisor > (int)dispwidth || divisor <= 0) return; // avoid fpe
        int oldzoom = zoom;
        zoom = dispwidth / (pixArraySizeX - arg->i);
        if (zoom == oldzoom) zoom += arg->i;
    }
    fixDimensionVariables();
    if (verboseZoom) {
        std::cout << "Zoom changed to X" << zoom << ", (" << pixArraySizeX;
        std::cout << " px * " << pixArraySizeY << " px)\n";
    }
    previewLocked = false; // redraw preview after zoom change
}

void main_loop(int previewSizeX, int previewSizeY) {
    prepareXWindow(previewSizeX, previewSizeY);
    prepareMouse();
    prepareKeyboard();
    XEvent event;
    XSelectInput(d, root, KeyPressMask | KeyReleaseMask);

    fixDimensionVariables();
    int screenwidth, screenheight;
    int mousePosX, mousePosY;
    screenwidth = DisplayWidth(d, DefaultScreen(d));
    screenheight = DisplayHeight(d, DefaultScreen(d));

    while(!0) {
        getMouseCoordinates(mousePosX, mousePosY);
        fixOffset( mousePosX, mousePosY, screenheight, screenwidth);
        getPixels(mousePosX-offsetX, mousePosY-offsetY);
        if (!previewLocked && !isMouseOnWindow(mousePosX, mousePosY)) {
            drawPixelPreview();
            drawPixelSelector();
        }
        XFlush(d);
        if (XCheckWindowEvent(d, root, ButtonPressMask | KeyPressMask, &event)) {
            switch(event.type) {
                case ButtonPress:
                    for (Button b : buttons) {
                        if (b.button == event.xbutton.button) {
                            b.func(&b.arg);
                        }
                    }
                    break;

                case KeyPress:
                    for (Key k : keys) {
                        if (k.keysym == XLookupKeysym(&event.xkey, 0)) {
                            k.func(&k.arg);
                        }
                    }
                    break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    cleanup();
}

bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool stringcomp(std::string a, std::string b) {
    return a == b;
}

void parseCLI(int argc, char** argv, int& previewSizeX, int& previewSizeY) {
    int previewsizecounter = -1;
    previewSizeX = defaultPrevX;
    previewSizeY = defaultPrevY;
    zoom = defaultZoom;
    shortflag = alwaysShort;
    oneshotflag = alwaysOneshot;
    bool helpflag = false;
    for (int i = 1; i < argc; i++) {
        if (stringcomp(argv[i], "--size")) {
            previewsizecounter = 0;
        } else if (stringcomp(argv[i], "--short")) {
            shortflag = true;
        } else if (stringcomp(argv[i], "--oneshot")) {
            oneshotflag = true;
        } else if (stringcomp(argv[i], "-h") || stringcomp(argv[i], "--help")) {
            helpflag = true;
        } else {
            if (is_number(argv[i])) {
                if (previewsizecounter < 0 || previewsizecounter > 1) {
                    std::cout << "Unknown argument '" << argv[i] << "'\n";
                    exit(-1);
                } else {
                    previewsizecounter++;
                    if (previewsizecounter == 1) {
                        previewSizeX = std::stoi(argv[i]);
                    } else {
                        previewSizeY = std::stoi(argv[i]);
                    }
                }
            } else {
                std::cout << "Unknown argument '" << argv[i] << "'\n";
                exit(-1);
            }
        }
    }

    if (previewsizecounter != 2 && previewsizecounter != -1) {
        std::cout << "Incorrect format, use --size X Y\n";
        exit(-1);
    }

    if (helpflag) {
        std::cout << "Usage: sxcp [OPTION]\n";
        std::cout << "Print rgb values for each selected pixel and show a ";
        std::cout << "zoomed preview of pixels\nunderneath the mouse.\n";
        std::cout << "\nOptions:\n";
        std::cout << "  --size X Y        Set the size of the preview window as X pixels by Y pixels\n";
        std::cout << "  --short           Only print rgb hex falue #xxxxxx\n";
        std::cout << "  --oneshot         Quit after printing colorcode for the first selected pixel\n";
        std::cout << "  -h, --help        Print this message\n";
        exit(0);
    }
}

int main(int argc, char** argv) {
    // read stuff from cli
    int previewSizeX;
    int previewSizeY;
    parseCLI(argc, argv, previewSizeX, previewSizeY);
    // actual program
    main_loop(previewSizeX, previewSizeY);
    return 0;
}
