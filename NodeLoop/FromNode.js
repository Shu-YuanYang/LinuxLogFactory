
const { exec } = require("child_process");
console.log("Hello?");
exec("./NodeLoop", (error, stdout, stderr) => console.log(stdout));
console.log("Hello Again?");

// Code copied from: https://stackoverflow.com/questions/61755518/is-it-possible-to-have-a-node-js-websocket-client
//#!/usr/bin/env node
/*
var WebSocketClient = require('websocket').client;

var client = new WebSocketClient();

client.on('connectFailed', function(error) {
    	console.log('Connect Error: ' + error.toString());
});

client.on('connect', function(connection) {
	console.log('WebSocket Client Connected');
    
 	connection.on('error', function(error) {
        	console.log("Connection Error: " + error.toString());
   	});
   
	connection.on('close', function() {
        	console.log('echo-protocol Connection Closed');
    	});
    
	connection.on('message', function(message) {
		console.log(message);
    	});

    	function sendNumber() {
        	if (connection.connected) {
			//console.log("Connected?");
            		var number = Math.round(Math.random() * 0xFFFFFF);
            		connection.sendUTF("See message: " + number.toString());
            		setTimeout(sendNumber, 1000);
        	}
    	}
    	sendNumber();
});

client.connect('ws://172.17.0.4:6969/', 'echo-protocol');
*/

