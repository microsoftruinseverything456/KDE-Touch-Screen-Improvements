#!/usr/bin/env bash
# Build, install and load Touch Screen Improvements. Re-run after every KWin update.

set -euo pipefail
cd "$(dirname "$(readlink -f "$0")")"

effect=touchscreenimprovements

# Build dependencies (Arch package names)
if command -v pacman >/dev/null; then
    missing=()
    for dep in cmake extra-cmake-modules vulkan-headers; do
        pacman -Qq "$dep" &>/dev/null || missing+=("$dep")
    done
    if ((${#missing[@]})); then
        echo ":: Installing build dependencies: ${missing[*]}"
        sudo pacman -S --needed --noconfirm "${missing[@]}"
    fi
fi

rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j"$(nproc)"
sudo cmake --install build

if [[ "$(qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.isEffectLoaded "$effect")" == true ]]; then
    # KWin keeps the old version in memory
    echo ":: Restarting KWin in 5 seconds to load the new version (Ctrl+C to cancel)."
    echo "   Apps that can't reconnect to a restarted compositor (many GTK/Electron/XWayland apps) will close."
    sleep 5
    kwin_wayland --replace
elif [[ "$(kreadconfig6 --file kwinrc --group Plugins --key "${effect}Enabled" --default true)" == true ]]; then
    qdbus6 org.kde.KWin /Effects org.kde.kwin.Effects.loadEffect "$effect" >/dev/null
    echo ":: Installed and loaded."
else
    echo ":: Installed. Enable it in System Settings > Window Management > Desktop Effects."
fi
