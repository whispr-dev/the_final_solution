# 1. Make the speed test script executable
chmod +x speed_tests.sh

# 2. Run the comprehensive bash test
./speed_tests.sh > baseline_results.txt

# 3. Run the detailed Python test
python3 python_speed_test.py

# 4. Quick manual tests
curl -w "Time: %{time_total}s\n" -o /dev/null -s localhost:9876/ping
curl -w "Time: %{time_total}s\n" -o /dev/null -s proxy-test.fastping.it.com/ping


 What This Will Test:

Response Times - How fast your endpoints respond
Concurrent Performance - How many simultaneous requests it can handle
Load Testing - Sustained performance under stress
System Metrics - CPU, memory, disk usage
Network Performance - Ping times, download speeds
Gunicorn Workers - Process health and resource usage

💾 Results:

Bash script saves to baseline_results.txt
Python script saves to flask_performance_baseline.json
You'll have solid numbers to compare against later!

Run these and you'll know exactly how fast your setup is, fren! 🎯⚡