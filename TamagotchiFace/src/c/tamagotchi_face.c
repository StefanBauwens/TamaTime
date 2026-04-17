#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

#define VRAM_SIZE (64 + 13)
#define BYTES_PER_LINE 32
#define BITS_PER_BYTE 8

#define MEM_BUFFER_SIZE 464
#define LCD_WIDTH 32
#define LCD_HEIGHT 16

#include <pebble.h>

static Window *s_main_window;
static BitmapLayer *s_background_layer;
static Layer *s_screen_layer;
static Layer *s_icons_layer;
static TextLayer *s_text_layer; //TODO use as temp show the time? DO NOT use fonts for tellling time since pixels change depending on platform
//static GFont s_lcd_font;

// Bitmaps
static GBitmap *s_bitmap_bg;
static GBitmap *s_bitmap_icon1;
static GBitmap *s_bitmap_icon2;
static GBitmap *s_bitmap_icon3;
static GBitmap *s_bitmap_icon4;
static GBitmap *s_bitmap_icon5;
static GBitmap *s_bitmap_icon6;
static GBitmap *s_bitmap_icon7;
static GBitmap *s_bitmap_icon8;

uint8_t memory[MEM_BUFFER_SIZE];
static int8_t s_selectedIcon = -1; // -1 is none, 0-6 says what icon
static bool s_showingAttentionIcon = false;
static bool s_js_ready;
static bool s_pixelsChanged = false;

static bool s_screen_buffer[LCD_HEIGHT][LCD_WIDTH] = {{0}};

static void Quit()
{
  window_stack_pop_all(false);
}

static void Message(const char * text) // Write message to screen
{
    layer_set_hidden((Layer *)s_screen_layer, true); // hide screen layer so we can read text
    text_layer_set_text(s_text_layer, text);
}


void set_screen_to_last_state(uint8_t *fullRam) { // gets screen data from memory and sets it to the screen
    uint8_t vram[VRAM_SIZE];

    memcpy(vram, fullRam + 320, VRAM_SIZE);

    uint8_t adjustedVram[VRAM_SIZE];
    int idx = 0;

    // Helper macro to copy forward
    #define COPY_RANGE(start, end) \
        for (int i = (start); i < (end); i++) { \
            adjustedVram[idx++] = vram[i]; \
        }

    // Helper macro to copy reversed
    #define COPY_RANGE_REVERSE(start, end) \
        for (int i = (end) - 1; i >= (start); i--) { \
            adjustedVram[idx++] = vram[i]; \
        }

    // Replicating your JS slices
    COPY_RANGE(0, 8);
    COPY_RANGE(9, 17);
    COPY_RANGE_REVERSE(29, 37);
    COPY_RANGE_REVERSE(20, 28);
    COPY_RANGE(40, 48);
    COPY_RANGE(49, 57);
    COPY_RANGE_REVERSE(69, 77);
    COPY_RANGE_REVERSE(60, 68);

    int totalBytes = idx;

    for (int i = 0; i < totalBytes; i++) {
        uint8_t byte = adjustedVram[i];

        int x = (i % BYTES_PER_LINE);
        int baseY = (i / BYTES_PER_LINE) * BITS_PER_BYTE;

        for (int bitIndex = 0; bitIndex < BITS_PER_BYTE; bitIndex++) {
          int bit = (byte >> bitIndex) & 1;  
          //int bit = (byte >> (7 - bitIndex)) & 1;

            int y = baseY + bitIndex;

            s_screen_buffer[y][x] = bit;
        }
    }

    s_pixelsChanged = true;

    #undef COPY_RANGE
    #undef COPY_RANGE_REVERSE
}

// Button presses
/*static void on_button_back(ClickRecognizerRef recognizer, void *context) //back
{
}*/

static void click_config_provider(void *context) { //TODO use a button to click to see the time or change view? though can be annoying
  // subscribe to button presses here  
  //window_single_click_subscribe(BUTTON_ID_BACK, on_button_back);
}

// Handles drawing icons layers
static void icons_update_proc(Layer *layer, GContext *ctx) {
  // Set the draw color
  graphics_context_set_fill_color(ctx, GColorBlack);

  // Set the compositing mode (GCompOpSet is required for transparency)
  graphics_context_set_compositing_mode(ctx, GCompOpSet);

  if(s_selectedIcon >= 0)
  {
    #if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
    uint8_t xPos = 12 + ((s_selectedIcon%4) * 40); 
    uint8_t yPos = (s_selectedIcon > 3 ? 120 : 0);
    #else
    uint8_t xPos = 12 + ((s_selectedIcon%4) * 32);
    uint8_t yPos = (s_selectedIcon > 3 ? 100 : 0);
    #endif

    GBitmap* selected_icon = s_bitmap_icon7;
    switch(s_selectedIcon)
    {
      case 0:
        selected_icon = s_bitmap_icon1;
        break;
      case 1:
        selected_icon = s_bitmap_icon2;
        break;
      case 2:
        selected_icon = s_bitmap_icon3;
        break;
      case 3:
        selected_icon = s_bitmap_icon4;
        break;
      case 4:
        selected_icon = s_bitmap_icon5;
        break;
      case 5:
        selected_icon = s_bitmap_icon6;
        break;
      case 6:
        selected_icon = s_bitmap_icon7;
        break;
      default:
        selected_icon = s_bitmap_icon7;
        break;
    }

    // Draw selected icon if selected
    #if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
    graphics_draw_bitmap_in_rect(ctx, selected_icon, GRect(xPos, yPos, 27, 22));
    #else
    graphics_draw_bitmap_in_rect(ctx, selected_icon, GRect(xPos, yPos, 22, 18));
    #endif
  }

  // Handle attention icon
  if(s_showingAttentionIcon)
  {
    #if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
    graphics_draw_bitmap_in_rect(ctx, s_bitmap_icon8, GRect(12+(40*3), 120, 27, 22)); 
    #else
    graphics_draw_bitmap_in_rect(ctx, s_bitmap_icon8, GRect(108, 100, 22, 18));
    #endif
  }
}

// Handles drawing screen layer
static void screen_update_proc(Layer *layer, GContext *ctx) { 
  // draw new screen
  graphics_context_set_fill_color(ctx, GColorBlack);

  //draw pixels
  for (size_t h = 0; h < LCD_HEIGHT; h++)
  {
    for (size_t w = 0; w < LCD_WIDTH; w++)
    {
      if (s_screen_buffer[h][w])
      {
        #if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
        graphics_fill_rect(ctx, GRect(w * 5, h * 5, 4, 4), 0, GCornerNone);
        #else
        graphics_fill_rect(ctx, GRect(w * 4, h * 4, 3, 3), 0, GCornerNone);
        #endif
      }
    }
  }
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox received");

  Tuple *ready_tuple_t = dict_find(iter, MESSAGE_KEY_JSReady);

  if(ready_tuple_t && !s_js_ready) {
    // PebbleKit JS is ready! Safe to send messages
    s_js_ready = true;
    //Message("Loading ROM 0%");
  }

  // Handle (error) messages //TODO probably wanna remove?
  Tuple *JSMessage_t = dict_find(iter, MESSAGE_KEY_JSMessage);
  if (JSMessage_t)
  {
    char *jsMessage = JSMessage_t->value->cstring;
    Message(jsMessage);
  }

  // Handle incoming save state
  Tuple *STATEnone_t = dict_find(iter, MESSAGE_KEY_STATEnone); //TODO do we need?
  Tuple *STATEmemory_t = dict_find(iter, MESSAGE_KEY_STATEmemory);
  Tuple *STATEselected_icon_t = dict_find(iter, MESSAGE_KEY_STATEselected_icon);
  Tuple *STATEshowing_attention_icon_t = dict_find(iter, MESSAGE_KEY_STATEshowing_attention_icon);

  if (STATEmemory_t && STATEselected_icon_t && STATEshowing_attention_icon_t)
  {
    //Message("Loading save state...");
    // handle screen
    uint8_t *state_memory = STATEmemory_t->value->data;

    memcpy(memory, state_memory, sizeof(memory));
    set_screen_to_last_state(memory); 
    layer_mark_dirty(s_screen_layer); //Tell the system to redraw screen

    //handle icons
    s_selectedIcon = STATEselected_icon_t->value->int8;
    s_showingAttentionIcon = STATEshowing_attention_icon_t->value->int8;

    layer_mark_dirty(s_icons_layer);
  }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);

  // Create GBitmap for background 
#if defined(PBL_COLOR)
  s_bitmap_bg = gbitmap_create_with_resource(RESOURCE_ID_BG_IMAGE);
#else
  s_bitmap_bg = gbitmap_create_with_resource(RESOURCE_ID_BG_IMAGE_BW);
#endif

  // Create background layer
#if defined(PBL_PLATFORM_CHALK)
  s_background_layer = bitmap_layer_create(GRect(0, 0, 180, 180));
#elif defined(PBL_PLATFORM_GABBRO)
    s_background_layer = bitmap_layer_create(GRect(0, 0, 260, 260));
#elif defined(PBL_PLATFORM_EMERY)
    s_background_layer = bitmap_layer_create(GRect(0, 0, 200, 228));
#else
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
#endif
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_background_layer, s_bitmap_bg);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Create bitmaps for icons
  s_bitmap_icon1 = gbitmap_create_with_resource(RESOURCE_ID_ICON1);
  s_bitmap_icon2 = gbitmap_create_with_resource(RESOURCE_ID_ICON2);
  s_bitmap_icon3 = gbitmap_create_with_resource(RESOURCE_ID_ICON3);
  s_bitmap_icon4 = gbitmap_create_with_resource(RESOURCE_ID_ICON4);
  s_bitmap_icon5 = gbitmap_create_with_resource(RESOURCE_ID_ICON5);
  s_bitmap_icon6 = gbitmap_create_with_resource(RESOURCE_ID_ICON6);
  s_bitmap_icon7 = gbitmap_create_with_resource(RESOURCE_ID_ICON7);
  s_bitmap_icon8 = gbitmap_create_with_resource(RESOURCE_ID_ICON8);

  // Create icons layer
#if defined(PBL_PLATFORM_CHALK)
  s_icons_layer = layer_create(GRect(0+18, 24+6, 144, 146));
#elif defined(PBL_PLATFORM_GABBRO) 
  s_icons_layer = layer_create(GRect(0+45, 60, 180, 183)); 
#elif defined(PBL_PLATFORM_EMERY)
  s_icons_layer = layer_create(GRect(0+15, 44, 180, 183)); 
#else
  s_icons_layer = layer_create(GRect(0, 24, 144, 146));
#endif
  layer_set_update_proc(s_icons_layer, icons_update_proc);

  // Add to window
  layer_add_child(window_layer, s_icons_layer);

  // Create screen Layer
#if defined(PBL_PLATFORM_CHALK)
  s_screen_layer = layer_create(GRect(8+18, 51+6, 128, 64));
#elif defined(PBL_PLATFORM_GABBRO)
  s_screen_layer = layer_create(GRect(50, 92, 160, 80));
#elif defined(PBL_PLATFORM_EMERY)
  s_screen_layer = layer_create(GRect(20, 76, 160, 80));
#else
  s_screen_layer = layer_create(GRect(8, 51, 128, 64));
#endif
  layer_set_update_proc(s_screen_layer, screen_update_proc);

  // Add to window  
  layer_add_child(window_layer, s_screen_layer);

  // Font
  //s_lcd_font    = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SMALL_LCD_9));

  // Create text layer
  #if defined(PBL_PLATFORM_CHALK)
  s_text_layer = text_layer_create(GRect(6+18, 60+6, 128, 50)); 
  #elif defined(PBL_PLATFORM_GABBRO)
  s_text_layer = text_layer_create(GRect(50, 60+46, 158, 50));
  #elif defined(PBL_PLATFORM_EMERY)
  s_text_layer = text_layer_create(GRect(20, 60+30, 158, 50));
  #else   
  s_text_layer = text_layer_create(GRect(6, 60, 128, 50)); 
  #endif
  text_layer_set_background_color(s_text_layer, GColorClear);
  Message("Fake:time:seconds");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void main_window_unload(Window *window) {
  // Destroy backrgound bitmap and its layer
  gbitmap_destroy(s_bitmap_bg);
  bitmap_layer_destroy(s_background_layer);

  // Destroy text layer
  text_layer_destroy(s_text_layer);

  // Unload font
  //fonts_unload_custom_font(s_lcd_font);

  // Destroy icon bitmaps
  gbitmap_destroy(s_bitmap_icon1);
  gbitmap_destroy(s_bitmap_icon2);
  gbitmap_destroy(s_bitmap_icon3);
  gbitmap_destroy(s_bitmap_icon4);
  gbitmap_destroy(s_bitmap_icon5);
  gbitmap_destroy(s_bitmap_icon6);
  gbitmap_destroy(s_bitmap_icon7);
  gbitmap_destroy(s_bitmap_icon8);

  // Destory icons layer
  layer_destroy(s_icons_layer);

  // Destroy screen layer
  layer_destroy(s_screen_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Listen for seconds
  //tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
  //app_message_open(256, 128); 
  app_message_open(2048, 2048); // tested on pebble 2 duo

  // Listen for button events
  window_set_click_config_provider(s_main_window, click_config_provider);
}

static void requestStateFromServer()
{ 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting state from server...");

  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if(result == APP_MSG_OK) {
    // Construct the message
    int value = 1;
    dict_write_int(out_iter, MESSAGE_KEY_RequestState, &value, sizeof(int8_t), false);
    dict_write_end(out_iter);
    // Send this message
    result = app_message_outbox_send();

    // Check the result
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
    else
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "State request sent to phone!");
      //TODO wait for response
    }
  } else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
    Quit();
  }
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}