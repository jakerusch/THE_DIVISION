#include <pebble.h>

#define KEY_TEMP 0
#define KEY_CITY 1

#define ANTIALIASING true

static Window *s_main_window;
static GFont s_font, s_clock_font, s_colon_font;
static Layer *s_circle, *s_seconds_circle;
static GBitmap *s_icon, *s_lightening, *s_bluetooth;
static BitmapLayer *s_icon_layer, *s_lightening_layer, *s_bluetooth_layer;
static TextLayer *s_date_layer, *s_hour_layer, *s_minute_layer, *s_colon_layer, *s_ampm_layer, *s_temp_layer, *s_city_layer;
BatteryChargeState battery_state;

static void bluetooth_callback(bool connected) {
  // show icon if connected
  layer_set_hidden(bitmap_layer_get_layer(s_bluetooth_layer), !connected); 
    
  if(!connected) {
    vibes_double_pulse();
  }
}

// draws gray circle
static void circle_update_proc(Layer *layer, GContext *ctx) {
  // offset bounds
  GRect bounds = GRect(-6, 0, 156, 168);
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 10, DEG_TO_TRIGANGLE(360-(battery_state.charge_percent*3.6)), DEG_TO_TRIGANGLE(360));
}

// draws seconds ticker
static void seconds_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  GRect bounds;
  graphics_context_set_antialiased(ctx, true);
  
  // do something every 10 seconds
  int32_t second_angle = t->tm_sec*6;
  
  // if even minute, add white
  // if odd minute, add full white and overlay black
  if(t->tm_min%2) {
    // odd
    // offset bounds, outer circle receeding
    bounds = GRect(-7, 0, 158, 168);
    graphics_context_set_fill_color(ctx, GColorWhite); 
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(second_angle), DEG_TO_TRIGANGLE(360));  
  
    // offset bounds, inner circle receeding
    bounds = GRect(4, 0, 136, 168);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(second_angle), DEG_TO_TRIGANGLE(360));        
  } else {
    // even
    // offset bounds, outer circle increasing
    bounds = GRect(-7, 0, 158, 168);
    graphics_context_set_fill_color(ctx, GColorWhite); 
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(second_angle));  
  
    // offset bounds, inner circle increasing
    bounds = GRect(4, 0, 136, 168);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(second_angle));      
  }  
}

static void main_window_load(Window *window) {
  window_set_background_color(window, GColorBlack); // default GColorWhite
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //////////////////
  // create fonts //
  //////////////////
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BORDA_BOLD_FONT_12));
  s_colon_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BORDA_BOLD_FONT_24));
  s_clock_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BORDA_FONT_36));
  
  /////////////////
  // create icon //
  /////////////////
  s_icon = gbitmap_create_with_resource(RESOURCE_ID_DIVISION_ICON_BLACK);
  s_icon_layer = bitmap_layer_create(GRect(0, 24, 144, 32));
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_icon_layer, s_icon);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer)); // add layer to window
  
  ///////////////////
  // create circle //
  ///////////////////
  s_circle = layer_create(bounds);
  layer_set_update_proc(s_circle, circle_update_proc);
  layer_add_child(window_layer, s_circle); // add layer to window
  
  ///////////////////////
  // create date layer //
  ///////////////////////
  s_date_layer = text_layer_create(GRect(18, 52, 108, 15));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite); 
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  text_layer_set_font(s_date_layer, s_font);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer)); 
  
  ////////////////////////
  // create colon layer //
  ////////////////////////
  s_colon_layer = text_layer_create(GRect(0, 68, 144, 30));
  text_layer_set_background_color(s_colon_layer, GColorClear);
  text_layer_set_text_color(s_colon_layer, GColorWhite); 
  text_layer_set_text_alignment(s_colon_layer, GTextAlignmentCenter);
  text_layer_set_font(s_colon_layer, s_colon_font);
  text_layer_set_text(s_colon_layer, ":");
  layer_add_child(window_layer, text_layer_get_layer(s_colon_layer));   
  
  ///////////////////////
  // create hour layer //
  ///////////////////////
  s_hour_layer = text_layer_create(GRect(0, 58, 70, 36));
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorWhite); 
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentRight);
  text_layer_set_font(s_hour_layer, s_clock_font);
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));   

  /////////////////////////
  // create minute layer //
  /////////////////////////
  s_minute_layer = text_layer_create(GRect(73, 58, 70, 36));
  text_layer_set_background_color(s_minute_layer, GColorClear);
  text_layer_set_text_color(s_minute_layer, GColorWhite); 
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentLeft);
  text_layer_set_font(s_minute_layer, s_clock_font);
  layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));  
  
  ////////////////////////
  // create am/pm layer //
  ////////////////////////
  s_ampm_layer = text_layer_create(GRect(110, 67, 25, 14));
  text_layer_set_text_alignment(s_ampm_layer, GTextAlignmentCenter);
  text_layer_set_font(s_ampm_layer, s_font);
  layer_add_child(window_layer, text_layer_get_layer(s_ampm_layer)); 
  
  ///////////////////////
  // create temp layer //
  ///////////////////////
  s_temp_layer = text_layer_create(GRect(110, 82, 25, 14));
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text_color(s_temp_layer, GColorWhite);   
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
  text_layer_set_font(s_temp_layer, s_font);
  layer_add_child(window_layer, text_layer_get_layer(s_temp_layer));  
  
  /////////////////////////
  // draw seconds circle //
  /////////////////////////
  s_seconds_circle = layer_create(bounds);
  layer_set_update_proc(s_seconds_circle, seconds_update_proc);
  layer_add_child(window_layer, s_seconds_circle); // add layer to window  
  
  ///////////////////////
  // create city layer //
  ///////////////////////
  s_city_layer = text_layer_create(GRect(17, 96, 110, 14));
  text_layer_set_background_color(s_city_layer, GColorClear);
  text_layer_set_text_color(s_city_layer, GColorWhite);
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);
  text_layer_set_font(s_city_layer, s_font);
  layer_add_child(window_layer, text_layer_get_layer(s_city_layer));   

  ////////////////////
  // bluetooth icon //
  ////////////////////
  s_bluetooth = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_WHITE_ICON);
  s_bluetooth_layer = bitmap_layer_create(GRect(8, 74, 14, 14));
  bitmap_layer_set_compositing_mode(s_bluetooth_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bluetooth_layer, s_bluetooth); 
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bluetooth_layer));  
      
  ///////////////////
  // charging icon //
  ///////////////////
  s_lightening = gbitmap_create_with_resource(RESOURCE_ID_LIGHTENING_WHITE_ICON);
  s_lightening_layer = bitmap_layer_create(GRect(22, 74, 14, 14));
  bitmap_layer_set_compositing_mode(s_lightening_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_lightening_layer, s_lightening); 
  layer_add_child(window_layer, bitmap_layer_get_layer(s_lightening_layer));   
}

static void update_time() {
  // get a tm strucutre
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // %a = Day (Mon)
  // %m = Month (01..12)
  // %d = Day (01..31)
  static char s_date_buffer[32];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %m-%d", tick_time);

  // write the hour into a buffer
  static char s_hour_buffer[8];
//   strftime(s_hour_buffer, sizeof(s_hour_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
  strftime(s_hour_buffer, sizeof(s_hour_buffer), "%I", tick_time);
  
  // write the minutes into a buffer
  static char s_minute_buffer[8];
  strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
  
  static char s_ampm_buffer[8];
  strftime(s_ampm_buffer, sizeof(s_ampm_buffer), "%p", tick_time);
  
  // display this date, hour, and minute on the text layers
  text_layer_set_text(s_date_layer, s_date_buffer);
  text_layer_set_text(s_hour_layer, s_hour_buffer);
  text_layer_set_text(s_minute_layer, s_minute_buffer);
  text_layer_set_text(s_ampm_layer, s_ampm_buffer);
}

static void time_handler(struct tm *tick_time, TimeUnits units_changed) {
  int sec = tick_time->tm_sec;
  switch(sec) {
    case 0:
      update_time();
      layer_mark_dirty(s_seconds_circle);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "minute"); 
    break;
    case 10:
    case 20:
    case 30:
    case 40:
    case 50:
      layer_mark_dirty(s_seconds_circle);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "second");   
    break;
  }
  
  /////////////////////////////////////////
  // Get weather update every 30 minutes //
  /////////////////////////////////////////
  if(tick_time->tm_min % 30 == 0) {
    
    //////////////////////
    // Begin dictionary //
    //////////////////////
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    //////////////////////////
    // Add a key-value pair //
    //////////////////////////
    dict_write_uint8(iter, 0, 0);

    ///////////////////////
    // Send the message! //
    ///////////////////////
    app_message_outbox_send();
  }    
}

static void handle_battery(BatteryChargeState charge_state) {
  battery_state = charge_state;
  layer_mark_dirty(s_circle);
  if (charge_state.is_charging) {
    layer_set_hidden(bitmap_layer_get_layer(s_lightening_layer), false);
  } else {
    layer_set_hidden(bitmap_layer_get_layer(s_lightening_layer), true);
  }  
}

static void main_window_unload(Window *window) {
  layer_destroy(s_circle);
  gbitmap_destroy(s_icon);
  bitmap_layer_destroy(s_icon_layer);
  bluetooth_connection_service_unsubscribe();
}

///////////////////////
// for weather calls //
///////////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  ////////////////////////////////
  // Store incoming information //
  ////////////////////////////////
  static char temp_buf[8];
  static char temp_layer_buf[32];

  static char city_buf[32];
  static char city_layer_buf[32];
  
  //////////////////////////
  // Read tuples for data //
  //////////////////////////
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMP);
  Tuple *city_tuple = dict_find(iterator, KEY_CITY);

  //////////////////////////////////////
  // If all data is available, use it //
  //////////////////////////////////////
  if(temp_tuple && city_tuple) {
    /////////////////
    // temperature //
    /////////////////
    snprintf(temp_buf, sizeof(temp_buf), "%d°", (int)temp_tuple->value->int32);
    snprintf(temp_layer_buf, sizeof(temp_layer_buf), "%s", temp_buf);
    
    //////////
    // city //
    //////////
    snprintf(city_buf, sizeof(city_buf), "%s", city_tuple->value->cstring);
    snprintf(city_layer_buf, sizeof(city_layer_buf), "%s", city_buf);
    
    text_layer_set_text(s_temp_layer, temp_layer_buf);
    text_layer_set_text(s_city_layer, city_layer_buf);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void init(void) {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  }); 
  window_stack_push(s_main_window, true);
  
  // start with updated time
  update_time();  
  
  // register with second ticks
  tick_timer_service_subscribe(SECOND_UNIT, time_handler);
  
  // register with Battery State Service
  battery_state_service_subscribe(handle_battery);
  handle_battery(battery_state_service_peek());  
  
  // register with bluetooth state service
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  bluetooth_callback(connection_service_peek_pebble_app_connection());

  // Register weather callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);  
  
  // Open AppMessage for weather callbacks
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);    
}

void deinit(void) {
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}