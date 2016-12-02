
noflo = require 'noflo'

exports.getComponent = ->
  c = new noflo.Component

  c.description = 'Cause the process to exit'
  c.icon = 'picture-o'

  c.inPorts.add 'in',
    datatype: 'number'
    description: 'Exit code to report'

  c.outPorts.add 'error',
    datatype: 'object'
    description: 'Error'

  noflo.helpers.WirePattern c,
    in: 'in'
    out: 'error'
    forwardGroups: true
    async: true
  , (payload, groups, out, callback) ->
    
    if payload? and typeof payload == 'number'
      code = payload
    else
      code = if payload then 1 else 0
    process.exit code
    return callback null # not that it matters... 

  return c
