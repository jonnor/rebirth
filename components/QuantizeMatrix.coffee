
noflo = require 'noflo'
Canvas = require 'canvas'

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Quantize the colors of an OpenCV Matrix using integer rounding'
  c.icon = 'picture-o'

  c.inPorts.add 'in',
    datatype: 'array'
    description: 'An OpenCV Matrix'
  c.inPorts.add 'number',
    datatype: 'integer'
    description: 'How much to reduce colors to'
    default: 2

  c.outPorts.add 'out',
    datatype: 'array'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  noflo.helpers.WirePattern c,
    in: 'in'
    out: 'out'
    params: 'number'
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->

    matrix = payload 
    quantized = matrix.copy()
    quantized.divide c.params.number
    quantized.multiply c.params.number

    out.send quantized
    return callback null

  return c
