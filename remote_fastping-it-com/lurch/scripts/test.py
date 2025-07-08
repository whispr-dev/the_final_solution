#!/usr/bin/env python3
"""
Test Flask Application for VPS Deployment
Complete with all production features
"""

from flask import Flask, jsonify, request, render_template_string
from flask_cors import CORS
import os
import logging
import json
import time
from datetime import datetime
import psutil
import requests

app = Flask(__name__)
app.config['SECRET_KEY'] = os.environ.get('SECRET_KEY', 'dev-key-change-in-production')

# Enable CORS
CORS(app)

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Simple HTML template for testing
HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <title>🚀 Flask VPS Test Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
        .container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .status { padding: 15px; margin: 10px 0; border-radius: 5px; }
        .success { background: #d4edda; border: 1px solid #c3e6cb; color: #155724; }
        .info { background: #d1ecf1; border: 1px solid #bee5eb; color: #0c5460; }
        .test-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin: 20px 0; }
        .test-card { background: #f8f9fa; padding: 20px; border-radius: 8px; border-left: 4px solid #007bff; }
        button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; margin: 5px; }
        button:hover { background: #0056b3; }
        pre { background: #f1f1f1; padding: 15px; border-radius: 5px; overflow-x: auto; }
        .metric { display: inline-block; margin: 10px; padding: 10px; background: #e9ecef; border-radius: 5px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 Flask VPS Production Server</h1>
        <div class="status success">
            ✅ Server is running successfully!
        </div>
        
        <div class="status info">
            📊 Server started at: {{ start_time }}<br>
            🌐 Environment: {{ environment }}<br>
            🐍 Python: {{ python_version }}<br>
            ⚡ Flask: {{ flask_version }}
        </div>

        <div class="test-grid">
            <div class="test-card">
                <h3>🧪 API Tests</h3>
                <button onclick="testAPI('/api/health')">Health Check</button>
                <button onclick="testAPI('/api/system')">System Info</button>
                <button onclick="testAPI('/api/test-json')">JSON Test</button>
                <button onclick="testAPI('/api/test-requests')">HTTP Test</button>
                <pre id="api-results">Click buttons to test APIs...</pre>
            </div>
            
            <div class="test-card">
                <h3>📈 System Metrics</h3>
                <div class="metric">CPU: {{ cpu_percent }}%</div>
                <div class="metric">Memory: {{ memory_percent }}%</div>
                <div class="metric">Disk: {{ disk_percent }}%</div>
                <button onclick="location.reload()">Refresh Metrics</button>
            </div>
            
            <div class="test-card">
                <h3>🔧 Dependencies</h3>
                <div style="max-height: 200px; overflow-y: scroll;">
                    {% for dep in dependencies %}
                        <div style="padding: 2px;">{{ dep }}</div>
                    {% endfor %}
                </div>
            </div>
        </div>

        <div class="test-card">
            <h3>🌐 Network Test</h3>
            <button onclick="testExternal()">Test External API</button>
            <pre id="network-results">Click to test external connectivity...</pre>
        </div>
    </div>

    <script>
        async function testAPI(endpoint) {
            const results = document.getElementById('api-results');
            results.textContent = 'Testing ' + endpoint + '...';
            
            try {
                const response = await fetch(endpoint);
                const data = await response.json();
                results.textContent = JSON.stringify(data, null, 2);
            } catch (error) {
                results.textContent = 'Error: ' + error.message;
            }
        }
        
        async function testExternal() {
            const results = document.getElementById('network-results');
            results.textContent = 'Testing external connectivity...';
            
            try {
                const response = await fetch('/api/test-external');
                const data = await response.json();
                results.textContent = JSON.stringify(data, null, 2);
            } catch (error) {
                results.textContent = 'Error: ' + error.message;
            }
        }
    </script>
</body>
</html>
"""

@app.route('/')
def index():
    """Main test page"""
    try:
        import sys
        import flask
        import pkg_resources
        
        # Get system metrics
        cpu_percent = psutil.cpu_percent(interval=1)
        memory = psutil.virtual_memory()
        disk = psutil.disk_usage('/')
        
        # Get installed packages
        dependencies = []
        try:
            installed_packages = [str(d) for d in pkg_resources.working_set]
            dependencies = sorted(installed_packages)[:20]  # Show first 20
        except:
            dependencies = ["Could not load package list"]
        
        return render_template_string(HTML_TEMPLATE,
            start_time=datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            environment=os.environ.get('FLASK_ENV', 'development'),
            python_version=f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}",
            flask_version=flask.__version__,
            cpu_percent=round(cpu_percent, 1),
            memory_percent=round(memory.percent, 1),
            disk_percent=round(disk.percent, 1),
            dependencies=dependencies
        )
    except Exception as e:
        logger.error(f"Error in index route: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/health')
def health():
    """Health check endpoint"""
    return "healthy\n", 200

@app.route('/api/health')
def api_health():
    """JSON health check"""
    return jsonify({
        "status": "healthy",
        "timestamp": datetime.now().isoformat(),
        "uptime": time.time(),
        "version": "1.0.0"
    })

@app.route('/api/system')
def api_system():
    """System information API"""
    try:
        return jsonify({
            "cpu_percent": psutil.cpu_percent(interval=1),
            "memory": {
                "total": psutil.virtual_memory().total,
                "available": psutil.virtual_memory().available,
                "percent": psutil.virtual_memory().percent
            },
            "disk": {
                "total": psutil.disk_usage('/').total,
                "free": psutil.disk_usage('/').free,
                "percent": psutil.disk_usage('/').percent
            },
            "load_avg": os.getloadavg() if hasattr(os, 'getloadavg') else [0, 0, 0],
            "timestamp": datetime.now().isoformat()
        })
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/test-json', methods=['GET', 'POST'])
def test_json():
    """Test JSON processing"""
    if request.method == 'POST':
        try:
            data = request.get_json()
            return jsonify({
                "message": "JSON received successfully",
                "received_data": data,
                "timestamp": datetime.now().isoformat()
            })
        except Exception as e:
            return jsonify({"error": f"JSON parsing error: {e}"}), 400
    else:
        return jsonify({
            "message": "JSON endpoint working",
            "methods": ["GET", "POST"],
            "test_data": {
                "string": "Hello World",
                "number": 42,
                "boolean": True,
                "array": [1, 2, 3],
                "object": {"nested": "value"}
            }
        })

@app.route('/api/test-requests')
def test_requests():
    """Test requests library"""
    try:
        # Test internal request
        import requests
        response = requests.get('http://httpbin.org/json', timeout=5)
        
        return jsonify({
            "requests_working": True,
            "test_url": "http://httpbin.org/json",
            "status_code": response.status_code,
            "response_sample": response.json(),
            "timestamp": datetime.now().isoformat()
        })
    except Exception as e:
        return jsonify({
            "requests_working": False,
            "error": str(e),
            "timestamp": datetime.now().isoformat()
        }), 500

@app.route('/api/test-external')
def test_external():
    """Test external API connectivity"""
    try:
        # Test multiple external services
        tests = []
        
        # Test 1: httpbin.org
        try:
            resp = requests.get('http://httpbin.org/ip', timeout=5)
            tests.append({
                "service": "httpbin.org",
                "status": "success",
                "response": resp.json()
            })
        except Exception as e:
            tests.append({
                "service": "httpbin.org", 
                "status": "failed",
                "error": str(e)
            })
        
        # Test 2: JSONPlaceholder
        try:
            resp = requests.get('https://jsonplaceholder.typicode.com/posts/1', timeout=5)
            tests.append({
                "service": "jsonplaceholder",
                "status": "success",
                "response": resp.json()
            })
        except Exception as e:
            tests.append({
                "service": "jsonplaceholder",
                "status": "failed", 
                "error": str(e)
            })
        
        return jsonify({
            "external_connectivity": True,
            "tests": tests,
            "timestamp": datetime.now().isoformat()
        })
        
    except Exception as e:
        return jsonify({
            "external_connectivity": False,
            "error": str(e),
            "timestamp": datetime.now().isoformat()
        }), 500

@app.errorhandler(404)
def not_found(error):
    return jsonify({"error": "Not found"}), 404

@app.errorhandler(500)
def internal_error(error):
    return jsonify({"error": "Internal server error"}), 500

if __name__ == '__main__':
    port = int(os.environ.get('PORT', 5000))
    debug = os.environ.get('FLASK_ENV') == 'development'
    
    logger.info(f"Starting Flask app on port {port}, debug={debug}")
    app.run(host='0.0.0.0', port=port, debug=debug)