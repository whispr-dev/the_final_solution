#!/bin/bash

echo "[+] Checking port 8090..."
sudo ss -tulpn | grep 8090 || echo "[-] Nothing listening on 8090"

echo "[+] Curl homepage..."
curl -s http://127.0.0.1:8090/ && echo -e "\n[+] Homepage reachable" || echo "[-] Homepage dead"

echo "[+] Curl login..."
curl -s http://127.0.0.1:8090/login && echo -e "\n[+] Login reachable" || echo "[-] Login dead"

echo "[+] Curl create customer..."
curl -s -X POST http://127.0.0.1:8090/admin/create_customer -H "Content-Type: application/json" -d '{"name": "Test User", "email": "test@example.com"}' || echo "[-] Create customer failed"
