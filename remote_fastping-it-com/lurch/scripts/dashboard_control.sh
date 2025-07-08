#!/bin/bash

# Configurable port for your dashboard
PORT=8090
DASH_SCRIPT="color_customer_dashboard.py"
VENV_PATH="./venv"

echo "== DASHBOARD CONTROL =="

# Step 1: Kill anything holding the port hostage
echo "[*] Checking for existing processes on port $PORT..."
PIDS=$(sudo lsof -t -i :$PORT)

if [ -n "$PIDS" ]; then
    echo "[!] Found processes using port $PORT: $PIDS"
    echo "[*] Killing them..."
    sudo kill -9 $PIDS
else
    echo "[✓] Port $PORT is free."
fi

# Step 2: Activate venv
if [ -d "$VENV_PATH" ]; then
    echo "[*] Activating virtual environment..."
    source "$VENV_PATH/bin/activate"
else
    echo "[!] Virtual environment not found at $VENV_PATH — aborting."
    exit 1
fi

# Step 3: Launch the dashboard
echo "[*] Starting dashboard on port $PORT..."
python3 "$DASH_SCRIPT" &

sleep 2

# Step 4: Confirm it's live
echo "[*] Checking dashboard status..."
curl -s "http://127.0.0.1:$PORT/" && echo -e "\n[✓] Dashboard is responding!" || echo "[!] Dashboard did not respond — check logs."

echo "== DONE =="
