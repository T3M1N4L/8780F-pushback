#include "liblvgl/misc/lv_color.h"

// ================================= Colors ================================= //

int hue = 200;

lv_color_t color_bg;
lv_color_t color_border;
lv_color_t color_shade;
lv_color_t color_primary;
lv_color_t color_primary_dark;
lv_color_t color_text;
lv_color_t color_red;

lv_color_t color_bar;
lv_color_t color_bar_dark;
lv_color_t color_bar_outline;

lv_color_t color_selector_screen_bg;
lv_color_t color_selector_panel_bg;
lv_color_t color_selector_divider;
lv_color_t color_selector_card_border;
lv_color_t color_selector_inactive_border;
lv_color_t color_selector_selected_text;
lv_color_t color_selector_unselected_name;
lv_color_t color_selector_selected_sub;
lv_color_t color_selector_unselected_sub;
lv_color_t color_selector_section_header;
lv_color_t color_selector_disabled_icon;
lv_color_t color_selector_red;
lv_color_t color_selector_blue;
lv_color_t color_selector_green;
lv_color_t color_selector_amber;
lv_color_t color_selector_idle;
lv_color_t color_selector_dark_btn_bg;
lv_color_t color_selector_card_bg;
lv_color_t color_selector_disabled_border;
lv_color_t color_selector_inactive_text;

lv_color_t color_position_accent;
lv_color_t color_position_text_dim;
lv_color_t color_position_text_med;
lv_color_t color_position_text_bright;
lv_color_t color_position_x;
lv_color_t color_position_y;
lv_color_t color_position_theta;

lv_color_t color_pid_text_dim;
lv_color_t color_pid_text_med;
lv_color_t color_pid_text_bright;
lv_color_t color_pid_card_bg;
lv_color_t color_pid_lat;
lv_color_t color_pid_ang;
lv_color_t color_pid_kp;
lv_color_t color_pid_ki;
lv_color_t color_pid_kd;
lv_color_t color_pid_windup;
lv_color_t color_pid_active_text;

lv_color_t color_motor_vel;
lv_color_t color_motor_pwr;
lv_color_t color_motor_cur;
lv_color_t color_motor_temp_green;
lv_color_t color_motor_temp_yellow;
lv_color_t color_motor_temp_red;
lv_color_t color_motor_trq;
lv_color_t color_motor_text_dim;
lv_color_t color_motor_text_med;
lv_color_t color_motor_card_bg;
lv_color_t color_motor_progress_track;

void _init_colors() {
	color_bg = lv_color_hsv_to_rgb(0, 0, 0);
	color_border = lv_color_hsv_to_rgb(0, 0, 20);
	color_shade = lv_color_hsv_to_rgb(0, 0, 20);
	color_primary = lv_color_hsv_to_rgb(266, 66, 100);
	color_primary_dark = lv_color_hsv_to_rgb(266, 66, 80);
	color_text = lv_color_hsv_to_rgb(227, 7, 100);
	color_bar = lv_color_hsv_to_rgb(0, 0, 0);
	color_bar_dark = lv_color_hsv_to_rgb(0, 0, 0);
	color_bar_outline = lv_color_hsv_to_rgb(0, 0, 20);
	color_red = lv_color_hsv_to_rgb(0, 75, 100);

	color_selector_screen_bg = lv_color_hex(0x000000);
	color_selector_panel_bg = lv_color_hex(0x050505);
	color_selector_divider = lv_color_hex(0x0f0f0f);
	color_selector_card_border = lv_color_hex(0x131313);
	color_selector_inactive_border = lv_color_hex(0x1a1a1a);
	color_selector_selected_text = lv_color_hex(0xffffff);
	color_selector_unselected_name = lv_color_hex(0x555555);
	color_selector_selected_sub = lv_color_hex(0x2a2a2a);
	color_selector_unselected_sub = lv_color_hex(0x1a1a1a);
	color_selector_section_header = lv_color_hex(0x1e1e1e);
	color_selector_disabled_icon = lv_color_hex(0x181818);
	color_selector_red = lv_color_hex(0xf87171);
	color_selector_blue = lv_color_hex(0x60a5fa);
	color_selector_green = lv_color_hex(0x22c55e);
	color_selector_amber = lv_color_hex(0xf59e0b);
	color_selector_idle = lv_color_hex(0x252525);
	color_selector_dark_btn_bg = lv_color_hex(0x070707);
	color_selector_card_bg = lv_color_hex(0x080808);
	color_selector_disabled_border = lv_color_hex(0x111111);
	color_selector_inactive_text = lv_color_hex(0x252525);

	color_position_accent = lv_color_hex(0x9333ea);
	color_position_text_dim = lv_color_hex(0x444444);
	color_position_text_med = lv_color_hex(0x888888);
	color_position_text_bright = lv_color_hex(0xffffff);
	color_position_x = lv_color_hex(0xef4444);
	color_position_y = lv_color_hex(0x22c55e);
	color_position_theta = lv_color_hex(0xa78bfa);

	color_pid_text_dim = lv_color_hex(0x444444);
	color_pid_text_med = lv_color_hex(0x888888);
	color_pid_text_bright = lv_color_hex(0xffffff);
	color_pid_card_bg = lv_color_hex(0x000000);
	color_pid_lat = lv_color_hex(0x22c55e);
	color_pid_ang = lv_color_hex(0x0ea5e9);
	color_pid_kp = lv_color_hex(0xa78bfa);
	color_pid_ki = lv_color_hex(0x0ea5e9);
	color_pid_kd = lv_color_hex(0x22c55e);
	color_pid_windup = lv_color_hex(0xef4444);
	color_pid_active_text = lv_color_hex(0x000000);

	color_motor_vel = lv_color_hex(0x22c55e);
	color_motor_pwr = lv_color_hex(0x0ea5e9);
	color_motor_cur = lv_color_hex(0xeab308);
	color_motor_temp_green = lv_color_hex(0x22c55e);
	color_motor_temp_yellow = lv_color_hex(0xeab308);
	color_motor_temp_red = lv_color_hex(0xef4444);
	color_motor_trq = lv_color_hex(0xa78bfa);
	color_motor_text_dim = lv_color_hex(0x444444);
	color_motor_text_med = lv_color_hex(0x555555);
	color_motor_card_bg = lv_color_hex(0x000000);
	color_motor_progress_track = lv_color_hex(0x000000);
}
