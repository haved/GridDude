from socketserver import ThreadingTCPServer
from http.server import BaseHTTPRequestHandler
from os import getenv

HOST = ""
PORT = int(getenv("PORT", "80"))

class GridDudeRequestHandler(BaseHTTPRequestHandler):
    "Handles requests for both the website and the updates"
    def get_content_string(self):
        "Gets the entire website as a string"
        return """
        <!DOCTYPE html>
        <html>
        <head>
        <title>Grid dude</title>
        </head>
        <body>
        <h1>We are live!</h1>
        {}
        </body>
        </html>
        """.format(self.client_address)

    def do_GET(self):
        "handles a GET request to the server"
        self.send_response(200)
        self.end_headers()

        self.wfile.write(self.get_content_string().encode('utf-8'))

with ThreadingTCPServer((HOST, PORT), GridDudeRequestHandler) as httpd:
    print("Serving Grid Dude on port", PORT)
    httpd.serve_forever()
