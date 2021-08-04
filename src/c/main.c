// By S.G. Productions, Version 1.1, Dec 2015 (If I ever remember to update this, Please check README instead)
#include <pebble.h>
#include <ctype.h>

#define KEY_FONT 0
#define KEY_ACCEL 1
#define KEY_ANIM 2
#define KEY_COLOR_BG_HOUR_1 3
#define KEY_COLOR_BG_HOUR_2 4
#define KEY_COLOR_BG_MIN_1 5
#define KEY_COLOR_BG_MIN_2 6
#define KEY_COLOR_TX_HOUR_1 7
#define KEY_COLOR_TX_HOUR_2 8
#define KEY_COLOR_TX_MIN_1 9
#define KEY_COLOR_TX_MIN_2 10

#define TIMER_DATE_DELAY 3000
#define ANIMATION_DURATION 1000
#define NUMBER_OF_KEYS 11
#define NUMBER_OF_TEXTBOXES 4

static Window *s_main_window;
static TextLayer *time_textbox[NUMBER_OF_TEXTBOXES];
static TextLayer *time_textbox_next[NUMBER_OF_TEXTBOXES];
static TextLayer *date_textbox[NUMBER_OF_TEXTBOXES];

static Layer *s_color_layer;
static GFont s_time_font;

static bool accelOn;
static bool animOn;

AppTimer *timer;

// Just one function prototype because this is the only really necessary one
static void animate_out(TextLayer *time_text_out, TextLayer *time_text_in, int column);

// Parses an input string of numbers and loads corresponding text into textlayers
static void parser(char number[], TextLayer *time_textbox[]) {
  int length;
  int digit = 0;
  char character;
  
  for (length = strlen(number); length >0; length--) {
    character = number[digit]; //determine the character
    switch (character) 
      {
      case 48:
          text_layer_set_text(time_textbox[digit], "Z\nE\nR\nO");
          break;
      case 49:
          text_layer_set_text(time_textbox[digit], "O\nN\nE");
          break;
      case 50:
          text_layer_set_text(time_textbox[digit], "T\nW\nO");
          break;
      case 51:
          text_layer_set_text(time_textbox[digit], "T\nH\nR\nE\nE");
          break;
      case 52:
          text_layer_set_text(time_textbox[digit], "F\nO\nU\nR");
          break;
      case 53:
          text_layer_set_text(time_textbox[digit], "F\nI\nV\nE");
          break;
      case 54:
          text_layer_set_text(time_textbox[digit], "S\nI\nX");
          break;
      case 55:
          text_layer_set_text(time_textbox[digit], "S\nE\nV\nE\nN");
          break;
      case 56:
          text_layer_set_text(time_textbox[digit], "E\nI\nG\nH\nT");
          break;
      case 57:
          text_layer_set_text(time_textbox[digit], "N\nI\nN\nE");
          break;
      default:
          printf("Incorrect input");
      }
  digit=digit+1;
  }
}

// Update a date texbox with formatted date input string
static void parser_date(char number[], TextLayer *date_textbox, char textbox[]) {
  int length;
  int digit = 0;
  int index_char = 0;
  char character;
  for (length = strlen(number); length >0; length--) {
    character = number[digit];
    textbox[index_char] = toupper((int) character);
    textbox[index_char+1] = '\n';
    digit = digit+1;
    index_char = digit*2;
  }
  textbox[index_char-1] = '\0';
  text_layer_set_text(date_textbox, textbox);
}

// Update all date texboxes with the correctly formatted date
static void update_date(struct tm *tick_time) {
  static char s_date_buffer[10];
  
  static char s_date_buffer_week[10];
  strftime(s_date_buffer, sizeof(s_date_buffer_week), "%a", tick_time);
  parser_date(s_date_buffer, date_textbox[0], s_date_buffer_week);
  
  static char s_date_buffer_day[10];
  strftime(s_date_buffer, sizeof(s_date_buffer_day), "%d", tick_time);
  parser_date(s_date_buffer, date_textbox[1], s_date_buffer_day);

  static char s_date_buffer_month[10];
  strftime(s_date_buffer, sizeof(s_date_buffer_month), "%b", tick_time);
  parser_date(s_date_buffer, date_textbox[2], s_date_buffer_month);

  static char s_date_buffer_year[10];
  strftime(s_date_buffer, sizeof(s_date_buffer_year), "%Y", tick_time);
  parser_date(s_date_buffer, date_textbox[3], s_date_buffer_year);
}
// No animation time updater
static void update_time_init() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H%M" : "%I%M", tick_time);
  
  // Parse the time in numbers and set apporpriate text to textlayer(s)
  parser(s_buffer, time_textbox);
  
  update_date(tick_time);
}
// Anmiated time updater
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H%M" : "%I%M", tick_time);
  
  // Parse the time in numbers and set apporpriate text to the next textlayer(s)
  parser(s_buffer, time_textbox_next);
  // If textbox needs replacing, animate!
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++){
    int len = strlen(text_layer_get_text(time_textbox[i]));
    if (memcmp(text_layer_get_text(time_textbox[i]), text_layer_get_text(time_textbox_next[i]), len) != 0) {
      animate_out(time_textbox[i], time_textbox_next[i], i);
    };
  }
  // Set the date!
  update_date(tick_time);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (animOn){
    update_time();
  }
  else {
    update_time_init();
  }
}

static void setup_text(Layer *window_layer) {
  // Loading Text colors
  GColor tx_color[NUMBER_OF_TEXTBOXES];
  int key_tx_color[NUMBER_OF_TEXTBOXES];
  key_tx_color[0] = persist_read_int(KEY_COLOR_TX_HOUR_1);
  key_tx_color[1] = persist_read_int(KEY_COLOR_TX_HOUR_2);
  key_tx_color[2] = persist_read_int(KEY_COLOR_TX_MIN_1);
  key_tx_color[3] = persist_read_int(KEY_COLOR_TX_MIN_2);
  #if defined(PBL_COLOR)
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++) {
    tx_color[i].argb = key_tx_color[i];
  }
  #elif defined(PBL_BW)
  // Original Pebble color fallbacks (to allow B/W selection)
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++) {
    if (key_tx_color[i] == 192){
      tx_color[i] = GColorBlack;
    } else {
      tx_color[i] = GColorWhite;
    }
  }
  #endif
  
  // Read font setting
  char font[8];
  persist_read_string(KEY_FONT, font, sizeof(font));

  // Load appropriate font
  if(strncmp (font, "LECO", sizeof("LECO")) == 0) {
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_30));
  }
  else if(strncmp (font, "SQUARE", sizeof("SQUARE")) == 0) {
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_30));
  }
  else{
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LATO_BLACK_30));
  }

  GRect bounds = layer_get_bounds(window_layer);
  int stripe_width = bounds.size.w/4;
  
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++) {
    // Creating and Formatting TextLayers
    time_textbox[i] = text_layer_create(
      GRect(bounds.size.w-(stripe_width*(i+1)), bounds.origin.y+5, stripe_width, bounds.size.h-5));
    
    time_textbox_next[i] = text_layer_create(
      GRect(bounds.size.w-(stripe_width*(i+1)), bounds.origin.y+5-bounds.size.h, stripe_width, bounds.size.h-5));
    
    date_textbox[i] = text_layer_create(
      GRect(bounds.size.w-(stripe_width*(i+1)), bounds.origin.y+5, stripe_width, bounds.size.h-5));
    
    text_layer_set_background_color(time_textbox[i], GColorClear);
    text_layer_set_text(time_textbox[i], "N\nU\nL\nL\nX");
    text_layer_set_text_alignment(time_textbox[i], GTextAlignmentCenter);
    
    text_layer_set_background_color(time_textbox_next[i], GColorClear);
    text_layer_set_text(time_textbox_next[i], "N\nU\nL\nL\nX");
    text_layer_set_text_alignment(time_textbox_next[i], GTextAlignmentCenter);
    
    text_layer_set_background_color(date_textbox[i], GColorClear);
    text_layer_set_text(date_textbox[i], "N\nU\nL\nL\nX");
    text_layer_set_text_alignment(date_textbox[i], GTextAlignmentCenter);
    // Set font, color and add Textlayers as child layers to the Window's root layer
    text_layer_set_text_color(time_textbox[i], tx_color[i]);
    text_layer_set_text_color(time_textbox_next[i], tx_color[i]);
    text_layer_set_text_color(date_textbox[i], tx_color[i]);
    text_layer_set_font(time_textbox[i], s_time_font);
    text_layer_set_font(time_textbox_next[i], s_time_font);
    text_layer_set_font(date_textbox[i], s_time_font);
    layer_add_child(window_layer, text_layer_get_layer(time_textbox[i]));
    layer_add_child(window_layer, text_layer_get_layer(time_textbox_next[i]));
  }
}

// Update protocol for color layer (stripes between text and background)
static void color_layer_draw(Layer *layer, GContext *ctx) {
  // Setting stripe colors
  GColor bg_color[NUMBER_OF_TEXTBOXES];
  int key_bg_color[NUMBER_OF_TEXTBOXES];
  key_bg_color[0] = persist_read_int(KEY_COLOR_BG_HOUR_1);
  key_bg_color[1] = persist_read_int(KEY_COLOR_BG_HOUR_2);
  key_bg_color[2] = persist_read_int(KEY_COLOR_BG_MIN_1);
  key_bg_color[3] = persist_read_int(KEY_COLOR_BG_MIN_2);
  #if defined(PBL_COLOR)
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++) {
    bg_color[i].argb = key_bg_color[i];
  }
  #elif defined(PBL_BW)
  // Original Pebble color fallbacks (to allow B/W selection)
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++) {
    if (key_bg_color[i] == 255) {
      bg_color[i] = GColorWhite;
    } else {
      bg_color[i] = GColorBlack;
    }
  }
  #endif
  
  // Create the stripes and Fill the stripes with respective colors
  GRect bounds = layer_get_bounds(layer);
  int stripe_width = bounds.size.w/4;
  GRect stripes[NUMBER_OF_TEXTBOXES];
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++){
    stripes[i] = GRect(bounds.size.w-(stripe_width*(i+1)), bounds.origin.y, stripe_width, bounds.size.h);
    graphics_context_set_fill_color(ctx, bg_color[i]);
    graphics_fill_rect(ctx, stripes[i], 0, GCornerNone);
  }
}

// Handler to reset textbox positions and text after animate_out animation
static void animation_stopped(Animation *animation, bool finished, void *data) {
  update_time_init();
  GRect bounds = layer_get_bounds(s_color_layer);
  int stripe_width = bounds.size.w/4;
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++){
    GRect out = GRect(bounds.size.w-(stripe_width*(i+1)), bounds.origin.y+5, stripe_width, bounds.size.h-5);
    layer_set_frame(text_layer_get_layer(time_textbox[i]), out);
    
    GRect in = GRect(bounds.size.w-(stripe_width*(i+1)), bounds.origin.y+5-bounds.size.h, stripe_width, bounds.size.h-5);
    layer_set_frame(text_layer_get_layer(time_textbox_next[i]), in);
  }
  #ifdef PBL_SDK_2
  property_animation_destroy((PropertyAnimation*)animation);
  #endif
}

// Creates and runs animation of current time textbox out and moving new time textbox in
static void animate_out(TextLayer *time_text_out, TextLayer *time_text_in, int column) {
  GRect bounds = layer_get_bounds(s_color_layer);
  int stripe_width = bounds.size.w/4;
  GRect start_out = GRect(bounds.size.w-(stripe_width*(column+1)), bounds.origin.y+5, stripe_width, bounds.size.h-5);
  GRect finish_out = start_out;
  finish_out.origin.y += bounds.size.h;
  PropertyAnimation *prop_anim_out = property_animation_create_layer_frame(text_layer_get_layer(time_text_out), &start_out, &finish_out);
  Animation *anim_move_out = property_animation_get_animation(prop_anim_out);
  animation_set_curve(anim_move_out, AnimationCurveEaseOut);
  animation_set_duration(anim_move_out, ANIMATION_DURATION);

  GRect start_in = GRect(bounds.size.w-(stripe_width*(column+1)), bounds.origin.y+5-bounds.size.h, stripe_width, bounds.size.h-5);
  GRect finish_in = start_in;
  finish_in.origin.y += bounds.size.h;
  PropertyAnimation *prop_anim_in = property_animation_create_layer_frame(text_layer_get_layer(time_text_in), &start_in, &finish_in);
  Animation *anim_move_in = property_animation_get_animation(prop_anim_in);
  animation_set_curve(anim_move_in, AnimationCurveEaseOut);
  animation_set_duration(anim_move_in, ANIMATION_DURATION);
  
  animation_set_handlers((Animation*) anim_move_in, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler) animation_stopped,
  }, NULL);
  animation_set_handlers((Animation*) anim_move_out, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler) animation_stopped,
  }, NULL);
  animation_schedule(anim_move_out);
  animation_schedule(anim_move_in);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Add the color overlay layer
  s_color_layer = layer_create(bounds);
  layer_set_update_proc(s_color_layer, color_layer_draw);
  layer_add_child(window_get_root_layer(window), s_color_layer);
  
  // Load the text layers
  setup_text(window_layer);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++){
    text_layer_destroy(time_textbox[i]);
    text_layer_destroy(time_textbox_next[i]);
    text_layer_destroy(date_textbox[i]);
  }

  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  
  // Destroy Middle Layer
  layer_destroy(s_color_layer);
}

// Accelerometer timeout action: Remove date, insert time
void timer_callback(void *data) {
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++){
    layer_remove_from_parent(text_layer_get_layer(date_textbox[i]));
    layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(time_textbox[i]));
    layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(time_textbox_next[i]));
  }
}

// Accelerometer trigger to show date: Remove time, insert date
void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  for(int i = 0; i < NUMBER_OF_TEXTBOXES; i++){
    layer_remove_from_parent(text_layer_get_layer(time_textbox[i]));
    layer_remove_from_parent(text_layer_get_layer(time_textbox_next[i]));
    layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(date_textbox[i]));
  }
	// Fire timer	
	timer = app_timer_register(TIMER_DATE_DELAY, (AppTimerCallback)timer_callback, NULL);
}

// AppMessage recieve failure response
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

// Proccess received messages from AppMessage to change background image
static void in_recv_handler(DictionaryIterator *iterator, void *context)
{
  // Get Tuple and iterate through all tuples
  Tuple *t = dict_read_first(iterator);
  for (int i = 0; i < NUMBER_OF_KEYS; i++) {
  if(t)
  {
    switch(t->key)
    {
    case KEY_FONT:
      // It's the KEY_FONT key: Copy incoming value into KEY_FONT
      persist_write_string(KEY_FONT, t->value->cstring);
      break;
    case KEY_ACCEL:
      // Shake to see time on/off toggle
      if(strcmp(t->value->cstring, "on") == 0) {
        persist_write_bool(KEY_ACCEL, true);
        accel_tap_service_subscribe(&accel_tap_handler);
      } else {
        persist_write_bool(KEY_ACCEL, false);
        accel_tap_service_unsubscribe();
      }
      break;
    case KEY_ANIM:
      // Animation on/off toggle
      if(strcmp(t->value->cstring, "on") == 0) {
        persist_write_bool(KEY_ANIM, true);
        animOn = persist_read_bool(KEY_ANIM);
      } else {
        persist_write_bool(KEY_ANIM, false);
        animOn = persist_read_bool(KEY_ANIM);
      }
      break;
    // Background and text color keys
    case KEY_COLOR_BG_HOUR_1:
      persist_write_int(KEY_COLOR_BG_HOUR_1, t->value->uint8);
      break;
    case KEY_COLOR_BG_HOUR_2:
      persist_write_int(KEY_COLOR_BG_HOUR_2, t->value->uint8);
      break;
    case KEY_COLOR_BG_MIN_1:
      persist_write_int(KEY_COLOR_BG_MIN_1, t->value->uint8);
      break;
    case KEY_COLOR_BG_MIN_2:
      persist_write_int(KEY_COLOR_BG_MIN_2, t->value->uint8);
      break;
    case KEY_COLOR_TX_HOUR_1:
      persist_write_int(KEY_COLOR_TX_HOUR_1, t->value->uint8);
      break;
    case KEY_COLOR_TX_HOUR_2:
      persist_write_int(KEY_COLOR_TX_HOUR_2, t->value->uint8);
      break;
    case KEY_COLOR_TX_MIN_1:
      persist_write_int(KEY_COLOR_TX_MIN_1, t->value->uint8);
      break;
    case KEY_COLOR_TX_MIN_2:
      persist_write_int(KEY_COLOR_TX_MIN_2, t->value->uint8);
      break;
    }
  }
  t = dict_read_next(iterator);
  }
  // Force redraw the watchface on config
  window_stack_pop(s_main_window);
  window_stack_push(s_main_window,false);
  update_time_init();
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time_init();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Open AppMessage to enable communication with phone
  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Register with AccelTapService if show date toggle is on
  accelOn = persist_read_bool(KEY_ACCEL);
  if(accelOn){
    accel_tap_service_subscribe(&accel_tap_handler);
  }
  // Turn on animation if toggle is on
  animOn = persist_read_bool(KEY_ANIM);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  
  // Unsubscribe from services
  tick_timer_service_unsubscribe();
  if(accelOn){
    accel_tap_service_unsubscribe();
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}