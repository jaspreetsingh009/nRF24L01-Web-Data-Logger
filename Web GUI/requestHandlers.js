var fs      =  require('fs');
var server  =  require('./server');

function sendInterface(response) {
  console.log("Request handler 'webui' was called.");
  response.writeHead(200, {"Content-Type": "text/html"});
  var html = fs.readFileSync(__dirname + "/webpage/main.html")
  response.end(html);
}

function smoothie(response) {
  console.log("Request handler 'smoothie' was called.");
  response.writeHead(200, {"Content-Type": "text/html"});
  var html = fs.readFileSync(__dirname + "/scripts/smoothie.js")
  response.end(html);
}

function socketio(response) {
  console.log("Request handler 'socketio' was called.");
  response.writeHead(200, {"Content-Type": "text/html"});
  var html = fs.readFileSync(__dirname + "/scripts/socket-io.js")
  response.end(html);
}

function noty(response) {
  console.log("Request handler 'jquery.noty.packaged.min.js' was called.");
  response.writeHead(200, {"Content-Type": "text/html"});
  var html = fs.readFileSync(__dirname + "/scripts/jquery.noty.packaged.min.js")
  response.end(html);
}

function rximage(response) {
  console.log("Request handler 'rx.jpeg' was called.");
  response.writeHead(200, {"Content-Type": "text/html"});
  var html = fs.readFileSync(__dirname + "/images/rx.jpeg")
  response.end(html);
}

function img1(response) {
  console.log("Request handler 'ok.jpg' was called.");
  response.writeHead(200, {"Content-Type": "text/html"});
  var html = fs.readFileSync(__dirname + "/images/ok.jpg")
  response.end(html);
}

function img2(response) {
  console.log("Request handler 'disconnect.jpeg' was called.");
  response.writeHead(200, {"Content-Type": "text/html"});
  var html = fs.readFileSync(__dirname + "/images/disconnect.jpg")
  response.end(html);
}

function img3(response) {
  console.log("Request handler 'error.jpeg' was called.");
  response.writeHead(200, {"Content-Type": "text/html"});
  var html = fs.readFileSync(__dirname + "/images/error.jpg")
  response.end(html);
}

exports.sendInterface  =   sendInterface;
exports.smoothie       =   smoothie;
exports.socketio       =   socketio;
exports.noty	       =   noty;
exports.rximage	       =   rximage;
exports.img1           =   img1;
exports.img2           =   img2;
exports.img3           =   img3;
