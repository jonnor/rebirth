
noflo = require 'noflo'
cv = require 'opencv'

startCapture = (c, device, frameCallback) ->
  readFrame = () ->
    c.camera.read (err, image) ->
      return frameCallback err if err
      [width, height] = image.size()
      if width > 0 and height > 0
        return frameCallback null, image

  frameTime = 1000/c.params.framerate
  console.log 'time', frameTime
  c.camera = new cv.VideoCapture device
  c.interval = setInterval readFrame, frameTime

stopCapture = (c) ->
  console.log 'stoppping'
  clearInterval c.interval
  c.interval = null 
  c.camera = null

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Get a video stream from camera'
  c.icon = 'picture-o'

  c.inPorts.add 'device',
    datatype: 'number'
    description: 'Which video device to use. 0 means /dev/video0'
  c.inPorts.add 'framerate',
    datatype: 'number'
    description: 'Number of frames per second to capture'
    default: 30

  c.outPorts.add 'image',
    datatype: 'object'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  c.camera = null
  c.interval = null

  noflo.helpers.WirePattern c,
    in: 'device'
    out: 'image'
    params: 'framerate'
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->
    
    sendFrame = (err, image) ->
      if err
        console.log 'err', err
        stopCapture c
        return callback err
      
      c.outPorts.image.send image
    
    # FIXME: allow stopping
    c.outPorts.image.connect()
    startCapture c, payload, sendFrame

  return c
