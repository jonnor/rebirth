
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
    [width, height] = matrix.size()
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
    for pixelNumber in [0...matrixPixels]
      srcIndex = pixelNumber*3
      tgtIndex = pixelNumber*4
      source = matrixBuffer[srcIndex]
      imageData.data[tgtIndex] = source
    
    # render into canvas  
    ctx.putImageData imageData, 0, 0
  
    out.send canvas
    return callback null

  return c
