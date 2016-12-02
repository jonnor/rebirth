
noflo = require 'noflo'
SerialPort = require 'serialport'
 
openSerial = (c, port, callback) ->
  c.serialport = new SerialPort port
  c.serialport.on 'error', (err) ->
    c.serialport = null
    return callback err
  c.serialport.on 'open', () ->
    return callback null 

readySend = (c) ->
  return c.serialport and c.serialPort.isOpen()

writeUpdate = (c, color) ->
  [r, g, b] = color
  cmd = "SET r=#{r} g=#{g} b=#{b}\n" 
  c.serialport.write cmd

sendUpdate = (c, data, callback) ->
  return callback new Error "Missing port data" if not data.port
  return callback new Error "Missing color data" if not data.color

  if readySend c
    writeUpdate c, data.color
    return callback null
  else
    openSerial c, data.port, (err) ->
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

  c.outPorts.add 'updated',
    datatype: 'object'
  c.outPorts.add 'error',
    datatype: 'object'
    required: false

  c.serialport = null # Stateful
  c.buffer = []



  noflo.helpers.WirePattern c,
    in: ['port', 'color']
    out: 'updated'
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->

    sendUpdate c, payload, (err, updated) ->
      return callback err if err
      out.send updated
      return callback null

  return c
