#!/usr/bin/env python3
"""
Fixed proxy-test-app.py
"""

from flask import Flask, jsonify, request
import datetime

# CREATE THE APP FIRST! (This was missing)
app = Flask(__name__)

# Now we can use @app.route decorators
@app.route("/ping", methods=["GET"])
def ping():
    return jsonify({
        "status": "pong",
        "timestamp": datetime.datetime.now().isoformat(),
        "message": "Proxy test server is running!"
    })

@app.route("/health", methods=["GET"])
def health():
    return jsonify({
        "status": "healthy",
        "service": "proxy-test",
        "timestamp": datetime.datetime.now().isoformat()
    })

@app.route("/", methods=["GET"])
def index():
    return jsonify({
        "message": "Proxy Test Server",
        "endpoints": ["/ping", "/health", "/test"],
        "status": "running"
    })

@app.route("/test", methods=["GET", "POST"])
def test():
    if request.method == "POST":
        data = request.get_json() or {}
        return jsonify({
            "method": "POST",
            "received_data": data,
            "timestamp": datetime.datetime.now().isoformat()
        })
    else:
        return jsonify({
            "method": "GET",
            "message": "Test endpoint working",
            "timestamp": datetime.datetime.now().isoformat()
        })

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)