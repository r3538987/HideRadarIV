# HideRadar

HideRadar is a GTA IV plugin-sdk ASI plugin that lets you hide or restore the
in-game radar with a keyboard key or gamepad button.

When dynamic warnings are enabled, the plugin can display a low-health message
while the radar is hidden.

## Installation

1. Build `HideRadar.sln` for the GTA4 configuration, or use a prebuilt release.
2. Place `HideRadar.IV.asi` in the game's `scripts` directory.
3. Start the game once to generate `HideRadar.IV.ini` beside the ASI file.

An ASI loader compatible with your GTA IV installation is required.

## Configuration

Default `HideRadar.IV.ini`:

```ini
KeyboardKey=73
GamepadButton=9
Dynamic=0
HealthThreshold=30
Debug=0
WarningText="Low Health"
```

The configuration is read when the plugin starts. Restart the game after
editing it.

### KeyboardKey

Windows virtual-key code used to toggle the radar.

- Default: `73` (`I`)
- Set to `0` to disable the keyboard shortcut.
- Examples: `72` = H, `82` = R, `120` = F9.

### GamepadButton

plugin-sdk gamepad button ID used to toggle the radar.

- Default: `9` (D-pad Down)
- Set to `-1` to disable the gamepad shortcut.

Common button IDs:

| Value | Button |
| ---: | --- |
| 4 | Left bumper |
| 5 | Left trigger |
| 6 | Right bumper |
| 7 | Right trigger |
| 8 | D-pad Up |
| 9 | D-pad Down |
| 10 | D-pad Left |
| 11 | D-pad Right |
| 12 | Start |
| 13 | Select/Back |
| 14 | X/Square |
| 15 | Y/Triangle |
| 16 | A/Cross |
| 17 | B/Circle |
| 18 | Left stick button |
| 19 | Right stick button |

### Dynamic

Controls the low-health warning.

- `0`: disabled
- `1`: enabled

The warning appears only when dynamic mode is enabled, the radar is hidden,
and player health is below `HealthThreshold`. Dynamic mode does not make the
radar visible.

### HealthThreshold

Health percentage below which the warning appears.

- Default: `30`
- Valid range: `0` to `100`

For example, `HealthThreshold 25` displays the warning below 25% health. Left lower corner. 

### Debug

Controls diagnostic file logging.

- `0`: disabled
- `1`: enabled

When enabled, `HideRadar.IV.log` is updated beside the INI file approximately
once per second. It records raw health, calculated health percentage, dynamic
state, warning state, and radar state.

### WarningText

Text displayed by the low-health warning.

- Default: `"Low Health"`
- Keep text containing spaces inside quotation marks.
- An empty value falls back to `Low Health`.

Example:

```ini
WarningText "Find cover!"
```

## Usage

Press the configured keyboard key or gamepad button to toggle the radar. Input
is edge-triggered, so holding the button does not repeatedly toggle it.
