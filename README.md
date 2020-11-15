# sxcp  
A Simple/Small/Suckless X11 Color Picker
This lightweight utility allows you to quickly and easily pick the color of any pixel displayed on your screen, and print it to stdout.

## Usage  
As with all things 'Suckless', the config is the code and the code is the config.  
Default flags and key bindings can be found in `config.def.h`

sxcp can be run without arguments, but the following exist:
`  --size X Y   : Set the size of the preview window in pixels
   --short      : Only print color hex code
   --oneshot    : Quit after selecting one pixel
   -h, --help   : Prints a short help message`

## Dependencies
- Xlib

## Compilation
`git clone` this repository
`make`

## Installation
`sudo make install` will install sxcp to `/usr/local/bin` by default. PREFIX can be set to alter this behaviour.
