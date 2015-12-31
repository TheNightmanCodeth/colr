#include <pebble.h>

//Main window (where everything goes)
static Window *s_main_window;
//Some text (the time)
static TextLayer *s_time_layer;
//Font
static GFont s_time_font;

unsigned concatenate(unsigned x, unsigned y) {
    unsigned pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;        
}

//Create a TickTimerService to get the time
static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  static char s_time_text[] = "000000";

  strftime(s_time_text, sizeof(s_time_text), "%I%M%S", tick_time);
  text_layer_set_text(s_time_layer, s_time_text);
  int hour = tick_time->tm_hour;
  int min = tick_time->tm_min;
  int sec = tick_time->tm_sec;  
  window_set_background_color(s_main_window, GColorFromRGB(hour + 100, min + 100, sec + 100));
}

static void main_window_load(Window *window) {
  //Get information about the window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Create the text layer
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  
  //Layout
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TERMINUS_BOLD_24));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
  
  //BG
  int hour = current_time->tm_hour;
  int min = current_time->tm_min;
  int sec = current_time->tm_sec;  
  window_set_background_color(window, GColorFromRGB(255, 0, 0));
  
  //Add it as a child to the window
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
}

static void main_window_unload(Window *window) {
  //Destroy the text layer
  text_layer_destroy(s_time_layer);
  //Unload GFont
  fonts_unload_custom_font(s_time_font);
}

static void init() {  
  //Main window
  s_main_window = window_create();
  
  //Elements inside the window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
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