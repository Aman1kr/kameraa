#!/usr/bin/env bash
# Installs and loads the v4l2loopback kernel module so Kameraa can publish a
# virtual camera on Linux. See docs/05-PLATFORM-INTEGRATION.md.
set -euo pipefail

CARD_LABEL="Kameraa"

need_root() {
    if [[ $EUID -ne 0 ]]; then
        echo "This step needs root. Re-run with sudo." >&2
        exit 1
    fi
}

install_module() {
    if modinfo v4l2loopback >/dev/null 2>&1; then
        echo "v4l2loopback already installed."
        return
    fi
    echo "Installing v4l2loopback..."
    if command -v apt-get >/dev/null; then
        apt-get update && apt-get install -y v4l2loopback-dkms
    elif command -v dnf >/dev/null; then
        dnf install -y v4l2loopback
    elif command -v pacman >/dev/null; then
        pacman -S --noconfirm v4l2loopback-dkms
    else
        echo "Unsupported distro. Install v4l2loopback manually." >&2
        exit 1
    fi
}

load_module() {
    echo "Loading v4l2loopback (label: ${CARD_LABEL})..."
    modprobe -r v4l2loopback 2>/dev/null || true
    modprobe v4l2loopback devices=1 card_label="${CARD_LABEL}" exclusive_caps=1
    echo "Done. Virtual camera device:"
    v4l2-ctl --list-devices 2>/dev/null | grep -A1 "${CARD_LABEL}" || true
}

persist_module() {
    echo "v4l2loopback" > /etc/modules-load.d/kameraa.conf
    echo "options v4l2loopback devices=1 card_label=${CARD_LABEL} exclusive_caps=1" \
        > /etc/modprobe.d/kameraa.conf
    echo "Configured to load at boot."
}

need_root
install_module
load_module
if [[ "${1:-}" == "--persist" ]]; then
    persist_module
fi
