const PORT = 7777;

var fs = require('fs');
var ip = require('ip');
var url = require('url');
var http = require('http');
var WebSocket = require('ws');


function requestHandler(request, response) {
    fs.readFile('./index.html', function (error, content) {
        response.writeHead(200, {
            'Content-Type': 'text/html'
        });
        response.end(content);
    });
};

// create http server
var server = http.createServer(requestHandler);

var ws = new WebSocket.Server({
    server
});

var clients = [];

function broadcast(socket, data) {
    console.log(clients.length);

    for (var i = 0; i < clients.length; i++) {
        if (clients[i] != socket) {
            clients[i].send(data);
        }
    }
};

ws.on('connection', function (socket, req) {
    clients.push(socket);
    socket.on('message', function (message) {
        console.log('received: %s', message);
        broadcast(socket, message);
    });

    socket.on('close', function () {
        var index = clients.indexOf(socket);
        clients.splice(index, 1);
        console.log('disconnected');
    });
});

server.listen(PORT);
console.log('Server listening on http://' + ip.address() + ":" + PORT);
