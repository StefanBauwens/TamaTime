var messageKeys = require('message_keys');

const LAST_STATE_KEY = "LAST_STATE";
const APISERVER_KEY = "APISERVER";
const ROMURL_KEY = "ROMURL";
const SERVER_SAVE_FAILED_KEY = "SERVER_SAVE_FAILED";

// Import the Clay package
var Clay = require('@rebble/clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});

var xhrRequest = function (url, type, data, callback, errorCallback, timeout = 10000) {
  var xhr = new XMLHttpRequest();

  xhr.timeout = timeout; // in milliseconds (10s default)

  xhr.onload = function () {
    if (xhr.status >= 200 && xhr.status < 300) {
      callback(xhr.responseText);
    } else {
      if (errorCallback) {
        errorCallback(xhr.status, xhr.responseText);
      }
    }
  };

  xhr.onerror = function () {
    if (errorCallback) {
      errorCallback('network_error', null);
    }
  };

  xhr.ontimeout = function () {
    if (errorCallback) {
      errorCallback('timeout', null);
    }
  };

  xhr.open(type, url);

  // Set JSON header if sending data
  if (data) {
    xhr.setRequestHeader('Content-Type', 'application/json;charset=UTF-8');
    xhr.send(JSON.stringify(data));
  } else {
    xhr.send();
  }
};

function FetchScreenInfoAndSendToWatch() // Send last save state back to watch
{
    if(localStorage.getItem(APISERVER_KEY) !== null && localStorage.getItem(APISERVER_KEY).trim().length !== 0)
    {
        //Pebble.sendAppMessage({'JSMessage': "Trying to sync with server..."});
        xhrRequest(localStorage.getItem(APISERVER_KEY) + "/state", 'GET', null, 
        (responseText) => { // success
            console.log("Successfully fetched save state from server: " + responseText);

            // parse it
            let serverState = JSON.parse(responseText);

            if(serverState.memory[0] === null)
            {
                console.log("Empty state received. No need to show anything to user");
                //Pebble.sendAppMessage({'JSMessage': "Empty state received. Restoring from local storage..."});
                return;
            }

            let parsedDict = {
                'STATEmemory': serverState.memory,
                'STATEshowing_attention_icon': serverState.showing_attention_icon
            };

            console.log("Sending save state screen info from server to watch....");
            SendDictRetrying(parsedDict);

        }, 
        (error, response) => { // fail
            console.log("Failed to fetch from server. Error: " + error);
            //Pebble.sendAppMessage({'JSMessage': "Sync failed! Restoring from local storage..."});
        }, 5000);
    }
}

function SendDictRetrying(dict)
{
    Pebble.sendAppMessage(dict, 
    () => { console.log("Success"), 
    () => { // on fail
        console.log("Retrying...");
        setTimeout(() => {
            SendDictRetrying(dict);
        }, 100); 
    }}); 
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
    function(e) {
        console.log('PebbleKit JS ready!');

        // Update s_js_ready on watch
        Pebble.sendAppMessage({'JSReady': 1});

        //TODO use setinterval? to get every minute? Let pebble ask for it, more reliable
    }   
);

// Listen for appmessage
Pebble.addEventListener('appmessage', function(e) {
    var dict = e.payload;
    console.log("Got message: " + JSON.stringify(dict));

    if ('RequestState' in dict)
    {
      FetchScreenInfoAndSendToWatch();
    }
    /*if ('STATEpc' in dict)
    {
        SaveStateAfterClosingApp(dict);
    }*/
  });

// We need to implement this since we are overriding events in webviewclosed
Pebble.addEventListener('showConfiguration', 
    function(e) {
        clay.config = clayConfig;
        Pebble.openURL(clay.generateUrl());
    }
);

// Listen for when web view is closed
Pebble.addEventListener('webviewclosed',
    function(e) {
        if (e && !e.response) { return; }
    
        var dict = clay.getSettings(e.response);

        localStorage.setItem(APISERVER_KEY, dict[messageKeys.APIServerUrl]);
    }
);