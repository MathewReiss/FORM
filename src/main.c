#include <pebble.h>

Window *my_window;
Layer *time_layer;
GBitmap *bg_left, *bg_right, *colon, *digits[4];

const int DIGIT_IDS[] = {
	RESOURCE_ID_DIGIT_0,
	RESOURCE_ID_DIGIT_1,
	RESOURCE_ID_DIGIT_2,
	RESOURCE_ID_DIGIT_3,
	RESOURCE_ID_DIGIT_4,
	RESOURCE_ID_DIGIT_5,
	RESOURCE_ID_DIGIT_6,
	RESOURCE_ID_DIGIT_7,
	RESOURCE_ID_DIGIT_8,
	RESOURCE_ID_DIGIT_9
};

int CURRENT_THEME = 0;
#define BLUE 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define GRAY 4
#define MUZEI 5
#define SAVED_THEME 10

#ifdef PBL_COLOR
void replace_gbitmap_color(GColor color_to_replace, GColor replace_with_color, GColor accent_to_replace, GColor new_accent, GBitmap *im){
	GColor *current_palette = gbitmap_get_palette(im);

	for(int i = 0; i < 4; i++){
		if ((color_to_replace.argb & 0x3F)==(current_palette[i].argb & 0x3F)){
			current_palette[i].argb = (current_palette[i].argb & 0xC0)| (replace_with_color.argb & 0x3F);
		}
		else if((accent_to_replace.argb & 0x3F)==(current_palette[i].argb & 0x3F)){
			current_palette[i].argb = (current_palette[i].argb & 0xC0)| (new_accent.argb & 0x3F);
		}
	}
}

void custom_palette(GColor right, GColor left, GColor accent, GColor accent2){
	if(CURRENT_THEME != MUZEI){
		replace_gbitmap_color(GColorWhite, left, GColorWhite, GColorWhite, bg_left);
		replace_gbitmap_color(GColorBlack, right, GColorWhite, GColorWhite, bg_right);
	}
	replace_gbitmap_color(GColorBlack, accent, GColorLightGray, accent2, digits[0]);
	replace_gbitmap_color(GColorBlack, accent, GColorLightGray, accent2, digits[1]);
	replace_gbitmap_color(GColorBlack, accent, GColorWhite, GColorWhite, colon);
	replace_gbitmap_color(GColorBlack, accent, GColorLightGray, accent2, digits[2]);
	replace_gbitmap_color(GColorBlack, accent, GColorLightGray, accent2, digits[3]);	
}

void make_blue(){
	custom_palette(GColorBlue, GColorBlueMoon, GColorPictonBlue, GColorCyan);
}

void make_red(){
	custom_palette(GColorDarkCandyAppleRed, GColorRoseVale, GColorSunsetOrange, GColorRed);
}

void make_green(){
	custom_palette(GColorDarkGreen, GColorMidnightGreen, GColorCadetBlue, GColorTiffanyBlue);
}

void make_yellow(){
	custom_palette(GColorChromeYellow, GColorRajah, GColorOrange, GColorRed);
}

void make_gray(){
	custom_palette(GColorBlack, GColorDarkGray, GColorLightGray, GColorLightGray);
}

void make_muzei(){
	CURRENT_THEME = MUZEI;
	gbitmap_destroy(bg_left);
	gbitmap_destroy(bg_right);
	gbitmap_destroy(colon);
	bg_left = gbitmap_create_with_resource(RESOURCE_ID_BG_SPECIAL_LEFT);
	bg_right = gbitmap_create_with_resource(RESOURCE_ID_BG_SPECIAL_RIGHT);
	colon = gbitmap_create_with_resource(RESOURCE_ID_COLON);
	make_blue();
}

void refresh_bitmaps(){
	gbitmap_destroy(colon);
	gbitmap_destroy(bg_left);
	gbitmap_destroy(bg_right);
	colon = gbitmap_create_with_resource(RESOURCE_ID_COLON);
	bg_left = gbitmap_create_with_resource(RESOURCE_ID_BG_LEFT);
	bg_right = gbitmap_create_with_resource(RESOURCE_ID_BG_RIGHT);
}

void clear_bitmaps(){
	gbitmap_destroy(digits[0]);
	gbitmap_destroy(digits[1]);
	gbitmap_destroy(digits[2]);
	gbitmap_destroy(digits[3]);
}

void restore_saved_theme(){
	switch(CURRENT_THEME){
		case BLUE: make_blue(); break;
		case GREEN: make_green(); break;
		case RED: make_red(); break;
		case YELLOW: make_yellow(); break;
		case GRAY: make_gray(); break;
		case MUZEI: make_muzei(); break;
		default: make_blue(); break;
	}
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed){
	int display_hour = tick_time->tm_hour;
	if(!clock_is_24h_style() && (display_hour == 0 || display_hour > 12)){
		display_hour = display_hour % 12;
		display_hour = display_hour ? display_hour : 12;
	}
	
	clear_bitmaps();
	
	digits[0] = gbitmap_create_with_resource(DIGIT_IDS[display_hour/10]);
	digits[1] = gbitmap_create_with_resource(DIGIT_IDS[display_hour%10]);
	digits[2] = gbitmap_create_with_resource(DIGIT_IDS[tick_time->tm_min/10]);
	digits[3] = gbitmap_create_with_resource(DIGIT_IDS[tick_time->tm_min%10]);
		
	restore_saved_theme();
	
	layer_mark_dirty(time_layer);
}

static void draw_time(Layer *layer, GContext *ctx){
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	
	graphics_draw_bitmap_in_rect(ctx, bg_right, GRect(0,0,144,168));
	graphics_draw_bitmap_in_rect(ctx, bg_left, GRect(0,0,144,168));

	graphics_draw_bitmap_in_rect(ctx, digits[0], GRect(2, 69, 30, 30));
	graphics_draw_bitmap_in_rect(ctx, digits[1], GRect(34, 69, 30, 30));
	graphics_draw_bitmap_in_rect(ctx, colon, GRect(66, 69, 12, 30));
	graphics_draw_bitmap_in_rect(ctx, digits[2], GRect(80, 69, 30, 30));
	graphics_draw_bitmap_in_rect(ctx, digits[3], GRect(112, 69, 30, 30));
}

void inbox(DictionaryIterator *iter, void *context){
	Tuple *t = dict_read_first(iter);
	CURRENT_THEME = atoi(t->value->cstring);
	if(CURRENT_THEME != MUZEI){
		refresh_bitmaps();
	}
	time_t now = time(NULL);
	struct tm *tick_time = localtime(&now);
	handle_minute_tick(tick_time, MINUTE_UNIT);
	persist_write_int(SAVED_THEME, CURRENT_THEME);
}

void handle_init(void) {
	app_message_open(32,32);
	app_message_register_inbox_received(inbox);
	
	
	bg_left = gbitmap_create_with_resource(RESOURCE_ID_BG_LEFT);
	bg_right = gbitmap_create_with_resource(RESOURCE_ID_BG_RIGHT);
	colon = gbitmap_create_with_resource(RESOURCE_ID_COLON);

	digits[0] = gbitmap_create_with_resource(RESOURCE_ID_DIGIT_1);
	digits[1] = gbitmap_create_with_resource(RESOURCE_ID_DIGIT_0);
	digits[2] = gbitmap_create_with_resource(RESOURCE_ID_DIGIT_1);
	digits[3] = gbitmap_create_with_resource(RESOURCE_ID_DIGIT_0);

	if(persist_exists(SAVED_THEME)){
		CURRENT_THEME = persist_read_int(SAVED_THEME);
		restore_saved_theme();
	}
	else{
		make_blue();
	}
		
	time_layer = layer_create(GRect(0,0,144,168));

	layer_set_update_proc(time_layer, draw_time);

	my_window = window_create();
	layer_add_child(window_get_root_layer(my_window), time_layer);
	window_stack_push(my_window, true);

	time_t now = time(NULL);
	struct tm *tick_time = localtime(&now);
	handle_minute_tick(tick_time, MINUTE_UNIT);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	app_message_deregister_callbacks();
	
	gbitmap_destroy(colon);
	gbitmap_destroy(bg_left);
	gbitmap_destroy(bg_right);
	gbitmap_destroy(digits[0]);
	gbitmap_destroy(digits[1]);
	gbitmap_destroy(digits[2]);
	gbitmap_destroy(digits[3]);
	
	layer_destroy(time_layer);
	window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}

#endif