# opilion

![opilion running on i3](extra/screenshot.jpg)

## Description

opilion is a PulseAudio volume manager for X11 with vim-like keybindings.

For usage instructions, check:

```sh
$ man opilion
```

## Theming

Edit `include/config.h` to change colors & font

## Dependencies

To run opilion, you need the following libraries installed: libxcb, libxcb-image, libxcb-keysyms, libxcb-xkb, libxcb-shm, libxcb-cursor, libpulse, libfcft and libpixman-1

## Building and installing

```sh
$ make
$ sudo make PREFIX=/usr install
```

Alternatively, opilion is available on the Arch User Repository (AUR)

```sh
$ yay --noconfirm -S opilion
```

## License

Code is licensed under GNU's General Public License v2. See `COPYING` for details.
