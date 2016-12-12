
chai = require 'chai'
sendToArduino = require '../components/SendToArduino'

s = "updated distance=2322"

parsed = sendToArduino.parseDistance s
chai.expect(parsed).to.equal 2322
