# sxcp  
A Small/Simple/Suckless X11 Color Picker  
This lightweight utility allows you to quickly and easily pick the color of
any pixel displayed on your screen, and print it to stdout.

## Usage  
As with all things 'Suckless', the config is the code and the code is the config.  
Default flags and key bindings can be found in `config.def.h`

sxcp can be run without arguments, but the following exist:  
```
  --size X Y   : Set the size of the preview window in pixels
  --short      : Only print color hex code
  --oneshot    : Quit after selecting one pixel
  -h, --help   : Prints a short help message
```

By default, left-click to print the rgb value under the mouse, L to lock the
preview window (for easier selection), and q to quit. Output will print as  
`R: ddd, G: ddd, B: ddd | HEX: #rrggbb`  


## Dependencies
- Xlib

## Compilation
`git clone https://github.com/space-pagan/sxcp.git`  
`make`  
Running `make` for the first time will generate `config.h` out of
`config.def.h`. Do not edit `config.def.h` to set your preferences. If you
want to change your configuration, edit `config.h` and recompile.

## Installation
`sudo make install` will install sxcp to `/usr/local/bin` by default. PREFIX
can be set to alter this behaviour.
