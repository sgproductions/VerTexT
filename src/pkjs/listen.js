Pebble.addEventListener("ready",
  function(e) {
    console.log("PebbleKit JS ready!");
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    Pebble.openURL("https://sgproductions.github.io/VerTexT/index.html");
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));
    
    var dict = {};
    dict.KEY_FONT = configuration.font;
    dict.KEY_ACCEL = configuration.accel;
    dict.KEY_ANIM = configuration.anim;
    dict.KEY_COLOR_BG_HOUR_1 = configuration.x1;
    dict.KEY_COLOR_BG_HOUR_2 = configuration.x2;
    dict.KEY_COLOR_BG_MIN_1 = configuration.x3;
    dict.KEY_COLOR_BG_MIN_2 = configuration.x4;
    dict.KEY_COLOR_TX_HOUR_1 = configuration.t1;
    dict.KEY_COLOR_TX_HOUR_2 = configuration.t2;
    dict.KEY_COLOR_TX_MIN_1 = configuration.t3;
    dict.KEY_COLOR_TX_MIN_2 = configuration.t4;

    //Send to Pebble, persist there
    Pebble.sendAppMessage(dict,
      function(e) {
        console.log("Sending settings data...(dict)" + JSON.stringify(dict));
      },
      function(e) {
        console.log("Settings feedback failed!(dict)");
      }
    );
  }
);