
React = window.React

# DOM helpers
id = (name) ->
  document.getElementById name

clearChildren = (node) ->
  while node.firstChild
    node.removeChild node.firstChild
  return true

registerChangeListeners = (inputs, client) ->
  onInput = (event) ->
    console.log 'change', event

    port = event.target.name
    value = event.target.value
    topic = "anim.#{port.toUpperCase()}"

    console.log 'updating', value, port, topic

    message = new Paho.MQTT.Message value
    message.destinationName = topic
    client.send message

  updateOutput = (event) ->
    input = event.target
    output = document.getElementById input.name
    output.value = input.value

  for input in inputs
    console.log input
    input.addEventListener 'input', onInput
    input.addEventListener 'change', updateOutput
    updateOutput { target: input }

connectMqtt = () ->

  onConnect = () ->
    console.log "onConnect"
    client.subscribe "World"
    message = new Paho.MQTT.Message "Hello"
    message.destinationName = "World"
    client.send message

  onConnectionLost = (responseObject) ->
    if responseObject.errorCode != 0
      console.log "onConnectionLost:"+responseObject.errorMessage

  onMessageArrived = (message) ->
    console.log "onMessageArrived:"+message.payloadString

  port = 1884
  client = new Paho.MQTT.Client location.hostname, port, "clientId"
  client.onConnectionLost = onConnectionLost;
  client.onMessageArrived = onMessageArrived;

  client.connect {onSuccess:onConnect}
  return client


# Main
main = () ->
  console.log 'main'

  client = connectMqtt()

  inputs = id('inputs').getElementsByTagName 'input'
  registerChangeListeners inputs, client

  console.log 'main done'

main()
