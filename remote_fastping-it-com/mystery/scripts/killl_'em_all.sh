#!/bin/bash

TARGET_PORT=8080

echo "[*] Checking for processes listening on port $TARGET_PORT..."

PIDS=$(sudo lsof -t -i :$TARGET_PORT)

if [ -z "$PIDS" ]; then
    echo "[*] No active listener, checking for ESTABLISHED or zombie connections..."
else
    echo "[*] Found processes holding port $TARGET_PORT: $PIDS"
    sudo kill -9 $PIDS
    echo "[*] Listener processes terminated."
fi

echo "[*] Forcing kill on any socket tied to port $TARGET_PORT..."
sudo fuser -k ${TARGET_PORT}/tcp

echo "[*] Reducing lingering TCP timeouts for stuck connections..."
sudo sysctl -w net.ipv4.tcp_fin_timeout=5

echo "[*] Final check for port $TARGET_PORT state:"
sudo ss -tulnp | grep :$TARGET_PORT || echo "[*] Port $TARGET_PORT is now clear."

echo "[*] Socket purge complete."
