#!/usr/bin/env python

from socketserver import ThreadingTCPServer
from http.server import BaseHTTPRequestHandler
from os import getenv
import json
import threading

HOST = ""
PORT = int(getenv("PORT", "80"))

MOVE_CODES = {
    'U': [0, -1],
    'D': [0, 1],
    'L': [-1, 0],
    'R': [1, 0],
}

GRID_LOCK = threading.Lock()
GRID = {}

def getGridIndex(x, y):
    width = GRID['width']
    return x+y*width

def gridWithDudeIndex():
    dude = GRID['dude']
    GRID['dudeIndex'] = getGridIndex(*dude)
    return GRID

def makeDefaultGrid():
    size = int(getenv("DEFAULT_GRID_SIZE", "100"))
    GRID['width'], GRID['height'] = (size, size)
    GRID['data'] = [0 for _ in range(size*size)]
    GRID['dude']=[size//2, size//2]

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
        GRID_LOCK.acquire()
        try:
            if 'data' not in GRID:
                makeDefaultGrid()
            self.wfile.write(json.dumps(gridWithDudeIndex()).encode('utf-8'))
        finally:
            GRID_LOCK.release()

    def update_grid(self):
        length = int(self.headers['Content-length'])
        update = self.rfile.read(length).decode('utf-8')

        self.send_header('Content-type', 'text/plain')
        if False in [c in MOVE_CODES for c in update]:
            self.send_response(400)
            self.end_headers()
            self.wfile.write("Bad data\r\n".encode('utf-8'))
            return

        self.send_response(200, 'OK')
        self.end_headers()
        self.wfile.write("success\r\n".encode('utf-8'))

        GRID_LOCK.acquire()
        if 'data' not in GRID:
            makeDefaultGrid()
        try:
            for code in update:
                move = MOVE_CODES[code]
                new_dude_x = GRID['dude'][0] + move[0]
                new_dude_y = GRID['dude'][1] + move[1]
                new_dude_x %= GRID['width']
                new_dude_y %= GRID['height']
                index = getGridIndex(new_dude_x, new_dude_y)
                GRID['dude'] = [new_dude_x, new_dude_y]
                GRID['data'][index] += 1
        finally:
            GRID_LOCK.release()

    def do_POST(self):
        self.protocol_version='HTTP/1.1'
        if self.path == "/update_grid":
            self.update_grid()
        else:
            self.output_404()

with ThreadingTCPServer((HOST, PORT), GridDudeRequestHandler) as httpd:
    print("Serving Grid Dude on port", PORT)
    httpd.serve_forever()
