#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;

static DictationSession *s_dictation_session;
static char s_last_text[512];

static void start_listening(void) {
  // Start dictation UI, just for debug rn
  dictation_session_start(s_dictation_session);
}

static void prompt_add_account(void) {
  text_layer_set_text(s_text_layer, 
    "Welcome to Squawk! To get started, add an account on your phone."
  );
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  // A new message has been successfully received

  // Should we start listening?
  Tuple *start_listening_tuple = dict_find(iter, MESSAGE_KEY_StartListening);
  if(start_listening_tuple) {
    // PKJS is ready, start sending messages
    APP_LOG(APP_LOG_LEVEL_INFO, "Received signal to start listening");
    
    start_listening();
  }
  // Should we ask to add an account?
  Tuple *prompt_add_account_tuple = dict_find(iter, MESSAGE_KEY_PromptAddAccount);
  if(prompt_add_account_tuple) {
    // PKJS is ready, start sending messages
    APP_LOG(APP_LOG_LEVEL_INFO, "Received signal to show the account prompt");
    
    prompt_add_account();
  }
}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status,
                                       char *transcription, void *context) {
  // Print the results of a transcription attempt
  APP_LOG(APP_LOG_LEVEL_INFO, "Dictation status: %d", (int)status);

  if(status == DictationSessionStatusSuccess) {
    // Display the dictated text
    snprintf(s_last_text, sizeof(transcription), "Transcription:\n\n%s", transcription);
    text_layer_set_text(s_text_layer, transcription);

    // Declare the dictionary's iterator
  DictionaryIterator *out_iter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);

  if(result == APP_MSG_OK) {
    // Construct the message
    dict_write_cstring(out_iter, MESSAGE_KEY_TweetMessage, transcription);
  } else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }

  // Send this message
  result = app_message_outbox_send();

  // Check the result
  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
  }
  } else {
    // Display the reason for any error
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nReason:\n%d",
             (int)status);
    text_layer_set_text(s_text_layer, s_failed_buff);
  }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  GRect text_frame = PBL_IF_ROUND_ELSE(
    GRect(28, 45, 124, 99),
    GRect(10, 39, 124, 99)
  );
  s_text_layer = text_layer_create(text_frame);
  text_layer_set_text(s_text_layer, 
    "Connecting to phone..."
  );
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);

  // Create new dictation session
  s_dictation_session = dictation_session_create(sizeof(s_last_text),
                                               dictation_session_callback, NULL);

  // Largest expected inbox and outbox message sizes
  const uint32_t inbox_size = 64;
  const uint32_t outbox_size = 256;

  // Open AppMessage
  app_message_open(inbox_size, outbox_size);

  // Register to be notified about inbox received events
  app_message_register_inbox_received(inbox_received_callback);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
