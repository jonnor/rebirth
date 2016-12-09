
noflo = require 'noflo'
Canvas = require 'canvas'

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Quantize the colors of an OpenCV Matrix using integer rounding'
  c.icon = 'picture-o'

  c.inPorts.add 'in',
    datatype: 'object'
    description: 'An OpenCV Matrix'
  c.inPorts.add 'k',
    datatype: 'integer'
    description: 'Number of colors (K value)'
    default: 16

  c.outPorts.add 'out',
    datatype: 'object'
    description: 'An OpenCV Matrix'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  noflo.helpers.WirePattern c,
    in: 'in'
    out: 'out'
    params: 'k'
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->

    matrix = payload
    reduced = matrix.copy()
    reduced.reduceColorsKmeans c.params.k

    out.send reduced
    return callback null

  return c
