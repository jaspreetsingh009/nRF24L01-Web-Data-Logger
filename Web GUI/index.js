var server = require("./server");
var router = require("./route");
var requestHandlers = require("./requestHandlers");

var debug = false;

var handle = {}
handle["/"] = requestHandlers.sendInterface;
handle["/webui"] = requestHandlers.sendInterface;
handle["/smoothie.js"] = requestHandlers.smoothie;
handle["/jquery.noty.packaged.min.js"] = requestHandlers.noty;
handle["/rx.jpeg"] = requestHandlers.rximage;

handle["/ok.jpg"] = requestHandlers.img1;
handle["/disconnect.jpg"] = requestHandlers.img2;
handle["/error.jpg"] = requestHandlers.img3;

server.start(router.route,handle,debug);
