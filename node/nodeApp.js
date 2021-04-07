const net = require('net')
const client = new net.Socket()
const WebSocket = require('ws')
const wss = new WebSocket.Server({ port: 1237 })

let buffer = ""

client.connect(1236, '127.0.0.1', function() {
	console.log('Connected')
})

client.on('data', function(data) {
	buffer = data.toString()
	
	console.log(buffer)
})


client.on('close', function() {
	console.log('Connection closed')
})


wss.on('connection', ws => {
  	ws.on('message', message => {ws.send(buffer)})
  	ws.send('0;0;0;0')
})


setInterval(()=>client.write("h"), 50)