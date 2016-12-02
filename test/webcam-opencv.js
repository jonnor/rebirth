var cv = require('opencv');

var number = 0;

try {
  var camera = new cv.VideoCapture(0);
  //var window = new cv.NamedWindow('Video', 0)
  
  console.log("Running capture");
  setInterval(function() {
    camera.read(function(err, im) {
      if (err) throw err;
      //console.log(im.size())
      var size = im.size();
      if (size[0] > 0 && size[1] > 0){
        im.save("image"+number+".jpg");
        number+=1;
      }
      //window.blockingWaitKey(0, 50);
    });
  }, 20);
  
} catch (e){
  console.log("Couldn't start camera:", e)
}
