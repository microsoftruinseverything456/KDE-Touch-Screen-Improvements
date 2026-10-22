# Touch Screen Improvements for KDE Plasma

An experimental KWin desktop effect that makes Plasma (Wayland) nicer to use on a touch screen.

- **Touch and hold to right-click.** Hold a finger still for a moment and lift it.
- **Touch, hold and drag to highlight text word by word.** Lifting opens the right-click menu on the selection.
- **The on-screen keyboard only appears when you tap a text field.** Not when scrolling over one, switching to a window with a field already selected, or long-pressing.
- **Using a real mouse, touchpad or keyboard hides the on-screen keyboard**, and typing on the real keyboard doesn't bring it back.
- **Swipe up from the bottom edge to show the on-screen keyboard**, even without a text field selected. With a right-click menu open, the swipe closes the menu first, e.g. to delete highlighted text.

Each feature can be turned off. Works with [plasma-keyboard](https://invent.kde.org/plasma/plasma-keyboard) and other input-method-v1 virtual keyboards.

## Requirements

- KDE Plasma 6 on Wayland. Developed and tested with KWin 6.8.
- To build: CMake, extra-cmake-modules, and the development files of KWin, Qt 6 and KDE Frameworks 6 (KConfig, KCoreAddons, KCMUtils). KWin's CMake package also needs the Vulkan headers.

Nothing beyond Plasma itself is needed to run it.

> [!IMPORTANT]
> KWin has no stable plugin API, so the effect has to be built against the exact KWin version you run. **Rebuild it after every KWin update.** KWin skips effects built for another version, so until you rebuild, the effect is simply off.

## Install

### Arch Linux and derivatives

```sh
./install.sh
```

Installs missing build dependencies, builds, installs to `/usr` and loads the effect (no restart needed on first install). Updating an already loaded effect restarts KWin, since KWin keeps the loaded version in memory; apps that can't reconnect to a restarted compositor will close.

Alternatively, build a pacman package with the `PKGBUILD` in `packaging/arch/`.

### Other distributions

Install the build dependencies listed above, then:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
sudo cmake --install build
qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.loadEffect touchscreenimprovements
```

## Usage

**System Settings > Window Management > Desktop Effects > Tools > Touch Screen Improvements.** The checkbox turns everything on or off; the configure button next to it has a setting for each feature (all on by default). Changes apply immediately.

## How it works

- **Tap to show keyboard:** KWin shows the keyboard whenever a text field gets focus after touch input, and many apps focus fields as soon as a finger lands on them, or re-request the keyboard on every frame while scrolling. The effect takes over KWin's handlers that show the keyboard and only lets them through after a tap on the window that owns the focused text field. Requests it holds back are remembered, so tapping the field shows the keyboard even if the app doesn't ask again.
- **Long press:** KWin doesn't know where text is inside a window, so the effect emulates a mouse. After a hold, the touch is cancelled for the app (so its own long-press handling doesn't also fire) and a virtual pointer right-clicks, or double-click-drags, which most toolkits treat as word-wise selection. A plain hold never double-clicks, so long-pressing a file or icon won't open it.
- **Physical input:** only real (libinput) devices count, not emulated input or keys sent by the virtual keyboard. Volume, brightness, media and power keys are ignored.
- **Swipe up:** uses KWin's bottom touch screen edge, which KWin disables over fullscreen windows unless screen edges are kept active there:
  `kwriteconfig6 --notify --file kwinrc --group ScreenEdges --key RemainActiveOnFullscreen true`

## Known limitations

- How well touch works still depends on the app: apps without real touch support (e.g. many terminals) may not scroll or select well.
- Word-wise selection needs the app to support double-click-and-drag selection.
- The mouse cursor appears where emulated clicks happen.
- KWin runs only one action per screen edge: if something else uses the bottom touch edge, one of them won't work.

## Uninstall

Remove the files listed in `build/install_manifest.txt` (`sudo xargs rm < build/install_manifest.txt`) and restart KWin (`kwin_wayland --replace`), or uninstall the pacman package.

## Debugging

Add `kwin_touchscreenimprovements.debug=true` to the `[Rules]` section of `~/.config/QtProject/qtlogging.ini`, restart KWin and follow `journalctl --user -f -u plasma-kwin_wayland`.

## License

GPL-2.0-or-later, see [LICENSE](LICENSE).
