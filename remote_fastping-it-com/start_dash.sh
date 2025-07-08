#!/usr/bin/env bash
clear
echo "[INFO] Starting Customer Dashboard…"

cd /mnt/d/code/fastping-it-com || {
  echo "[ERROR] Project folder not found"; exit 1
}

# Activate venv
source venv/bin/activate || {
  echo "[ERROR] venv activation failed"; exit 1
}

# Install missing deps without touching system
pip install -r requirements.txt

# Make sure no gunicorn is running
sudo pkill -f gunicorn || true

# Run server
python3 color_customer_dashboard.py --port=9000 &
SERVER_PID=$!
echo "[INFO] Server PID: $SERVER_PID"

# Wait a moment
sleep 2

# Check port status
sudo ss -tulpn | grep 9000 || echo "[WARN] No server listening on 9000"
