#!/usr/bin/nodejs

var http = require("http");
var fs   = require("fs");
var url  = require("url");

function app(req, res) {
	res.writeHead(	200
				 ,	{'Content-Type': 'text/plain'}
				 );
	res.end('Dette er Node serveren. Hallo!', 'text');
}

http.createServer(app).listen(80, function(err) {
	process.setgid(666);
	process.setuid(666);
});
