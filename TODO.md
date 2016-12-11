
# Software

Make it work

* Add support for distance sensor to `SendArduino`
* Expose `Animate` inputs on Msgflo.
At least the heartbeat/breathing rates.
Maybe also color etc, for quicker experimentation
* Detect idle state as threshold on distance sensing,
change the heartbeat/breathing rate

Make it nice

* Increase interactivity
Make the heartbeat/breathing intensify in inverse porportion to distance
* Improve transition
When making transitions, do a mix between previous and new animations, over a short period?
Or probably better to have heart/breathing rate move towards new target gradually. 
* Improve idle animation.
Use a bit of randomization, maybe Brownian motion.

# Checkpoints

* Monday, around 18.00. Deploy and test final installation code.

# Electronics

* Buy a new PSU cable
* Mount box to structure

# Tooling wants

* Color pickers and numeric sliders in a webUI for tweaking parameters.
Flowhub should be able to do this, using FBP protocol via MsgFlo?
Port type information may be lacking though?
Alternate hack is to send/receive directly on MQTT.
* Visualization of entire simulation timelines in webUI. Primarily the color result.
Optionally have other (numeric) values there also, for debugging of intermediate values.
Flowtrace should be right tool for this?
* Define simulation timeline as data. Show multiple timelines in the webUI.
* Also show the installation in webUI. Sculpture model/picture, output changing the colors.
Show sensor, area where the interactivity happens. Visualize the inputs in the area, allow to manipulate.
* Allow changing a input trace in the UI. Move events around, insert new events, change value of events.
* Tool that allows drawing datalines with Beziers in SVG.
Then evaluates this by sampling, and generate C data. 
Raw data samples as array, number of entries, value mapping (meaning of 0 and max), timestep.
This can then be used to get exactly the behavior wanted.
No need to tween etc at runtime.
Runtime code should be able to do interpolation, at minimum linear.
Initially draw in Inkscape. Over time move to have dedicated webUI inputs.
Data formation should be (de)serializable, so it can be sent over protocol, and persisted.
* Have an emulated Arduino. Should echo back. For testing `SendToArduino` component.
`socat PTY,link=/dev/ttyS10 PTY,link=/dev/ttyS11`
Alternative would be to use MicroFlo on the Arduino, and running emulator locally.

# Future

## Color mirroring

## Subject extraction

* Fix BackgroundSubtractor or use alternate method for extracting subject to analyse

## Color pallette

* Reduce number of colors in image
`ReduceColors` component now uses kmeans. Good results, but slow. 
Quick and dirty alternative is integer truncation, with `QuantizeMatrix`.
Can maybe do piece-wise k-means, this should be much faster.
Alternative algorithms are using [octree](https://rosettacode.org/wiki/Color_quantization#C),
* Note: kmeans and similar methods may work better in CIE LAB space than RGB
* Because we won't display a picture with the reduced colors, dithering is not needed

## Color picking

* With a reduced image, count the existence of the different colors into bins
* From the number of times each color appears, calculate area (in percent)
* Update `PickColor` to also weight area heavily
