
noflo = require 'noflo'

buildUserAgent = ->
  "#{pkg.name}/#{pkg.version} (+#{pkg.repository.url})"

validateInput = (input) ->
  throw new Error "Input is not an array" if not Array.isArray input
  throw new Error "Input array is empty" if not input.length
  is8bit = (number) ->
    return number > 0 and number < 256
  isRgb = (item) ->
    return item.length == 3 and is8bit item[0] and is8bit item[1] and is8bit item[2]
  wrong = input.filter (i) -> not isRgb i
  throw new Error "Some inputs are not RGB 8 bit colors: #{wrong}" if wrong.length
  return true

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Pick the most suitable color from a set'
  c.icon = 'picture-o'

  c.inPorts.add 'colors',
    datatype: 'array'
    description: 'Array of RGB 8bit colors to pick from'

  c.outPorts.add 'color',
    datatype: 'array'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  noflo.helpers.WirePattern c,
    in: 'colors'
    out: 'color'
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->
    
    try
        validateInput payload
        pick = payload[0]
        out.send pick
    catch e
        return callback e
                
    return callback null 

  return c
