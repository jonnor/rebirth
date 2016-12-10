
# Software

Make it work

* Detect idle state
Use existinance of foreground object.
Start with binary yes/no
* Expose animation inputs on Msgflo.
At least the rates. Maybe also color for quick experimentation
* Implement transition between idle and detected states
In detection state

Make it nice

* Improve idle animation.
Use randomization, maybe Brownian motion.
* Improve color picking.
Should respect biggest/most prominent colors.
Can it be done just by sorting?
* Tweak webcam streaming setting.
Maybe specify FPS property on VideoCapture?

RPi

* Setup ssh config for single-command connection
* Set up autostart with systemd
* Test BackgoundSubtractor on RPi

# Checkpoints

* Ideally test/enable the idle animation.
* Saturday, around 10.00. Deploy and test final installation code.

# Electronics

* Buy a new PSU cable
* Mount camera to structure
* Mount box to structure

# Future

Improve color analysis

* Quantizie the image to reduce number of colors, removing gradients. Can be done by integer truncating division
http://stackoverflow.com/a/16902092/1967571
* Tweak parameters to RgbQuant?
* Get rid of the matrix->canvas conversion step.
Might be GetColors can take linear 8 bit buffer directly, but need to pass width through.
* Alternative method is K-means clustering
http://docs.opencv.org/3.0-beta/doc/py_tutorials/py_ml/py_kmeans/py_kmeans_opencv/py_kmeans_opencv.html
http://www.alanzucconi.com/2015/05/24/how-to-find-the-main-colours-in-an-image/

Tooling

* Have an emulated Arduino. Should echo back.
`socat PTY,link=/dev/ttyS10 PTY,link=/dev/ttyS11`
Alternative would be to use MicroFlo on the Arduino, and running emulator locally.
* Tool that allows drawing datalines with Beziers in SVG.
Then evaluates this by sampling, and generate a C array.
This can then be used to get exactly the behavior wanted.
