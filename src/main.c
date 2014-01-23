#include "pebble.h"

static Window *window;
static TextLayer *time_layer; 
static TextLayer *battery_layer;
static GBitmap *Anonymous;
static BitmapLayer* Anonymous_Layer;
static GBitmap *bt_connected;
static BitmapLayer* bt_connected_layer;

int WIDTH = 144;
int HEIGHT = 168;
int ICON_WIDTH = 14;
int ICON_HEIGHT = 14;

static void handle_battery(BatteryChargeState charge_state) {
	static char battery_text[] = "100%";

	if (charge_state.is_charging) {
		snprintf(battery_text, sizeof(battery_text), "%d%%  charging", charge_state.charge_percent);
	} else {
		snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
	}
	text_layer_set_text(battery_layer, battery_text);
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
	static char time_text[] = "00:00:00";
	if (clock_is_24h_style() == true){
		strftime(time_text, sizeof(time_text), "%T", tick_time);
	} else {
		strftime(time_text, sizeof(time_text), "%I:%M:%S %p", tick_time);
	}
	text_layer_set_text(time_layer, time_text);

	handle_battery(battery_state_service_peek());
}

static void handle_bluetooth(bool connected) {
	if (connected == 1){
		bt_connected = gbitmap_create_with_resource(RESOURCE_ID_BT_CONNECTED);
		bitmap_layer_set_bitmap(bt_connected_layer, bt_connected);
	} else {
		bt_connected = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCONNECTED);
		bitmap_layer_set_bitmap(bt_connected_layer, bt_connected);
	}
}

static void do_init(void) {

	window = window_create();
	window_stack_push(window, true);
	window_set_background_color(window, GColorBlack);

	Layer *root_layer = window_get_root_layer(window);
	GRect frame = layer_get_frame(root_layer);

	/* Background */
	Anonymous = gbitmap_create_with_resource(RESOURCE_ID_BG_ANONYMOUS);
	Anonymous_Layer = bitmap_layer_create(GRect(0, 0, WIDTH, HEIGHT));
	bitmap_layer_set_background_color(Anonymous_Layer, GColorBlack);
	bitmap_layer_set_bitmap(Anonymous_Layer, Anonymous);
	layer_add_child(root_layer, bitmap_layer_get_layer(Anonymous_Layer));

	/* Time block */
	time_layer = text_layer_create(GRect(0, 52, frame.size.w, 34));
	text_layer_set_text_color(time_layer, GColorBlack);
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD ));
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

	/* Bluetooth block */
	bt_connected = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCONNECTED);
	bt_connected_layer = bitmap_layer_create(GRect(WIDTH - ICON_WIDTH, 1, ICON_WIDTH, ICON_HEIGHT));
	bitmap_layer_set_background_color(bt_connected_layer, GColorBlack);
	layer_add_child(root_layer, bitmap_layer_get_layer(bt_connected_layer));

	/* Battery block */
	battery_layer = text_layer_create(GRect(2, -2, frame.size.w, 16 ));
	text_layer_set_text_color(battery_layer, GColorWhite);
	text_layer_set_background_color(battery_layer, GColorClear);
	text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(battery_layer, GTextAlignmentLeft);
	text_layer_set_text(battery_layer, "100% charged");

	/* Init blocks */
	time_t now = time(NULL);
	struct tm *current_time = localtime(&now);
	handle_second_tick(current_time, SECOND_UNIT);

	tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
	battery_state_service_subscribe(&handle_battery);

	bool connected = bluetooth_connection_service_peek();
	handle_bluetooth(connected);
	bluetooth_connection_service_subscribe(&handle_bluetooth);

	layer_add_child(root_layer, text_layer_get_layer(time_layer));
	layer_add_child(root_layer, text_layer_get_layer(battery_layer));
}

static void do_deinit(void) {
	gbitmap_destroy(Anonymous);
	bitmap_layer_destroy(Anonymous_Layer);
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	text_layer_destroy(time_layer);
	text_layer_destroy(battery_layer);
	window_destroy(window);
}

int main(void) {
	do_init();
	app_event_loop();
	do_deinit();
}