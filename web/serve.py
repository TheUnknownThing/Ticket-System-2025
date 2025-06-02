#!/usr/bin/env python3
"""
Simple HTTP server to serve the web frontend.
This solves CORS issues by serving the frontend from the same protocol.
"""
import http.server
import socketserver
import os
import sys
import urllib.request
import urllib.parse

PORT = 10808
API_BASE_URL = "http://localhost:9988"

class CORSRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        # Set the directory to serve files from
        web_dir = os.path.dirname(os.path.abspath(__file__))
        super().__init__(*args, directory=web_dir, **kwargs)
    
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type, Authorization')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()
    
    def do_GET(self):
        # Proxy API requests
        if self.path.startswith('/api/'):
            self.proxy_request()
        else:
            super().do_GET()
    
    def do_POST(self):
        if self.path.startswith('/api/'):
            self.proxy_request()
        else:
            self.send_response(405)
            self.end_headers()
    
    def proxy_request(self):
        try:
            # Remove /api prefix and forward to backend
            api_path = self.path[4:]  # Remove '/api'
            target_url = f"{API_BASE_URL}{api_path}"
            
            # Forward the request
            req = urllib.request.Request(target_url)
            
            # Copy headers if needed
            if hasattr(self, 'headers'):
                for key, value in self.headers.items():
                    if key.lower() not in ['host', 'connection']:
                        req.add_header(key, value)
            
            # Handle POST data
            if self.command == 'POST':
                content_length = int(self.headers.get('Content-Length', 0))
                post_data = self.rfile.read(content_length)
                req.data = post_data
            
            response = urllib.request.urlopen(req)
            
            # Send response
            self.send_response(response.status)
            for key, value in response.headers.items():
                if key.lower() not in ['connection', 'transfer-encoding']:
                    self.send_header(key, value)
            self.end_headers()
            
            # Send body
            self.wfile.write(response.read())
            
        except Exception as e:
            self.send_response(500)
            self.end_headers()
            self.wfile.write(f"Proxy error: {str(e)}".encode())

if __name__ == "__main__":
    try:
        web_dir = os.path.dirname(os.path.abspath(__file__))
        print(f"Serving files from: {web_dir}")
        
        with socketserver.TCPServer(("0.0.0.0", PORT), CORSRequestHandler) as httpd:
            print(f"Server running at http://0.0.0.0:{PORT}")
            print("Press Ctrl+C to stop the server")
            httpd.serve_forever()

    except KeyboardInterrupt:
        print("\nServer stopped.")
        sys.exit(0)