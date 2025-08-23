# opilion

![opilion running on i3](extra/screenshot.jpg)

## Description

opilion is a PulseAudio volume manager for X11 with vim-like keybindings.  

For usage instructions, check:

```sh
$ man opilion
```

## Configuration

Modify opilion's include/config.h file to customize opilion to your tasting

```c
// Window settings
#define OPILION_BACKGROUND_COLOR 0x000000
#define OPILION_SINK_SELECTOR_WIDTH 900
#define OPILION_SINK_SELECTOR_HEIGHT 1000

// Font settings
#define OPILION_FONT_FAMILY "Terminus"
#define OPILION_FONT_SIZE 12

// Normal theme
#define OPILION_THEME_NORMAL_SINK_NAME 0xffffff
#define OPILION_THEME_NORMAL_VOLUME 0xffffff
#define OPILION_THEME_NORMAL_VOLUME_BAR_FILL 0x333333
#define OPILION_THEME_NORMAL_VOLUME_BAR_BG 0x555555

// Selected sink theme
#define OPILION_THEME_SELECTED_SINK_NAME 0xa0e547
#define OPILION_THEME_SELECTED_VOLUME 0xa0e547
#define OPILION_THEME_SELECTED_VOLUME_BAR_FILL 0x333333
#define OPILION_THEME_SELECTED_VOLUME_BAR_BG 0x5e5eed /* just like the seeed */
```

## Dependencies

To run opilion, you need the following libraries installed:

- libxcb
- libxcb-image
- libxcb-keysyms
- libxcb-xkb
- libxcb-shm
- libxcb-cursor
- libpulse
- libfcft
- libgrapheme
- libpixman-1

## Building and installing

```sh
$ make
$ sudo make PREFIX=/usr install
```

## License

Code is licensed under GNU's General Public License v2. See `COPYING` for details.
