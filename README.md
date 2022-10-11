# Horwin (Horizontal Window)
Horwin is a little program to utilize QMK Digitizer feature


## Installation

Activate digitizer feature in your keymaps by adding this line at the bottom in rules.mk
```sh
# Digitizer Enable
DIGITIZER_ENABLE = yes
```

Copy horwin.c to your keymap folder then add this line at the top in your keymap.c
```sh
#include "horwin.c"
```
