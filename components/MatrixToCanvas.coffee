
noflo = require 'noflo'
Canvas = require 'canvas'

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Convert an OpenCV Matrix to a Canvas'
  c.icon = 'picture-o'

  c.inPorts.add 'in',
    datatype: 'array'
    description: 'An OpenCV Matrix'

  c.outPorts.add 'canvas',
    datatype: 'array'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  noflo.helpers.WirePattern c,
    in: 'in'
    out: 'canvas'
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->
    
    matrix = payload
    console.log 'matrix', matrix
    [height, width] = matrix.size()
    console.log width, height
    
    canvas = new Canvas width, height
    ctx = canvas.getContext '2d' 
    imageData = ctx.getImageData 0, 0, width, height
    matrixBuffer = matrix.getData()
    
    # input matrix should be RGB (3 bytes per pixel), imageData RGBA (4 bytes per pixel)
    canvasPixels = imageData.data.length/4
    matrixPixels = matrixBuffer.length/3
    if canvasPixels != matrixPixels
      return callback new Error 'Unexpected number of bytes per pixel' 
    
    # Copy from matrixBuffer to imageData
    target = imageData.data
    for i in [0...target.length] by 4
      srcIndex = i*3/4
      tgtIndex = i
      #console.log 'byte', srcIndex, tgtIndex
      target[tgtIndex+0] = matrixBuffer[srcIndex+2] # R
      target[tgtIndex+1] = matrixBuffer[srcIndex+1] # G
      target[tgtIndex+2] = matrixBuffer[srcIndex+0] # B
      target[tgtIndex+3] = 255 # A
    
    # render into canvas  
    ctx.putImageData imageData, 0, 0
  
    out.send canvas
    return callback null

  return c
