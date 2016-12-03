
cv = require 'opencv'

video = new cv.VideoCapture 'test/my_video-5.mkv'
extract = new cv.BackgroundSubtractor()
extract = cv.BackgroundSubtractor.createMOG();
console.log extract

# copyWithMask

iteration = 0

process = (err, input) ->
  throw err if err
  
  extract.applyMOG input, (err, mask) ->
    console.log 'applied', err, mask
    throw err if err
    [height, width] = mask.size()
    masked = new cv.Matrix height, width
    console.log 's', mask.size(), masked.size()
    input.copyWithMask masked, mask
    
    if iteration >= 0
      mask.save("1mask#{iteration}.jpg")
      masked.save("2masked#{iteration}.jpg")
    #console.log 'masked', masked

    # Next frame
    iteration += 1
    setTimeout () ->
      video.read process
    , 100

video.read process
###
-try {
-  var camera = new cv.VideoCapture(0);
-  //var window = new cv.NamedWindow('Video', 0)
-  
-  console.log("Running capture");
-  setInterval(function() {
-    camera.read(function(err, im) {
-      if (err) throw err;
-      //console.log(im.size())
-      var size = im.size();
-      if (size[0] > 0 && size[1] > 0){
-        im.save("image"+number+".jpg");
-        number+=1;
-      }
-      //window.blockingWaitKey(0, 50);
-    });
-  }, 20);
-  
-} catch (e){
-  console.log("Couldn't start camera:", e)
-}
###

