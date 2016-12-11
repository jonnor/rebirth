
noflo = require 'noflo'
SerialPort = require 'serialport'

exports.parseDistance = parseDistance = (cmd) ->
  regexp = 'updated distance=(\d+).*'
  match = cmd.match regexp
  if match?.length > 1
    return match[1]
  return null

openSerial = (c, port, distanceCallback, callback) ->
  c.serialport = new SerialPort port
  c.serialport.on 'error', (err) ->
    c.serialport = null
    return callback err
  c.serialport.on 'open', () ->
    return callback null

  returned = ""
  c.serialport.on 'data', (data) ->
    returned += data.toString()
    cmds = returned.split '\n'
    if cmds.length >= 2
      console.log 'd', cmds[0], cmds.length, cmds[1]
      distance = parseDistance cmd
      distanceCallback distance if distance?
      returned = cmds[1] or ''

readySend = (c) ->
  return c.serialport and c.serialport.isOpen()

writeUpdate = (c, color) ->
  {r, g, b} = color
  cmd = "set r=#{r} g=#{g} b=#{b}\n"
  c.serialport.write cmd

sendUpdate = (c, data, distanceCallback, callback) ->
  return callback new Error "Missing port data" if not data.port
  return callback new Error "Missing color data" if not data.color

  if readySend c
    writeUpdate c, data.color
    return callback null
  else
    openSerial c, data.port, distanceCallback, (err) ->
      return callback err if err
      writeUpdate c, data.color
      return callback null # TODO: wait for ACK

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Send state to the Arduino'
  c.icon = 'picture-o'

  c.inPorts.add 'port',
    datatype: 'string'
    description: 'Serial port to use'
  c.inPorts.add 'color',
    datatype: 'array'
    description: 'RGB 8bit color'

  c.outPorts.add 'distance',
    datatype: 'number'
  c.outPorts.add 'updated',
    datatype: 'object'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  c.serialport = null # Stateful
  c.buffer = []

  noflo.helpers.WirePattern c,
    in: ['color']
    params: 'port'
    out: 'updated'
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->

    color = payload
    if color.length == 3
      [r, g, b] = color
      color = { r, g, b }

    #c.outPorts.distance.connect()
    onDistanceChanged = (d) ->
      c.outPorts.distance.send d
      c.outPorts.distance.disconnect()

    data = { color: payload, port: c.params.port }
    sendUpdate c, data, onDistanceChanged, (err, updated) ->
      return callback err if err
      # FIXME: respect updated
      out.send data.color
      return callback null

  return c
