#!/usr/bin/env python3
"""
Python Flask Performance Tester
Detailed testing of your Flask app performance
"""

import requests
import time
import statistics
import concurrent.futures
import json
from datetime import datetime

class FlaskSpeedTester:
    def __init__(self, base_url="http://localhost:9876"):
        self.base_url = base_url
        self.session = requests.Session()
        
    def test_endpoint(self, endpoint, num_requests=10):
        """Test a single endpoint multiple times"""
        print(f"🧪 Testing {endpoint} ({num_requests} requests)...")
        times = []
        errors = 0
        
        for i in range(num_requests):
            try:
                start_time = time.time()
                response = self.session.get(f"{self.base_url}{endpoint}", timeout=10)
                end_time = time.time()
                
                if response.status_code == 200:
                    times.append(end_time - start_time)
                else:
                    errors += 1
                    print(f"   ❌ Request {i+1}: HTTP {response.status_code}")
                    
            except Exception as e:
                errors += 1
                print(f"   ❌ Request {i+1}: {str(e)}")
        
        if times:
            avg_time = statistics.mean(times)
            min_time = min(times)
            max_time = max(times)
            median_time = statistics.median(times)
            
            print(f"   📊 Results:")
            print(f"      Average: {avg_time:.4f}s")
            print(f"      Median:  {median_time:.4f}s")
            print(f"      Min:     {min_time:.4f}s")
            print(f"      Max:     {max_time:.4f}s")
            print(f"      Errors:  {errors}/{num_requests}")
            print(f"      Success: {(num_requests-errors)/num_requests*100:.1f}%")
            
            return {
                'endpoint': endpoint,
                'avg_time': avg_time,
                'median_time': median_time,
                'min_time': min_time,
                'max_time': max_time,
                'errors': errors,
                'success_rate': (num_requests-errors)/num_requests*100
            }
        else:
            print(f"   ❌ All requests failed!")
            return None
    
    def concurrent_test(self, endpoint, num_requests=20, num_workers=5):
        """Test concurrent requests"""
        print(f"🚀 Concurrent test: {endpoint} ({num_requests} requests, {num_workers} workers)...")
        
        def make_request():
            try:
                start_time = time.time()
                response = requests.get(f"{self.base_url}{endpoint}", timeout=10)
                end_time = time.time()
                return end_time - start_time, response.status_code
            except Exception as e:
                return None, str(e)
        
        start_total = time.time()
        
        with concurrent.futures.ThreadPoolExecutor(max_workers=num_workers) as executor:
            futures = [executor.submit(make_request) for _ in range(num_requests)]
            results = [future.result() for future in concurrent.futures.as_completed(futures)]
        
        end_total = time.time()
        total_time = end_total - start_total
        
        times = [r[0] for r in results if r[0] is not None]
        errors = [r for r in results if r[0] is None]
        
        if times:
            print(f"   📊 Concurrent Results:")
            print(f"      Total time:     {total_time:.4f}s")
            print(f"      Requests/sec:   {num_requests/total_time:.2f}")
            print(f"      Avg response:   {statistics.mean(times):.4f}s")
            print(f"      Errors:         {len(errors)}/{num_requests}")
            
            return {
                'total_time': total_time,
                'requests_per_second': num_requests/total_time,
                'avg_response_time': statistics.mean(times),
                'error_count': len(errors)
            }
        else:
            print(f"   ❌ All concurrent requests failed!")
            return None
    
    def load_test(self, endpoint, duration=30):
        """Load test for specified duration"""
        print(f"🔥 Load test: {endpoint} for {duration} seconds...")
        
        start_time = time.time()
        end_time = start_time + duration
        request_count = 0
        error_count = 0
        response_times = []
        
        while time.time() < end_time:
            try:
                req_start = time.time()
                response = self.session.get(f"{self.base_url}{endpoint}", timeout=5)
                req_end = time.time()
                
                request_count += 1
                response_times.append(req_end - req_start)
                
                if response.status_code != 200:
                    error_count += 1
                    
            except Exception:
                error_count += 1
                request_count += 1
        
        actual_duration = time.time() - start_time
        
        if response_times:
            print(f"   📊 Load Test Results:")
            print(f"      Duration:       {actual_duration:.2f}s")
            print(f"      Total requests: {request_count}")
            print(f"      Requests/sec:   {request_count/actual_duration:.2f}")
            print(f"      Avg response:   {statistics.mean(response_times):.4f}s")
            print(f"      Error rate:     {error_count/request_count*100:.2f}%")
            
            return {
                'duration': actual_duration,
                'total_requests': request_count,
                'requests_per_second': request_count/actual_duration,
                'avg_response_time': statistics.mean(response_times),
                'error_rate': error_count/request_count*100
            }
        else:
            print(f"   ❌ Load test failed!")
            return None
    
    def run_full_test(self):
        """Run complete test suite"""
        print("🚀 FLASK PERFORMANCE TEST SUITE")
        print("=" * 40)
        print(f"🕐 Started at: {datetime.now()}")
        print(f"🎯 Testing: {self.base_url}")
        print()
        
        results = {}
        
        # Test individual endpoints
        endpoints = ['/', '/ping', '/health', '/test']
        for endpoint in endpoints:
            results[endpoint] = self.test_endpoint(endpoint, 20)
            print()
        
        # Concurrent tests
        print("🚀 CONCURRENT TESTS")
        print("=" * 20)
        results['concurrent'] = self.concurrent_test('/ping', 50, 10)
        print()
        
        # Load test
        print("🔥 LOAD TEST")
        print("=" * 12)
        results['load_test'] = self.load_test('/ping', 15)
        print()
        
        # Summary
        print("📊 PERFORMANCE SUMMARY")
        print("=" * 22)
        
        if results.get('/ping'):
            ping_result = results['/ping']
            print(f"✅ Ping endpoint average: {ping_result['avg_time']:.4f}s")
            print(f"✅ Ping success rate: {ping_result['success_rate']:.1f}%")
        
        if results.get('concurrent'):
            conc_result = results['concurrent']
            print(f"🚀 Concurrent RPS: {conc_result['requests_per_second']:.2f}")
        
        if results.get('load_test'):
            load_result = results['load_test']
            print(f"🔥 Load test RPS: {load_result['requests_per_second']:.2f}")
            print(f"🔥 Load test errors: {load_result['error_rate']:.2f}%")
        
        print()
        print("💾 Saving results to flask_performance_baseline.json...")
        
        # Save results
        baseline_data = {
            'timestamp': datetime.now().isoformat(),
            'base_url': self.base_url,
            'results': results
        }
        
        with open('flask_performance_baseline.json', 'w') as f:
            json.dump(baseline_data, f, indent=2)
        
        print("✅ Baseline saved!")
        print()
        print("🎯 RECOMMENDATIONS:")
        
        if results.get('/ping') and results['/ping']['avg_time'] > 0.1:
            print("   ⚠️ Response time > 100ms - consider optimization")
        
        if results.get('concurrent') and results['concurrent']['requests_per_second'] < 50:
            print("   ⚠️ Low concurrent RPS - consider more workers")
            
        if results.get('load_test') and results['load_test']['error_rate'] > 5:
            print("   ⚠️ High error rate under load - investigate stability")
        
        print("   ✅ Run this test regularly to track performance")
        print("   ✅ Compare results after making changes")

if __name__ == "__main__":
    tester = FlaskSpeedTester()
    tester.run_full_test()