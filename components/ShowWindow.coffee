
noflo = require 'noflo'
cv = require 'opencv'

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Display images in a window'
  c.icon = 'picture-o'

  c.inPorts.add 'image',
    datatype: 'object'
    description: 'Image to display (opencv.Matrix)'
  c.inPorts.add 'name',
    datatype: 'string'
    description: 'Name of the window'
    default: "Image"

  c.outPorts.add 'image',
    datatype: 'object'
  c.outPorts.add 'window',
    datatype: 'object'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  c.window = null

  noflo.helpers.WirePattern c,
    in: 'image'
    out: ['window', 'image']
    params: ['name']
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->

    image = payload
    if not c.window
      flags = 1
      name = c.params.name
      c.window = new cv.NamedWindow name, flags
      out.window.send c.window

    c.window.show image
    c.window.blockingWaitKey 0, 20 # need to spin event loop?
    out.image.send image
    return callback null

  return c
