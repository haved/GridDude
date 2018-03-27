#!/usr/bin/env python

from socketserver import ThreadingTCPServer
from http.server import BaseHTTPRequestHandler
from os import getenv
import json

HOST = ""
PORT = int(getenv("PORT", "80"))

GRID = {}
def makeDefaultGrid():
    size = int(getenv("DEFAULT_GRID_SIZE", "100"))
    GRID['width'], GRID['height'] = (size, size)
    GRID['data'] = [i for i in range(size*size)]
    GRID['dude']=[size//2,size//2]

def read_file(file_name):
    with open("page/"+file_name, "r") as file_handle:
        return file_handle.read()

class GridDudeRequestHandler(BaseHTTPRequestHandler):
    "Handles requests for both the website and the updates"

    def output_page(self, content_name):
        self.send_response(200, 'OK')
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(read_file("template.html").format(read_file(content_name)).encode('utf-8'))

    def output_resource(self, name, content_type):
        self.send_response(200, 'OK')
        self.send_header('Content-type', content_type)
        self.end_headers()
        self.wfile.write(read_file(name).encode('utf-8'))

    def output_404(self):
        self.send_response(404)
        self.end_headers()
        self.wfile.write("404 page not found".encode('utf-8'))

    def do_redirect(self, target):
        self.send_response(301)
        self.send_header('Location', target)
        self.end_headers()

    def do_GET(self):
        self.protocol_version='HTTP/1.1'
        if self.path == "/" or self.path == "/index.html":
            self.output_page("grid.html")
        elif self.path == "/about.html":
            self.output_page("about.html")
        elif self.path == "/gridScript.js":
            self.output_resource("gridScript.js", 'application/javascript')
        elif self.path == "/style.css":
            self.output_resource("style.css", 'text/css')
        elif self.path == "/grid.json":
            self.output_gridData_json()
        elif self.path == "/index" or self.path == "/about":
            self.do_redirect(self.path+".html")
        else:
            self.output_404()

    def output_gridData_json(self):
        self.send_response(200, 'OK')
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        if 'data' not in GRID:
            makeDefaultGrid()
        self.wfile.write(json.dumps(GRID).encode('utf-8'))

    def update_grid(self):
        self.send_response(200, 'OK')
        self.send_header('Content-type', '')


    def do_POST(self):
        self.protocol_version='HTTP/1.1'
        if self.path == "/update_grid":
            self.update_grid()
        else:
            self.output_404()

with ThreadingTCPServer((HOST, PORT), GridDudeRequestHandler) as httpd:
    print("Serving Grid Dude on port", PORT)
    httpd.serve_forever()
