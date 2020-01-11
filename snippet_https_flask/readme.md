Steps to generate test certificate and key
------------------------------------------


PEM pass phrase: test123
Country: IN

openssl req -x509 -newkey rsa:2048 -keyout key2048.pem -out cert2048.pem -days 365

openssl req -x509 -newkey rsa:1024 -keyout key1024.pem -out cert1024.pem -days 365

openssl req -x509 -newkey rsa:512 -keyout key512.pem -out cert512.pem -days 365

openssl req -x509 -sha256 -keyout key256.pem -out cert256.pem -days 365

