
noflo = require 'noflo'
Color = require 'color'

validateInput = (input) ->
  throw new Error "Input is not an array" if not Array.isArray input
  throw new Error "Input array is empty" if not input.length
  is8bit = (number) ->
    return number >= 0 and number <= 255
  isRgb = (item) ->
    return item.length == 3 and is8bit item[0] and is8bit item[1] and is8bit item[2]
  wrong = input.filter (i) -> not isRgb i
  throw new Error "Some inputs are not RGB 8 bit colors: #{wrong}" if wrong.length
  return true

enrichColor = (rgb) ->
  c = Color().rgb(rgb)
  hsl = c.hsl()
  hsv = c.hsv()
  hwb = c.hwb()
  d =
    r: rgb[0]
    g: rgb[1]
    b: rgb[2]
    h: hsl.h
    s: hsl.s
    l: hsl.l
    v: hsv.v
    w: hwb.w
    b: hwb.b
    lum: c.luminosity()
  return d

applyWeight = (c) ->
  # prefer
  c.weight = ((c.v/255) * 0.5) + ((c.s/255) * 0.5)
  return c

rankColors = (input) ->
  colors = input.map (c) -> enrichColor c
  colors = colors.map applyWeight
  sorted = colors.sort (a, b) -> return a.weight < b.weight
  return sorted

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
        ranked = rankColors payload
        pick = ranked[0]
        rgb = [pick.r, pick.g, pick.b]
        out.send rgb
    catch e
        return callback e
                
    return callback null 

  return c
