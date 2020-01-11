# Refer article at : https://blog.anvileight.com/posts/simple-python-http-server/
#

from http.server import HTTPServer, BaseHTTPRequestHandler
import ssl
from io import BytesIO

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
	
	def do_GET(self):
		self.send_response(200)
		self.end_headers()
		self.wfile.write(b'Hello, world!')
		
	def do_POST(self):
		content_length = int(self.headers['Content-Length'])
		body = self.rfile.read(content_length)
		self.send_response(200)
		self.end_headers()
		response = BytesIO()
		response.write(b'This is POST request. ')
		response.write(b'Received: ')
		response.write(body)
		self.wfile.write(response.getvalue())
        
httpd = HTTPServer(('localhost', 4443), SimpleHTTPRequestHandler)

httpd.socket = ssl.wrap_socket (httpd.socket, 
	keyfile='key2048.pem', 
	certfile='cert2048.pem', 
	server_side=True)

httpd.serve_forever()
