#!/bin/bash

# === CONFIGURATION ===
LOCAL_FILE=~/D/remote_fastping-it-com/remote/sernder.sh
REMOTE_USER=wofl
REMOTE_HOST=138.197.11.134
REMOTE_PORT=22   # ← change if not default
REMOTE_DEST=/home/wofl/proxy_test_app/destinationfile.zip

# === TRANSFER ===
echo "[INFO] Sending $LOCAL_FILE to $REMOTE_USER@$REMOTE_HOST:$REMOTE_DEST..."
scp -P "$REMOTE_PORT" "$LOCAL_FILE" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DEST"

if [ $? -eq 0 ]; then
    echo "[SUCCESS] File sent successfully!"
else
    echo "[ERROR] Transfer failed."
fi
