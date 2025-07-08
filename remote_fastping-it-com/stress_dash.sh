# Linux:
curl -L https://hey-release.s3.us-east-2.amazonaws.com/hey_linux_amd64 -o ~/bin/hey
chmod +x ~/bin/hey

# Windows (PowerShell):
Invoke-WebRequest https://hey-release.s3.us-east-2.amazonaws.com/hey_windows_amd64 -OutFile hey.exe

#!/usr/bin/env bash
clear
hey -n 500 -c 50 -m POST \
  -H "Content-Type: application/json" \
  -d '{"name":"LoadTest","email":"load@test.com"}' \
  http://127.0.0.1:9000/admin/create_customer
