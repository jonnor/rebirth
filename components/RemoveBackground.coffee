
noflo = require 'noflo'
cv = require 'opencv'

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Remove background from images using a temporal statistical subtractor'
  c.icon = 'picture-o'

  c.inPorts.add 'in',
    datatype: 'number'
    description: 'A opencv.Matrix to remove background from'

  c.inPorts.add 'history',
    datatype: 'integer'
    description: 'Number of frames to take into account'
    default: 200
  c.inPorts.add 'mixtures',
    datatype: 'integer'
    description: 'Number of Gaussian mixtures to use'
    default: 5
  c.inPorts.add 'backgroundratio',
    datatype: 'number'
    description: 'Parameter of the MOG algorithm'
    default: 0.7
  c.inPorts.add 'noisesigma',
    datatype: 'number'
    description: 'Sigma of noise component'
    default: 0.0

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
    params: ['history', 'backgroundratio', 'mixtures', 'noisesigma']
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->
    
    input = payload
    if not c.subtractor
      c.subtractor = new cv.BackgroundSubtractor()

    # Set parameters
    if c.subtractor.history() != c.params.history
      c.subtractor.history c.params.history
    if c.subtractor.nmixtures() != c.params.mixtures
      c.subtractor.nmixtures c.params.mixtures
    if c.subtractor.backgroundRatio() != c.params.backgroundratio
      c.subtractor.backgroundRatio c.params.backgroundratio
    if c.subtractor.noiseSigma() != c.params.noisesigma
      c.subtractor.noiseSigma c.params.noisesigma

    c.subtractor.applyMOG input, (err, mask) ->
      [height, width] = mask.size()
      masked = new cv.Matrix height, width
      input.copyWithMask masked, mask        

      out.send masked
      return callback null
      
  return c
