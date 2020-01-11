from flask import Flask
app = Flask(__name__)

@app.route("/")
def hello():
	return "Hello World!"

if __name__ == "__main__":
	app.run(host='localhost', port=4443, ssl_context=('cert2048.pem', 'key2048.pem'))
