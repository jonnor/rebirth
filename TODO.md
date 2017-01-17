
# Make it work

## Problem
After a time, does not transition to animation when close to the sensor.
In one case it took 10 minutes, in another case 5 minutes.

## Possible causes
What could cause problem, and how to observe symptoms of such a cause.

```
### In RPi code

F1. Distance value from sensor is being reported too high.
Invalid values on `anim.DISTANCE`, always above something.

F2. Distance value stops being reported.
No new data on `sender.DISTANCE`.

F3. Distance value falls outside valid range and is ignored by `anim.DISTANCE`.
A message on `anim.ERROR` should appear in this case,
and no `anim.DISTANCECHANGED` would appear after a `anim.DISTANCE` message.

F4. Animate has bug in interaction logic.
Gets correct data on `anim.DISTANCE`, but wrong on `anim.HEARTBEAT`/`anim.BREATHINGPERIOD`.

F5. Animate component is deadlocked.
Not sending new `anim.LEDCOLOR`

F6. Animate component is starved for CPU.
Not sending new `anim.LEDCOLOR`.
Would expect symptom to instead be slow/laggy color updates, not completely missing.

F7. Animate component restarts continiously.
Not sending new `anim.LEDCOLOR`.

F8. msgflo-setup not delivering messages from one participant to the other.
Either existing message on `anim.LEDCOLOR` then missing on `send.IN`,
or existing on `send.DISTANCE` but missing on `anim.DISTANCE`. 
Possible workaround: Use same queues on both sides, avoids need for msgflo-setup binding.

F9. Exception in Sender leaving broken state instead of restarting.
Should see something in rebirth-sender.service logs.

### On RPi system

F10. Mosquitto server stops delivering messages between participants.
Either all messages, or to one particular client/topic.

### On Arduino side

F11. Arduino restarts once, breaking the serial connection needed for updating.
`send.ERROR` should have message in this case, but untested.

F12. Arduino restarts continiously, re-initializing the LEDs to new value.
`send.ERROR` should have message in this case, but untested.

F13. Arduino analog input is electrically shorted to 5V. 
`anim.DISTANCE` has only high values.

F15. Arduino sensor reading/filtering/sending code has bug.
Correct values on `send.IN` and `send.OUT`, and no `send.ERROR`, but updated color not showing.

F15. Arduino color receiveing/updating code has bug.
Correct values on `send.IN` and `send.OUT`, and no `send.ERROR`, but updated color not showing.

```

## Observations
What we know...

From 1 case of being logged into system when problem was present,
before updating to individual systemd:

```
* All processes were running. Unknown if they were up for long or not.
* Overall system load was low, 10-20% CPU.
! `animate` process was using 0% CPU.
.. No logs were available
.. Unable to look at message queues.
.. Unknown memory usage on system
.. Unknown whether restaring code made it functional again
```

## Approaches

### A) Investigate and fix RPI

Attempt to reproduce issue by investigating RPI code on another Linux system,
then investigate live.

```
+ Does not invalidate what we know from before
- A overly complex system for the simple interaction needed
! Without access to Arduino, unable to account for possible bugs in Arduino and Sender.
  With access, can verify to 90%
```

Tools

* Fuzzing of input values to Animate and Sender to provoke fault
* Queue monitoring to detect missing values
* Queue monitoring to detect messages on ERROR ports
* Queue monitoring to detect out-of-range/invalid values

TODO

* Verify that animate and sender is restarted by systemd if crashed

### B) Move everything to Arduino

```
- Changes the system from what is known, potentially introducing new problems.
- May take a long time to do.
+ Amir can flash update the device with new updates
! Without access to a Arduino, unlikely the Arduino code will work on first try.
  With access, can verify 99% by having a potmeter and three LEDs connected.
+ Reduces number of components in the system substantially, lowering complexity.
  No RPi, no cross-device or cross-program communication.
  Mosquitto, msgflo-setup, msgflo-cpp, NoFlo/noflo-msgflo, serialport
- Debugging live on Arduino tricker than on RPi.
- Failures in production will not be possible to introspect after-the-fact.
- Cannot account for electrical problem on Arduino.
```

# Future

## Color mirroring

## Make it nice

* Configure the Mosquitto server to have WebSockets support
* Serve the webinterface, and setup in the graph
* Increase interactivity
Make the heartbeat/breathing intensify in inverse porportion to distance
* Improve transition
When making transitions, do a mix between previous and new animations, over a short period?
Or probably better to have heart/breathing rate move towards new target gradually. 
* Improve animation.
Tweak length of spacing between the two parts of the beat
https://www.freesound.org/people/klankbeeld/sounds/181805/

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

# Tooling wants

* Queue checking/monitoring for MsgFlo. https://github.com/msgflo/msgflo/issues/55
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
Related, details on how Beziers work: http://h14s.p5r.org/2013/01/bezier.html
Unrelated, fun hacking of fast inverse/squareroot computation, http://h14s.p5r.org/2012/09/0x5f3759df.html
Another, fast implementation of average (of two numbers), https://www.approxion.com/?p=119
* Have an emulated Arduino. Should echo back. For testing `SendToArduino` component.
`socat PTY,link=/dev/ttyS10 PTY,link=/dev/ttyS11`
Alternative would be to use MicroFlo on the Arduino, and running emulator locally.
