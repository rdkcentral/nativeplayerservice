# NativePlayer

C++17 launcher application that plays a manifest URL through `aamp`.

## Structure

- `source/` - application source files

## Build

Use https://github.com/rdkcentral/meta-bolt-native-player-service

## Testing the player

Connect the player using websocat from external device (or from box)
```
websocat ws://10.0.0.11:10101
//Open session
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.nativeplayer.openSession", "params":{"instanceId":"123-1232-321312-3123","displayId":"wst-mywindow"}}
{"id":"123454","jsonrpc":"2.0","result":{"sessionId":"session_1783267426","status":true}}

//Play a manifest
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.nativeplayer.play", "params":{"url":"http://dash.akamaized.net/dash264/TestCases/1a/qualcomm/1/MultiRate.mpd","sessionId":"session_1783267426"}}
{"id":"123454","jsonrpc":"2.0","result":{"message":"Content playback started.","status":true}}

//Stop playback
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.nativeplayer.stop", "params":{"sessionId":"session_1783267426"}}
{"id":"123454","jsonrpc":"2.0","result":{"message":"Playback stopped successfully.","status":true}}

//Close the session
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.nativeplayer.closeSession", "params":{"sessionId":"session_1783267426"}}
{"id":"123454","jsonrpc":"2.0","result":{"message":"Session closed successfully.","status":true}}

//Invalid playback
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.nativeplayer.play", "params":{"url":"http://dash.akamaized.net/dash264/TestCases/1a/qualcomm/1/MultiRate.mpd","sessionId":"session_1783267426"}}
{"id":"123454","jsonrpc":"2.0","result":{"message":"Session is not initialized.","status":false}}
```
## Reference
[AAMP](https://github.com/rdkcentral//aamp)