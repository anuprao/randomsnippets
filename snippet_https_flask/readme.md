Steps to generate test certificate and key
------------------------------------------

Refer articles at : 
	https://blog.anvileight.com/posts/simple-python-http-server/
	https://blog.miguelgrinberg.com/post/running-your-flask-application-over-https

Key in following parameters when generating the key and the certificate
	PEM pass phrase: test123
	Country: IN
	Common Name: localhost

openssl req -x509 -newkey rsa:2048 -keyout key2048.pem -out cert2048.pem -days 365

openssl req -x509 -newkey rsa:1024 -keyout key1024.pem -out cert1024.pem -days 365

openssl req -x509 -newkey rsa:512 -keyout key512.pem -out cert512.pem -days 365

openssl req -x509 -sha256 -keyout key256.pem -out cert256.pem -days 365

$ curl --cacert cert2048.pem  'https://localhost:4443/'
curl: (56) OpenSSL SSL_read: Success
Hello, world!

$ curl --cacert cert2048.pem  -X POST https://localhost:4443 -d 'Test data'
curl: (56) OpenSSL SSL_read: Success
This is POST request. Received: Test data

