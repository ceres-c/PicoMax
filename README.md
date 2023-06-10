
# Building
If running on Arch install `pico-sdk` and logout-login for the environment
variables to be set.

```bash
cd glitcher
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

## Note for GCC13
Ensure this PR is merged: https://github.com/raspberrypi/pico-sdk/pull/1367
