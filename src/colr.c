#include <pebble.h>

#define KEY_DATA 5

#define BG_RED_DATA 0
#define BG_GREEN_DATA 1
#define BG_BLUE_DATA 2
#define SHOW_DATE 3
#define SHOW_SECONDS 4
#define SHOW_BLUETOOTH 5
#define SHOW_TEMPERATURE 6
#define SHOW_WEATHER_CONDITIONS 7
#define USE_CUSTOM_BACKGROUND_VAL 8

//Main window (where everything goes)
static Window *s_main_window;
//Some text (the time)
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;
//Font
static GFont s_time_font;
static GFont s_date_font;
//Graphics layer (battery bar)
static Layer *graphics_layer;
//Battery %
static int charge_percent = 50;
//Options
static bool show_seconds = true;

//Methods for communicating with android app
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *data = dict_read_first(iterator);  
  
  while (data != NULL) {
    switch(data->key) {
      case BG_RED_DATA:
        //The new red value
        
        break;
      case BG_GREEN_DATA:
        //The new green value
        break;
      case BG_BLUE_DATA:
        //The new blue value
        break;
      case SHOW_DATE:
        //The boolean (0/1) value for date
        break;
      case SHOW_SECONDS:
        //The boolean (0/1) value for seconds
        if ((int)data->value->int32 == 0) {
          //Don't show seconds
          APP_LOG(APP_LOG_LEVEL_INFO, "set");
          show_seconds = false;
        }
        break;
      
    }
  }
  
  //window_set_background_color(s_main_window, GColorFromRGB(, , ));
  APP_LOG(APP_LOG_LEVEL_INFO, "Message Received");
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message Dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send failed");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success");
}

//Create a TickTimerService to get the time
static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  //Update the time
  static char s_time_text[] = "00:00:00";
  strftime(s_time_text, sizeof(s_time_text), show_seconds ? "%H:%M:%S" : "%H%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_text);
  //Update the date
  static char s_date_text[16];
  strftime(s_date_text, sizeof(s_date_text), "%a %d %b", tick_time);
  text_layer_set_text(s_date_layer, s_date_text);
  //Get int value of hours, minutes, and seconds
  int hour = tick_time->tm_hour;  
  int min = tick_time->tm_min;
  int sec = tick_time->tm_sec;
  //We want to use the time as a hex code (#RR(hour)GG(min)BB(sec)) so we must convert from Hex to RGB so we can use
  //GColorFromRGB
  //
  //RGB is the int value out of 255, so we have to divide each value by 255, and multiply by 1000 
  //to get the correct R,G,B values.
  int red = (hour / 255.0) * 1000;
  int grn = (min / 255.0) * 1000;
  int blu = (sec / 255.0) * 1000;  
  //Now we set the background color to the R, G & B values using GColorFromRGB
  window_set_background_color(s_main_window, GColorFromRGB(red, grn, blu));
}

static void graphics_update_proc(Layer *this_layer, GContext *ctx) {    
  //Battery bar
  GRect bounds = layer_get_bounds(this_layer);
  //Draw the bar
  graphics_context_set_fill_color(ctx, GColorBlack);
  ////Set the bar to the battery %  
  //////If the watch is charging, we need to set it to full & make it white or something
  if (charge_percent == 200) {
    //The watch is charging
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, PBL_IF_ROUND_ELSE(105, 95), bounds.size.w, 5), 0, GCornerNone);
  } else {
    //The watch is not charging
    int battery_bar_width = ((bounds.size.w*charge_percent)/(100));
    graphics_fill_rect(ctx, GRect(0, PBL_IF_ROUND_ELSE(105, 95), battery_bar_width, -30), 0, GCornerNone);
  }
}

static void main_window_load(Window *window) {
  //Get information about the window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Create the text layer
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(80, 70), bounds.size.w, 150));
  s_date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(110, 100), bounds.size.w, 50));
  
  //Graphics layer
  graphics_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_add_child(window_layer, graphics_layer);
  layer_set_update_proc(graphics_layer, graphics_update_proc);
  
  //Text
  ////Time
  text_layer_set_background_color(s_time_layer, GColorClear);  
  text_layer_set_text_color(s_time_layer, GColorWhite);                                   
  text_layer_set_text(s_time_layer, "00:00");
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_QUADRATS_20));
  text_layer_set_font(s_time_layer, s_time_font);                                 
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  ////Date
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "Date");
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_QUADRATS_SMALL_15));
  text_layer_set_font(s_date_layer, s_date_font);  
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  ////Weather
  //text_layer_set_background_color(s_weather_layer, GColorClear);
  //text_layer_set_text_color(s_weather_layer, GColorWhite);
  //text_layer_set_text(s_weather_layer, "Weather");  
  
  //Time service
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
     
  //Battery bar
  BatteryChargeState charge_state = battery_state_service_peek();
  charge_percent = charge_state.charge_percent;
  
  //BG
  int hour = current_time->tm_hour;
  int min = current_time->tm_min;
  int sec = current_time->tm_sec;  
  window_set_background_color(s_main_window, GColorFromRGB(hour + 150, min + 150, sec + 150));
  
  //Add it as a child to the window
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
}

static void main_window_unload(Window *window) {
  //Destroy the text layer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  //Unload GFont
  fonts_unload_custom_font(s_time_font);
}

//Battery change listener
static void battery_change_listener(BatteryChargeState charge_state) {
  
  if (charge_state.is_charging) {
    //I made it 200 because the % will never reach that otherwise. It could be anything > 100 
    //That way we can distinguish from charging and not charging easier 
    //from graphics_update_proc()
    charge_percent = 200;
  } else {
    charge_percent = charge_state.charge_percent;
  } 
}

static void init() {  
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Start");
  
  //Register callbacks for phone app communication
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "set callbacks");
  
  //Open Appmessage
  app_message_open(128, 128);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Open App message");
  
  //Main window
  s_main_window = window_create();
      
  //Elements inside the window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });  
    
  //Subscribe to battery service
  battery_state_service_subscribe(battery_change_listener);
  
  //Show the window on the watch
  window_stack_push(s_main_window, true);
}

static void deinit() {
  //Destroy the window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}