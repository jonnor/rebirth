var v4l2camera = require("v4l2camera");

var cam = new v4l2camera.Camera("/dev/video1");
	console.log(cam.formats);
//console.log(Object.keys(cam.controls));

var format = cam.formats.filter(function (f) { return f.formatName == 'RGB3' && f.width == 640 && f.height == 480 });
format = format[0];

if (!format) {
  throw new Error("Could not find format");
}
console.log(format);

cam.configSet(format);
var formatName = cam.configGet().formatName;

cam.start();
setTimeout(function() {
cam.capture(function (success) {
  var frame = cam.frameRaw();
  console.log(typeof(frame), frame.length);
  require("fs").createWriteStream("result.rgb.data").end(Buffer(frame));
  cam.stop();
});
}, 500);
