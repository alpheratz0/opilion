# opilion

![opilion running on i3](extra/screenshot.jpg)

## Description

opilion is a PulseAudio volume manager for X11 with vim-like keybindings.  

For usage instructions, check:

```sh
$ man opilion
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
