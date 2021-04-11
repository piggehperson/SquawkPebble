Pebble.addEventListener('ready', function() {
    // PebbleKit JS is ready!
    console.log('PebbleKit JS ready!');
    Pebble.sendAppMessage({'JSReady': 1});

    // Check if user is logged in
    var is_logged_in = true;
    if (is_logged_in){
      // Start listening for their tweet
      Pebble.sendAppMessage({'StartListening': 1});
      console.log('Start listening for user\'s tweet');
    } else {
      // Prompt them to log in
      Pebble.sendAppMessage({'PromptAddAccount': 1});
      console.log('Asking user to log in');
    }
  });

  // Get AppMessage events
Pebble.addEventListener('appmessage', function(e) {
  // Get the dictionary from the message
  var dict = e.payload;

  console.log('Got message: ' + JSON.stringify(dict));

  if(dict['TweetMessage']) {
    // The RequestData key is present, read the value
    var value = dict['TweetMessage'];
    console.log('Dictated message: ' + dict['TweetMessage']);
  }
});