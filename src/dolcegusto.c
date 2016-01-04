/*
 * dolcegusto.c
 * 
	Dolce Gusto Time by lordlouis is licensed under a 
	Creative Commons Attribution-ShareAlike 4.0 International License.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pebble.h>
#define TOTAL_LEVELS 7

static int s_count = 1;
static char s_selected_seconds_text_buffer[3];
static int s_seconds;
static int s_seconds_array[TOTAL_LEVELS] = {8, 10, 15, 21, 24, 28, 38};
static AppTimer *timer;
static const uint16_t timer_interval_ms = 1000;
int running;

// Graphics
static GBitmap *s_icon_up_bitmap;
static GBitmap *s_icon_down_bitmap;
static GBitmap *s_icon_play_bitmap;
static GBitmap *s_icon_pause_bitmap;
static GDrawCommandImage *s_level_bg_image;
static GDrawCommandImage *s_levels_image [TOTAL_LEVELS];
static Layer *s_canvas_layer;
static Layer *s_selected_seconds_layer;
static Window *s_main_window;
static ActionBarLayer *action_bar;
static StatusBarLayer *status_bar;

/**
 * Update the canvas layer of the level graphics
 */
static void update_proc(Layer *layer, GContext *ctx) {
	// Set the origin offset from the context for drawing the image
	GPoint origin = GPoint(10, 4);

	// Draw the GDrawCommandImage to the GContext
	gdraw_command_image_draw(ctx, s_level_bg_image, origin);

	for (int i=0; i < s_count; i ++){
		gdraw_command_image_draw(ctx, s_levels_image[i], origin);
	}
}

/**
 * Draws the timer canvas with multiple layers, simulating borders on the font
 */
void draw_seconds_layer(Layer *this_layer, GContext *ctx){
	GRect bounds = layer_get_bounds(this_layer);
	graphics_context_set_text_color(ctx, GColorBlack);

	graphics_draw_text(ctx, s_selected_seconds_text_buffer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49), GRect(0,8, bounds.size.w-15,bounds.size.h), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, s_selected_seconds_text_buffer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49), GRect(4,8, bounds.size.w-15,bounds.size.h), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, s_selected_seconds_text_buffer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49), GRect(0,12, bounds.size.w-15,bounds.size.h), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, s_selected_seconds_text_buffer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49), GRect(4,12, bounds.size.w-15,bounds.size.h), GTextOverflowModeFill, GTextAlignmentCenter, NULL);

	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_draw_text(ctx, s_selected_seconds_text_buffer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49), GRect(2,10, bounds.size.w-15,bounds.size.h), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

/**
 * Updates the time of the layer counter
 */
static void show_time(void) {
	snprintf(s_selected_seconds_text_buffer, sizeof(s_selected_seconds_text_buffer), "%d", s_seconds);
	layer_mark_dirty(s_selected_seconds_layer);
}

/**
 * Update de canvas of the level graphics
 */
static void update_level_graphics(int val, int *arr, int size){
    for (int i=0; i < size; i++) {
        if (arr[i] == val){
			s_count = i+1;
			layer_mark_dirty(s_canvas_layer);
		}
    }
    if (val == 0){
		s_count = val;
		layer_mark_dirty(s_canvas_layer);
	}
}

/**
 * The loop of the counter
 */
static void timer_callback(void *data) {
	s_seconds--;
	if (s_seconds <= 0) {
		s_seconds = 0;
		running = 0;
		vibes_long_pulse();
		action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, s_icon_up_bitmap, true);
		action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, s_icon_down_bitmap, true);
		action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_SELECT, s_icon_play_bitmap, true);
	}
	else {
		timer = app_timer_register(timer_interval_ms, timer_callback, NULL);
	}
	update_level_graphics(s_seconds, s_seconds_array, TOTAL_LEVELS);
	show_time();
}

/**
 * Update level of the counter
 */
static void update_level(int level){
	if (!running) {
		s_count += level;
		if (s_count < 1){
			s_count = 1;
		}
		else if (s_count > TOTAL_LEVELS){
			s_count = TOTAL_LEVELS;
		}
		s_seconds = s_seconds_array[s_count -1];
		strcpy(s_selected_seconds_text_buffer, "");
	}
}

/**
 * Add level to the counter
 */
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  update_level(1);
  layer_mark_dirty(s_canvas_layer);
}

/**
 * Remove level to the counter
 */
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  update_level(-1);
  layer_mark_dirty(s_canvas_layer);
}

/**
 * Start - Pause - Resume the counter 
 */
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	if ((running) || (s_seconds == 0)) {
		app_timer_cancel(timer);
		if (!running) {
			running = 0;
			s_seconds = 0;
			vibes_long_pulse();
			show_time();
		}
		else{
			vibes_short_pulse();
		}
		running = 0;
		action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, s_icon_up_bitmap, true);
		action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, s_icon_down_bitmap, true);
		action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_SELECT, s_icon_play_bitmap, true);
		return;
	}
	else {
		running = 1;
		action_bar_layer_clear_icon(action_bar, BUTTON_ID_UP);
		action_bar_layer_clear_icon(action_bar, BUTTON_ID_DOWN);
		action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_SELECT, s_icon_pause_bitmap, true);
	}
	timer = app_timer_register(timer_interval_ms, timer_callback, NULL);
	show_time();
	vibes_double_pulse();
}

/**
 * Register the ClickHandlers
 */
static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect window_bounds = layer_get_bounds(window_layer);

	update_level(0);

	status_bar = status_bar_layer_create();
	int16_t width = window_bounds.size.w - PBL_IF_ROUND_ELSE(0, ACTION_BAR_WIDTH);
	GRect frame = GRect(0, 0, width, STATUS_BAR_LAYER_HEIGHT);
	status_bar_layer_set_colors(status_bar, GColorClear, GColorWhite);
	layer_set_frame(status_bar_layer_get_layer(status_bar), frame);
	layer_add_child(window_layer, status_bar_layer_get_layer(status_bar));

	// Icons  
	s_icon_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
	s_icon_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);
	s_icon_play_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
	s_icon_pause_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
	// Initialize the action bar:
	action_bar = action_bar_layer_create();
	#ifdef PBL_PLATFORM_APLITE
		action_bar_layer_set_background_color(action_bar, GColorWhite);
	#endif
	// Associate the action bar with the window:
	action_bar_layer_add_to_window(action_bar, window);
	// Set the click config provider:
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
	// Set the icons:
	action_bar_layer_set_icon_press_animation(action_bar, BUTTON_ID_UP, ActionBarLayerIconPressAnimationMoveUp);
	action_bar_layer_set_icon_press_animation(action_bar, BUTTON_ID_DOWN, ActionBarLayerIconPressAnimationMoveDown);
	action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, s_icon_up_bitmap, true);
	action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, s_icon_down_bitmap, true);
	action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_SELECT, s_icon_play_bitmap, true);

	// Create the canvas Layer
	s_canvas_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(32, 0), PBL_IF_ROUND_ELSE(20, 10) , width, window_bounds.size.h));

	// Set the LayerUpdateProc
	layer_set_update_proc(s_canvas_layer, update_proc);

	// Add to parent Window
	//Graphics
	layer_add_child(window_layer, s_canvas_layer);

	// Create output TextLayer
	s_selected_seconds_layer = layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), width+10, 80));
	layer_set_update_proc(s_selected_seconds_layer, draw_seconds_layer);
	layer_add_child(window_layer, s_selected_seconds_layer);
}

static void main_window_unload(Window *window) {
	// Destroy Layers
	layer_destroy(s_selected_seconds_layer);
	layer_destroy(s_canvas_layer);

	//Graphics
	gbitmap_destroy(s_icon_up_bitmap);
	gbitmap_destroy(s_icon_down_bitmap);
	gbitmap_destroy(s_icon_play_bitmap);
	gbitmap_destroy(s_icon_pause_bitmap);

	gdraw_command_image_destroy(s_level_bg_image);
	for (int i=0; i < TOTAL_LEVELS -1; i ++){
		gdraw_command_image_destroy(s_levels_image[i]);
	}
}

static void init() {
	// Create main Window
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	window_set_click_config_provider(s_main_window, click_config_provider);

	//Graphics
	s_level_bg_image = gdraw_command_image_create_with_resource(RESOURCE_ID_LEVEL_BG_IMAGE);
	s_levels_image[0] = gdraw_command_image_create_with_resource(RESOURCE_ID_LEVEL_1_IMAGE);
	s_levels_image[1] = gdraw_command_image_create_with_resource(RESOURCE_ID_LEVEL_2_IMAGE);
	s_levels_image[2] = gdraw_command_image_create_with_resource(RESOURCE_ID_LEVEL_3_IMAGE);
	s_levels_image[3] = gdraw_command_image_create_with_resource(RESOURCE_ID_LEVEL_4_IMAGE);
	s_levels_image[4] = gdraw_command_image_create_with_resource(RESOURCE_ID_LEVEL_5_IMAGE);
	s_levels_image[5] = gdraw_command_image_create_with_resource(RESOURCE_ID_LEVEL_6_IMAGE);
	s_levels_image[6] = gdraw_command_image_create_with_resource(RESOURCE_ID_LEVEL_7_IMAGE);

	window_set_background_color(s_main_window, GColorBlueMoon);
	window_stack_push(s_main_window, true);
}

static void deinit() {
	// Destroy main Window
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}