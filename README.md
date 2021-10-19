# i3applyinput

This tool applys my GSettings input configuration using X11. This is useful when I run i3. Note that GSD no longer works for this purpose because gsd-mouse has been removed and all of its functionality moved into the compositor (mutter) itself.

# Build

```
sudo apt install libxcb-xinput-dev

meson setup build
cd build
ninja
```
