import json

from flask import Flask
from flask import request

app = Flask(__name__)

@app.route("/", methods=['GET', 'POST', 'DELETE'])
def root_get():
	
	if request.method == 'GET':
		print(dir(request))
		#print(request.get_data())
		return "Hello World!"
		
	if request.method == 'POST':
		print(dir(request))
		print(request.get_data())
		return json.dumps({"success": True, "recd": str(request.data)}), 201
	
if __name__ == "__main__":
	#app.run(host='localhost', port=4443)
	app.run(host='localhost', port=4443, ssl_context=('cert2048.pem', 'key2048.pem'))
