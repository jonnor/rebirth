
path = require 'path'
fs = require 'fs'
async = require 'async'
SerialPort = require 'serialport'

testfile = path.join __dirname, './arduinocommands.txt'

contents = fs.readFileSync(testfile, 'utf-8')
commands = contents.split('\n')

port = process.env.REBIRTH_SERIAL || '/dev/ttyACM1'
console.log 'using', port
options =
  baudRate: 9600
  #parser: SerialPort.parsers.readline
serial = new SerialPort port, options

serial.on 'error', (err) ->
  console.error err
  throw err

serial.on 'data', (data) ->
  console.log 'got', data.toString()

sendCommand = (cmd, callback) ->
  cmd = cmd + "\n"
  serial.write cmd, (err) ->
    console.log "sending '#{cmd}'"
    setTimeout () ->
      return callback null
    , 500

serial.on 'open', () ->
  setTimeout () ->
    async.eachSeries commands, sendCommand, (err) ->
      throw err if err
      console.log 'no error'
      process.exit 0
  , 1000
