
# Building
If running on Arch install `pico-sdk` and logout-login for the environment
variables to be set.

```bash
cd src
mkdir build
cd build
cmake ..
make
```

After a successful build, you'll have the `glitcher.uf2` file in the build
directory that can be flashed:
```bash
picotool load glitcher.uf2 && picotool reboot
```

### Note for GCC13
Ensure this PR is merged: https://github.com/raspberrypi/pico-sdk/pull/1367

# Glitching
The glitcher is to be controlled with the `src/controller.py` script.

## Personal notes
I had issues with the first 2-3 glitches right after booting the Pico being
way off in duration. The same happened (but for only the first glitch) after a
dis/connect of the USB serial terminal.
I thought this was a cache issue, but for some reason, setting
`pico_set_binary_type(glitcher no_flash)` in `CMakeLists.txt` didn't work. I
had to move the glitcher code to a function of its own and decorate it with
`__no_inline_not_in_flash_func`. Additionally, the above cmake line had to be
removed as it was also triggering the same bug, regardless of the decorator (?)
Don't ask me dude.
