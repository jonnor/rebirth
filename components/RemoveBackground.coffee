
noflo = require 'noflo'
cv = require 'opencv'

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Remove background from images using a temporal statistical subtractor'
  c.icon = 'picture-o'

  c.inPorts.add 'in',
    datatype: 'number'
    description: 'A opencv.Matrix to remove background from'

  c.outPorts.add 'out',
    datatype: 'object'
    decription: 'A opencv.Matrix with background removed'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  c.subtractor = null

  noflo.helpers.WirePattern c,
    in: 'in'
    out: 'out'
    params: []
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->
    
    input = payload
    if not c.subtractor
      c.subtractor = new cv.BackgroundSubtractor()
    c.subtractor.applyMOG input, (err, mask) ->
      [height, width] = mask.size()
      masked = new cv.Matrix height, width
      input.copyWithMask masked, mask        

      out.send masked
      return callback null
      
  return c
