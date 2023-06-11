
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

### Glitcher commands
I have tried to reduce the amount of commands necessary for an explorative
sweep to just one atomic command, but it didn't yield any actual speedup
compared to the separate `W`, `D` and `G` commands.

### Note for GCC13
Ensure this PR is merged: https://github.com/raspberrypi/pico-sdk/pull/1367

# Glitching
The glitcher is to be controlled with the `src/controller.py` script.

## Delay and width calculation
The delay and width are in Raspberry Pi Pico clock cycles. The Pico runs at
125MHz (save ring oscillator variability), thus delays can be varied by 8ns
increments.

With my specific MAX4619, switching time is ${\sim}10\ \text{ns}$ (quite close
to the limit of my oscilloscope, so take this with a grain of salt). A 0-`nop`
delay (the minimum possible low-high-low time at the output) measures at
${\sim}60\ \text{ns}$ *with no load*. Each additional `nop` adds
${\sim}30\ \text{ns}$ to the delay (it's not actually the `nop`, but rather the
loop exit condition check, but let's ignore that).

Out of the mentioned $60\ \text{ns}$, $10+10$ are switching times, and the
remaining $40$ are taken by the Pico to execute the loop's head. This gives us
this ballpark formula for the delay:

$$\text{delay} = 20ns_{\text{switching}} + 40ns_{\text{loop setup}} + \\#\text{nop}\ \cdot\ 30ns_{\text{nop}}$$

Same applies for the glitch width.

## Glitching improvements
The glitcher seems to be relatively consistent when code is running in ram
(already enabled, read next section), but probably it could be improved in both
consistency and accuracy by moving the glitcher code to PIO.

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
