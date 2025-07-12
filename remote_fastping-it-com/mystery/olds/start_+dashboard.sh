#!/bin/bash

echo "[+] Killing rogue gunicorn..."
sudo pkill -f gunicorn

echo "[+] Killing stray dashboard python..."
sudo pkill -f color_customer_dashboard.py

echo "[+] Waiting briefly..."
sleep 2

echo "[+] Activating venv..."
source /home/wofl/proxy_test_app/venv/bin/activate

echo "[+] Launching Customer Dashboard..."
python3 /home/wofl/proxy_test_app/color_customer_dashboard.py
