console.log("Started")

var socket = new WebSocket("ws://localhost:1237");

socket.onopen = () => alert("Соединение установлено.")

socket.onmessage = event => console.log("Получены данные " + event.data)

socket.onclose = () => alert("Connection lost")

