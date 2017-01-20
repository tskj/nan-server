#!/usr/bin/nodejs

var http = require("http");
var fs   = require("fs");
var url  = require("url");

var app = function(req, res) {};

http.createServer(app).listen(80, function(err) {
	process.setgid(666);
	process.setuid(666);
});
