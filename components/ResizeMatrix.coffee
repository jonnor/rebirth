
noflo = require 'noflo'
Canvas = require 'canvas'

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Quantize the colors of an OpenCV Matrix using integer rounding'
  c.icon = 'picture-o'

  c.inPorts.add 'in',
    datatype: 'object'
    description: 'An OpenCV Matrix'
  c.inPorts.add 'width',
    datatype: 'integer'
    description: 'New width'
    default: 128
  c.inPorts.add 'height',
    datatype: 'integer'
    description: 'New height'
    default: 128
  c.inPorts.add 'quality',
    datatype: 'integer'
    description: 'Controls rescaling algoritm used'
    default: 2
    max: 4
    min: 0

  c.outPorts.add 'out',
    datatype: 'object'
    description: 'A resized OpenCV Matrix'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  noflo.helpers.WirePattern c,
    in: 'in'
    out: 'out'
    params: ['width', 'height', 'quality']
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->

    matrix = payload 
    resized = matrix.copy()
    resized.resize c.params.width, c.params.height

    out.send resized
    return callback null

  return c
