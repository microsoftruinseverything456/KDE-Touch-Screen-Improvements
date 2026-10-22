#!/usr/bin/env bash
# Unload and uninstall Touch Screen Improvements, and clear its settings.

set -euo pipefail
cd "$(dirname "$(readlink -f "$0")")"

effect=touchscreenimprovements
package=kwin-touch-screen-improvements

if [[ "$(qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.isEffectLoaded "$effect" 2>/dev/null)" == true ]]; then
    qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.unloadEffect "$effect" >/dev/null
    echo ":: Unloaded."
fi

if command -v pacman >/dev/null && pacman -Qq "$package" &>/dev/null; then
    sudo pacman -R --noconfirm "$package"
else
    if [[ -f build/install_manifest.txt ]]; then
        mapfile -t files < build/install_manifest.txt
    else
        # Build directory is gone: fall back to where install.sh puts the plugins
        plugindir="$(qtpaths6 --query QT_INSTALL_PLUGINS 2>/dev/null || echo /usr/lib/qt6/plugins)"
        files=(
            "$plugindir/kwin/effects/plugins/$effect.so"
            "$plugindir/kwin/effects/configs/kwin_${effect}_config.so"
        )
    fi

    existing=()
    for file in "${files[@]}"; do
        [[ -e "$file" ]] && existing+=("$file")
    done
    if ((${#existing[@]})); then
        sudo rm -v -- "${existing[@]}"
    else
        echo ":: Nothing to remove, not installed."
    fi
fi

# Settings (see src/touchscreenimprovements.kcfg) and the enabled state from System Settings
for key in LongPressSelect LongPressRightClick TapToShowKeyboard HideKeyboardOnPhysicalInput SwipeUpForKeyboard; do
    kwriteconfig6 --file kwinrc --group "Effect-$effect" --key "$key" --delete
done
kwriteconfig6 --notify --file kwinrc --group Plugins --key "${effect}Enabled" --delete

echo ":: Uninstalled and settings cleared."
