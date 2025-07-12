#!/bin/bash
# Complete Speed Test & Benchmark Suite
# Run as: chmod +x speed_tests.sh && ./speed_tests.sh

echo "🚀 SPEED TEST & BENCHMARK SUITE"
echo "================================"
echo "🕐 Started at: $(date)"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test URLs
LOCAL_URL="http://localhost:9876"
DOMAIN_URL="http://proxy-test.fastping.it.com"

echo -e "${BLUE}📊 SYSTEM BASELINE METRICS${NC}"
echo "=========================="

# CPU Info
echo "🔧 CPU Info:"
lscpu | grep "Model name" | head -1
lscpu | grep "CPU(s):" | head -1
lscpu | grep "CPU MHz"

# Memory Info
echo ""
echo "💾 Memory Info:"
free -h

# Disk Info
echo ""
echo "💿 Disk Info:"
df -h / | tail -1

# Network Info
echo ""
echo "🌐 Network Info:"
ip route | grep default
ss -tuln | grep ":80\|:443\|:9876\|:8080"

echo ""
echo -e "${BLUE}🏃 BASIC CONNECTIVITY TESTS${NC}"
echo "=========================="

# Test if services are responding
echo "🔍 Testing local service (localhost:9876)..."
if curl -s --max-time 5 "$LOCAL_URL/health" > /dev/null; then
    echo -e "${GREEN}✅ Local service responding${NC}"
else
    echo -e "${RED}❌ Local service not responding${NC}"
fi

echo ""
echo "🔍 Testing domain service..."
if curl -s --max-time 10 "$DOMAIN_URL/health" > /dev/null; then
    echo -e "${GREEN}✅ Domain service responding${NC}"
else
    echo -e "${RED}❌ Domain service not responding${NC}"
fi

echo ""
echo -e "${BLUE}⚡ SPEED TESTS${NC}"
echo "=============="

# Simple response time test
echo "📊 Response Time Test (10 requests):"
for i in {1..10}; do
    TIME=$(curl -s -w "%{time_total}" -o /dev/null "$LOCAL_URL/ping")
    echo "Request $i: ${TIME}s"
done

echo ""
echo "📈 Average Response Time Test:"
TOTAL_TIME=0
for i in {1..20}; do
    TIME=$(curl -s -w "%{time_total}" -o /dev/null "$LOCAL_URL/ping")
    TOTAL_TIME=$(echo "$TOTAL_TIME + $TIME" | bc -l)
done
AVG_TIME=$(echo "scale=4; $TOTAL_TIME / 20" | bc -l)
echo "Average time over 20 requests: ${AVG_TIME}s"

echo ""
echo -e "${BLUE}🔥 LOAD TESTING (if tools available)${NC}"
echo "=================="

# Check if ab (Apache Bench) is available
if command -v ab &> /dev/null; then
    echo "🚀 Apache Bench Test (100 requests, 10 concurrent):"
    ab -n 100 -c 10 "$LOCAL_URL/ping"
else
    echo "⚠️ Apache Bench not installed. Install with: sudo apt install apache2-utils"
fi

echo ""

# Check if wrk is available
if command -v wrk &> /dev/null; then
    echo "🚀 WRK Load Test (30 seconds, 10 connections):"
    wrk -t4 -c10 -d30s "$LOCAL_URL/ping"
else
    echo "⚠️ WRK not installed. Install with: sudo apt install wrk"
fi

echo ""
echo -e "${BLUE}📡 NETWORK PERFORMANCE${NC}"
echo "==================="

# Ping tests to common servers
echo "🏓 Ping Tests:"
echo "Google DNS (8.8.8.8):"
ping -c 5 8.8.8.8 | tail -1

echo ""
echo "Cloudflare DNS (1.1.1.1):"
ping -c 5 1.1.1.1 | tail -1

# Download speed test (if wget available)
echo ""
echo "📥 Download Speed Test:"
if command -v wget &> /dev/null; then
    echo "Testing download speed with 10MB file..."
    wget --progress=dot --timeout=30 -O /tmp/speedtest.tmp http://speedtest.ftp.otenet.gr/files/test10Mb.db 2>&1 | \
    grep -o '[0-9.]\+[KM]B/s' | tail -1
    rm -f /tmp/speedtest.tmp
else
    echo "⚠️ wget not available for download test"
fi

echo ""
echo -e "${BLUE}🖥️ SYSTEM PERFORMANCE${NC}"
echo "=================="

# CPU stress test (if available)
if command -v stress &> /dev/null; then
    echo "🔥 CPU Stress Test (10 seconds):"
    echo "Before stress test:"
    top -bn1 | grep "Cpu(s)" | head -1
    
    stress --cpu 1 --timeout 10s &
    sleep 5
    echo "During stress test:"
    top -bn1 | grep "Cpu(s)" | head -1
    wait
    
    sleep 2
    echo "After stress test:"
    top -bn1 | grep "Cpu(s)" | head -1
else
    echo "⚠️ Stress test tool not installed. Install with: sudo apt install stress"
fi

echo ""
echo "📊 Current System Load:"
uptime
iostat 1 1 2>/dev/null | tail -4 || echo "iostat not available"

echo ""
echo -e "${BLUE}🐍 PYTHON/FLASK SPECIFIC TESTS${NC}"
echo "============================"

# Test different endpoints
echo "🧪 Testing different endpoints:"

endpoints=("/" "/ping" "/health" "/test")
for endpoint in "${endpoints[@]}"; do
    echo -n "Testing $endpoint: "
    TIME=$(curl -s -w "%{time_total}" -o /dev/null "$LOCAL_URL$endpoint")
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}${TIME}s${NC}"
    else
        echo -e "${RED}Failed${NC}"
    fi
done

echo ""
echo "📊 Concurrent Request Test:"
# Test concurrent requests
for concurrent in 1 5 10; do
    echo "Testing $concurrent concurrent requests:"
    START_TIME=$(date +%s.%N)
    for i in $(seq 1 $concurrent); do
        curl -s "$LOCAL_URL/ping" > /dev/null &
    done
    wait
    END_TIME=$(date +%s.%N)
    DURATION=$(echo "$END_TIME - $START_TIME" | bc -l)
    echo "  $concurrent requests completed in: ${DURATION}s"
done

echo ""
echo -e "${BLUE}💾 MEMORY USAGE TEST${NC}"
echo "=================="

# Monitor memory usage during requests
echo "Memory usage before load:"
free -h | grep "Mem:"

echo ""
echo "Sending 50 requests and monitoring memory..."
for i in {1..50}; do
    curl -s "$LOCAL_URL/ping" > /dev/null
done

echo "Memory usage after load:"
free -h | grep "Mem:"

echo ""
echo -e "${BLUE}📈 GUNICORN PROCESS INFO${NC}"
echo "======================"

# Check Gunicorn processes
echo "🔍 Gunicorn Processes:"
ps aux | grep gunicorn | grep proxy-test

echo ""
echo "📊 Process Resource Usage:"
if pgrep -f "proxy-test" > /dev/null; then
    PID=$(pgrep -f "proxy-test" | head -1)
    echo "Main PID: $PID"
    top -bn1 -p $PID | tail -1
fi

echo ""
echo -e "${BLUE}🎯 BASELINE SUMMARY${NC}"
echo "=================="

echo "📊 System Summary:"
echo "  CPU: $(lscpu | grep "Model name" | cut -d: -f2 | xargs)"
echo "  Cores: $(nproc)"
echo "  Memory: $(free -h | grep Mem | awk '{print $2}')"
echo "  Load: $(uptime | awk -F'load average:' '{print $2}')"

echo ""
echo "⚡ Performance Summary:"
echo "  Average Response Time: ${AVG_TIME}s"
echo "  Service Status: $(systemctl is-active proxy-test)"
echo "  Workers Running: $(pgrep -f proxy-test | wc -l)"

echo ""
echo "🕐 Benchmark completed at: $(date)"
echo ""
echo -e "${GREEN}✅ Baseline established!${NC}"
echo ""
echo "💡 Recommendations:"
echo "   - Save this output as your baseline"
echo "   - Run this test again after changes to compare"
echo "   - Monitor these metrics regularly"
echo "   - Consider installing: apache2-utils, wrk, stress, iostat"