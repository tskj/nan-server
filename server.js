#!/usr/bin/nodejs

var http = require("http");
var fs   = require("fs");
var url  = require("url");

var chroot = require('/usr/lib/node_modules/chroot');

function app(req, res) {

	var action = url.parse(req.url, true).pathname;

	if (nFirstEqual('/bin/', action, 5) || action === '/bin') {
		res.writeHead(404, {'Content-Type': 'text/html'});
		fs.readFile('lib/not-found.html', 'utf-8', function(err, data) {
			if (err) throw err;
			res.write(data);
			res.end();
		});
	} else if (nFirstEqual('/lib/', action, 5) || action === '/lib') {
		res.writeHead(404, {'Content-Type': 'text/html'});
		fs.readFile('lib/not-found.html', 'utf-8', function(err, data) {
			if (err) throw err;
			res.write(data);
			res.end();
		});
	} else {
		res.writeHead(	200
					 ,	{'Content-Type': 'text/plain; charset=utf-8'}
					 );
		res.end('Dette er Node serveren. Hallå!\n\n', 'utf-8');
	}
}

function nFirstEqual(str1, str2, n) {
	if (str1.length < n || str2.length < n) return false;

	var i = 0;
	while (i < n) {
		if (str1[i] === str2[i]) {
			i++;
			continue;
		}
		return false;
	}
	return true;
}

http.createServer(app).listen(80, function(err) {
	try {
		chroot('/www', 'www', 666);
	} catch(err) {
		console.error('Chaning root or user failed.', err);
		process.exit(1);
	}
});
