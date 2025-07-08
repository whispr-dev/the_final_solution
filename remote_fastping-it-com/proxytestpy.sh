#!/usr/bin/env bash
clear
URL="http://127.0.0.1:9000"
echo "[INFO] Testing GET /login"
curl -s -o /dev/null -w "HTTP %{http_code}\n" "$URL/login" || echo "[FAIL] Server unreachable"

echo "[INFO] Testing POST /admin/create_customer"
curl -s -o /dev/null -w "HTTP %{http_code}\n" \
  -X POST "$URL/admin/create_customer" \
  -H "Content-Type: application/json" \
  -d '{"name":"LoadTest","email":"load@test.com"}' \
  || echo "[FAIL] POST failed"
