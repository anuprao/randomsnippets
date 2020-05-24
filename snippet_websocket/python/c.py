from lomond import WebSocket
websocket = WebSocket('wss://127.0.0.1:6261')

for event in websocket:
    if event.name == "ready":
        print(event.json)
        websocket.send_text("Hello World")
    elif event.name == "text":
        print(event.json)
