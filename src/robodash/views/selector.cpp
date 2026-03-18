#include "robodash/views/selector.hpp"

#include "robodash/apix.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

namespace {

const char* kSaveFile = "/usd/rd_auton.txt";

struct ScopedMutex {
    explicit ScopedMutex(pros::Mutex& m) : mutex(m) { mutex.take(TIMEOUT_MAX); }
    ~ScopedMutex() { mutex.give(); }
    pros::Mutex& mutex;
};

std::string to_upper_copy(const std::string& text) {
    std::string out = text;
    for (char& c : out) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return out;
}

void apply_button_enabled_style(lv_obj_t* btn, lv_obj_t* icon, bool enabled, lv_color_t active_color) {
    if (enabled) {
        lv_obj_set_style_border_color(btn, active_color, 0);
        lv_obj_set_style_border_opa(btn, LV_OPA_27, 0);
        lv_obj_set_style_text_color(icon, active_color, 0);
    } else {
        lv_obj_set_style_border_color(btn, color_selector_disabled_border, 0);
        lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(icon, color_selector_disabled_icon, 0);
    }
}

} // namespace

rd::Selector::Selector(std::vector<routine_t> autons, pros::Controller* controller)
    : Selector("Auton Selector", autons, controller) {}

rd::Selector::Selector(std::string selector_name, std::vector<routine_t> autons, pros::Controller* controller)
    : view(nullptr),
      controller(controller),
      left_panel(nullptr),
      right_panel(nullptr),
      grid(nullptr),
      selected_name_label(nullptr),
      selected_sub_label(nullptr),
      alliance_red_btn(nullptr),
      alliance_blue_btn(nullptr),
      alliance_red_txt(nullptr),
      alliance_blue_txt(nullptr),
      transport_state_label(nullptr),
      transport_bar(nullptr),
      timer_elapsed_label(nullptr),
      timer_suffix_label(nullptr),
      play_btn(nullptr),
      pause_btn(nullptr),
      stop_btn(nullptr),
      play_icon(nullptr),
      pause_icon(nullptr),
      stop_icon(nullptr),
      transport_timer(nullptr),
      name(selector_name),
      selected_routine(nullptr),
      alliance(Alliance::NONE),
      transport_state(TransportState::IDLE),
      selected_index(-1),
      saved_elapsed(0),
      elapsed(0),
      start_tick(0) {
    routines = autons;

    for (routine_t& routine : routines) {
        if (routine.id.empty()) {
            routine.id = to_routine_id(routine);
        }
        if (routine.sub.empty()) {
            routine.sub = "No description";
        }
    }

    view = rd_view_create(name.c_str());
    lv_obj_t* root = view->obj;
    lv_obj_set_style_bg_color(root, color_selector_screen_bg, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    create_left_panel();
    create_right_panel();

    if (pros::usd::is_installed()) sd_load();
    refresh_selection_styles();
}

void rd::Selector::create_left_panel() {
    lv_obj_t* root = view->obj;

    left_panel = lv_obj_create(root);
    lv_obj_set_pos(left_panel, 0, 0);
    lv_obj_set_size(left_panel, 148, 240);
    lv_obj_set_style_bg_color(left_panel, color_selector_panel_bg, 0);
    lv_obj_set_style_border_width(left_panel, 1, 0);
    lv_obj_set_style_border_side(left_panel, LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_style_border_color(left_panel, color_selector_inactive_border, 0);
    lv_obj_set_style_radius(left_panel, 0, 0);
    lv_obj_set_style_pad_all(left_panel, 0, 0);
    lv_obj_clear_flag(left_panel, LV_OBJ_FLAG_SCROLLABLE);

    const int pad = 12;
    int y = 12;

    lv_obj_t* selected_hdr = lv_label_create(left_panel);
    lv_label_set_text(selected_hdr, "SELECTED");
    lv_obj_set_pos(selected_hdr, pad, y);
    lv_obj_set_style_text_font(selected_hdr, &lv_font_montserrat_8, 0);
    lv_obj_set_style_text_color(selected_hdr, color_selector_section_header, 0);
    lv_obj_set_style_text_letter_space(selected_hdr, 3, 0);

    y += 20;
    selected_name_label = lv_label_create(left_panel);
    lv_label_set_text(selected_name_label, "-");
    lv_obj_set_pos(selected_name_label, pad, y);
    lv_obj_set_style_text_font(selected_name_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(selected_name_label, color_selector_inactive_border, 0);

    y += 24;
    selected_sub_label = lv_label_create(left_panel);
    lv_label_set_text(selected_sub_label, "");
    lv_obj_set_pos(selected_sub_label, pad, y);
    lv_obj_set_style_text_font(selected_sub_label, &lv_font_montserrat_8, 0);
    lv_obj_set_style_text_color(selected_sub_label, color_selector_selected_sub, 0);
    lv_obj_add_flag(selected_sub_label, LV_OBJ_FLAG_HIDDEN);

    y += 18;
    lv_obj_t* divider1 = lv_obj_create(left_panel);
    lv_obj_set_pos(divider1, 0, y);
    lv_obj_set_size(divider1, 148, 1);
    lv_obj_set_style_bg_color(divider1, color_selector_divider, 0);
    lv_obj_set_style_border_width(divider1, 0, 0);
    lv_obj_set_style_radius(divider1, 0, 0);

    y += 11;
    lv_obj_t* alliance_hdr = lv_label_create(left_panel);
    lv_label_set_text(alliance_hdr, "ALLIANCE");
    lv_obj_set_pos(alliance_hdr, pad, y);
    lv_obj_set_style_text_font(alliance_hdr, &lv_font_montserrat_8, 0);
    lv_obj_set_style_text_color(alliance_hdr, color_selector_section_header, 0);

    y += 14;
    alliance_red_btn = lv_btn_create(left_panel);
    lv_obj_set_pos(alliance_red_btn, pad, y);
    lv_obj_set_size(alliance_red_btn, 55, 32);
    lv_obj_set_style_radius(alliance_red_btn, 4, 0);
    lv_obj_set_style_border_width(alliance_red_btn, 1, 0);
    lv_obj_set_style_bg_color(alliance_red_btn, color_selector_card_bg, 0);
    lv_obj_set_style_border_color(alliance_red_btn, color_selector_inactive_border, 0);
    lv_obj_set_user_data(alliance_red_btn, this);
    lv_obj_add_event_cb(alliance_red_btn, alliance_cb, LV_EVENT_CLICKED, reinterpret_cast<void*>(1));

    alliance_red_txt = lv_label_create(alliance_red_btn);
    lv_label_set_text(alliance_red_txt, "RED");
    lv_obj_center(alliance_red_txt);
    lv_obj_set_style_text_font(alliance_red_txt, &lv_font_montserrat_10, 0);

    alliance_blue_btn = lv_btn_create(left_panel);
    lv_obj_set_pos(alliance_blue_btn, pad + 61, y);
    lv_obj_set_size(alliance_blue_btn, 55, 32);
    lv_obj_set_style_radius(alliance_blue_btn, 4, 0);
    lv_obj_set_style_border_width(alliance_blue_btn, 1, 0);
    lv_obj_set_style_bg_color(alliance_blue_btn, color_selector_card_bg, 0);
    lv_obj_set_style_border_color(alliance_blue_btn, color_selector_inactive_border, 0);
    lv_obj_set_user_data(alliance_blue_btn, this);
    lv_obj_add_event_cb(alliance_blue_btn, alliance_cb, LV_EVENT_CLICKED, reinterpret_cast<void*>(2));

    alliance_blue_txt = lv_label_create(alliance_blue_btn);
    lv_label_set_text(alliance_blue_txt, "BLUE");
    lv_obj_center(alliance_blue_txt);
    lv_obj_set_style_text_font(alliance_blue_txt, &lv_font_montserrat_10, 0);

    y += 42;
    lv_obj_t* divider2 = lv_obj_create(left_panel);
    lv_obj_set_pos(divider2, 0, y);
    lv_obj_set_size(divider2, 148, 1);
    lv_obj_set_style_bg_color(divider2, color_selector_divider, 0);
    lv_obj_set_style_border_width(divider2, 0, 0);
    lv_obj_set_style_radius(divider2, 0, 0);

    y += 11;
    lv_obj_t* transport_hdr = lv_label_create(left_panel);
    lv_label_set_text(transport_hdr, "TRANSPORT");
    lv_obj_set_pos(transport_hdr, pad, y);
    lv_obj_set_style_text_font(transport_hdr, &lv_font_montserrat_8, 0);
    lv_obj_set_style_text_color(transport_hdr, color_selector_section_header, 0);

    transport_state_label = lv_label_create(left_panel);
    lv_label_set_text(transport_state_label, "IDLE");
    lv_obj_set_style_text_font(transport_state_label, &lv_font_montserrat_8, 0);
    lv_obj_set_style_text_color(transport_state_label, color_selector_idle, 0);
    lv_obj_align_to(transport_state_label, transport_hdr, LV_ALIGN_OUT_RIGHT_MID, 36, 0);

    y += 16;
    transport_bar = lv_bar_create(left_panel);
    lv_obj_set_pos(transport_bar, pad, y);
    lv_obj_set_size(transport_bar, 124, 3);
    lv_obj_set_style_bg_color(transport_bar, color_selector_divider, 0);
    lv_obj_set_style_bg_color(transport_bar, color_selector_idle, LV_PART_INDICATOR);
    lv_bar_set_range(transport_bar, 0, 100);

    y += 11;
    timer_elapsed_label = lv_label_create(left_panel);
    lv_label_set_text(timer_elapsed_label, "00:00.0");
    lv_obj_set_pos(timer_elapsed_label, pad, y);
    lv_obj_set_style_text_font(timer_elapsed_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(timer_elapsed_label, color_selector_idle, 0);

    timer_suffix_label = lv_label_create(left_panel);
    lv_label_set_text(timer_suffix_label, " / 00:15.0");
    lv_obj_set_style_text_font(timer_suffix_label, &lv_font_montserrat_8, 0);
    lv_obj_set_style_text_color(timer_suffix_label, color_selector_unselected_sub, 0);
    lv_obj_align_to(timer_suffix_label, timer_elapsed_label, LV_ALIGN_OUT_RIGHT_BOTTOM, 4, -3);

    y += 31;
    play_btn = lv_btn_create(left_panel);
    lv_obj_set_pos(play_btn, pad, y);
    lv_obj_set_size(play_btn, 39, 36);
    lv_obj_set_style_radius(play_btn, 4, 0);
    lv_obj_set_style_bg_color(play_btn, color_selector_dark_btn_bg, 0);
    lv_obj_set_style_border_width(play_btn, 1, 0);
    lv_obj_set_user_data(play_btn, this);
    lv_obj_add_event_cb(play_btn, play_cb, LV_EVENT_CLICKED, nullptr);
    play_icon = lv_label_create(play_btn);
    lv_label_set_text(play_icon, LV_SYMBOL_PLAY);
    lv_obj_center(play_icon);

    pause_btn = lv_btn_create(left_panel);
    lv_obj_set_pos(pause_btn, pad + 43, y);
    lv_obj_set_size(pause_btn, 39, 36);
    lv_obj_set_style_radius(pause_btn, 4, 0);
    lv_obj_set_style_bg_color(pause_btn, color_selector_dark_btn_bg, 0);
    lv_obj_set_style_border_width(pause_btn, 1, 0);
    lv_obj_set_user_data(pause_btn, this);
    lv_obj_add_event_cb(pause_btn, pause_cb, LV_EVENT_CLICKED, nullptr);
    pause_icon = lv_label_create(pause_btn);
    lv_label_set_text(pause_icon, LV_SYMBOL_PAUSE);
    lv_obj_center(pause_icon);

    stop_btn = lv_btn_create(left_panel);
    lv_obj_set_pos(stop_btn, pad + 86, y);
    lv_obj_set_size(stop_btn, 39, 36);
    lv_obj_set_style_radius(stop_btn, 4, 0);
    lv_obj_set_style_bg_color(stop_btn, color_selector_dark_btn_bg, 0);
    lv_obj_set_style_border_width(stop_btn, 1, 0);
    lv_obj_set_user_data(stop_btn, this);
    lv_obj_add_event_cb(stop_btn, stop_cb, LV_EVENT_CLICKED, nullptr);
    stop_icon = lv_label_create(stop_btn);
    lv_label_set_text(stop_icon, LV_SYMBOL_STOP);
    lv_obj_center(stop_icon);
}

void rd::Selector::create_right_panel() {
    lv_obj_t* root = view->obj;

    right_panel = lv_obj_create(root);
    lv_obj_set_pos(right_panel, 148, 0);
    lv_obj_set_size(right_panel, 332, 240);
    lv_obj_set_style_bg_color(right_panel, color_selector_panel_bg, 0);
    lv_obj_set_style_border_width(right_panel, 0, 0);
    lv_obj_set_style_radius(right_panel, 0, 0);
    lv_obj_set_style_pad_all(right_panel, 0, 0);
    lv_obj_clear_flag(right_panel, LV_OBJ_FLAG_SCROLLABLE);

    grid = lv_obj_create(right_panel);
    lv_obj_set_pos(grid, 10, 10);
    lv_obj_set_size(grid, 312, 220);
    lv_obj_set_style_bg_opa(grid, LV_OPA_0, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_set_style_pad_column(grid, 5, 0);
    lv_obj_set_style_pad_row(grid, 5, 0);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);

    const int n = static_cast<int>(routines.size());
    const int cols = (n <= 2) ? 1 : ((n <= 4) ? 2 : 3);
    const int rows = (n + cols - 1) / cols;

    static std::vector<lv_coord_t> col_dsc;
    static std::vector<lv_coord_t> row_dsc;
    col_dsc.assign(static_cast<size_t>(cols + 1), LV_GRID_TEMPLATE_LAST);
    row_dsc.assign(static_cast<size_t>(rows + 1), LV_GRID_TEMPLATE_LAST);
    for (int i = 0; i < cols; i++) col_dsc[static_cast<size_t>(i)] = LV_GRID_FR(1);
    for (int i = 0; i < rows; i++) row_dsc[static_cast<size_t>(i)] = LV_GRID_FR(1);
    lv_obj_set_grid_dsc_array(grid, col_dsc.data(), row_dsc.data());

    const int pad = (n <= 2) ? 14 : ((n <= 4) ? 11 : 9);
    const lv_font_t* name_font = (n <= 2) ? &lv_font_montserrat_16 : ((n <= 4) ? &lv_font_montserrat_14 : &lv_font_montserrat_12);

    cards.resize(static_cast<size_t>(n));
    for (int i = 0; i < n; i++) {
        CardRefs card;
        card.btn = lv_btn_create(grid);
        lv_obj_set_style_radius(card.btn, 6, 0);
        lv_obj_set_style_bg_color(card.btn, color_selector_card_bg, 0);
        lv_obj_set_style_border_width(card.btn, 1, 0);
        lv_obj_set_style_border_color(card.btn, color_selector_card_border, 0);
        lv_obj_set_style_pad_all(card.btn, pad, 0);
        lv_obj_set_style_pad_row(card.btn, 4, 0);
        lv_obj_set_user_data(card.btn, this);
        lv_obj_add_event_cb(card.btn, card_cb, LV_EVENT_CLICKED, reinterpret_cast<void*>(static_cast<intptr_t>(i)));

        const int row = i / cols;
        const int col = i % cols;
        int span = 1;
        if ((n % cols == 1) && (i == n - 1)) span = cols;
        lv_obj_set_grid_cell(card.btn, LV_GRID_ALIGN_STRETCH, col, span, LV_GRID_ALIGN_STRETCH, row, 1);

        lv_obj_set_layout(card.btn, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(card.btn, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card.btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

        card.name = lv_label_create(card.btn);
        lv_obj_set_width(card.name, lv_pct(100));
        lv_label_set_long_mode(card.name, LV_LABEL_LONG_WRAP);
        lv_label_set_text(card.name, routines[static_cast<size_t>(i)].name.c_str());
        lv_obj_set_style_text_font(card.name, name_font, 0);

        card.sub = lv_label_create(card.btn);
        lv_obj_set_width(card.sub, lv_pct(100));
        lv_label_set_long_mode(card.sub, LV_LABEL_LONG_WRAP);
        lv_label_set_text(card.sub, routines[static_cast<size_t>(i)].sub.c_str());
        lv_obj_set_style_text_font(card.sub, &lv_font_montserrat_8, 0);

        cards[static_cast<size_t>(i)] = card;
    }
}

lv_color_t rd::Selector::current_accent() const {
    if (alliance == Alliance::BLUE) return color_selector_blue;
    return color_selector_red;
}

int rd::Selector::current_duration() const {
    if (selected_index < 0 || selected_index >= static_cast<int>(routines.size())) return 15000;
    return static_cast<int>(routines[static_cast<size_t>(selected_index)].duration_ms);
}

std::string rd::Selector::to_routine_id(const routine_t& routine) const {
    std::string id;
    for (char c : routine.name) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            id.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else if (c == ' ' || c == '-' || c == '_') {
            if (id.empty() || id.back() == '_') continue;
            id.push_back('_');
        }
    }
    if (id.empty()) id = "auton";
    return id;
}

void rd::Selector::update_selected_panel() {
    if (selected_index < 0 || selected_routine == nullptr) {
        lv_label_set_text(selected_name_label, "-");
        lv_obj_set_style_text_color(selected_name_label, color_selector_inactive_border, 0);
        lv_obj_add_flag(selected_sub_label, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    const std::string upper = to_upper_copy(selected_routine->name);
    lv_label_set_text(selected_name_label, upper.c_str());
    lv_obj_set_style_text_color(selected_name_label, color_selector_selected_text, 0);

    lv_label_set_text(selected_sub_label, selected_routine->sub.c_str());
    lv_obj_clear_flag(selected_sub_label, LV_OBJ_FLAG_HIDDEN);
}

void rd::Selector::apply_card_unselected(int idx) {
    if (idx < 0 || idx >= static_cast<int>(cards.size())) return;
    CardRefs& card = cards[static_cast<size_t>(idx)];

    lv_obj_set_style_bg_color(card.btn, color_selector_card_bg, 0);
    lv_obj_set_style_bg_opa(card.btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card.btn, color_selector_card_border, 0);
    lv_obj_set_style_text_color(card.name, color_selector_unselected_name, 0);
    lv_obj_set_style_text_color(card.sub, color_selector_unselected_sub, 0);
    lv_obj_set_style_text_opa(card.sub, LV_OPA_COVER, 0);

    if (card.top_bar != nullptr) {
        lv_obj_del(card.top_bar);
        card.top_bar = nullptr;
    }
}

void rd::Selector::apply_card_selected(int idx) {
    if (idx < 0 || idx >= static_cast<int>(cards.size())) return;
    CardRefs& card = cards[static_cast<size_t>(idx)];
    const lv_color_t accent = current_accent();

    lv_obj_set_style_bg_color(card.btn, accent, 0);
    lv_obj_set_style_bg_opa(card.btn, LV_OPA_15, 0);
    lv_obj_set_style_border_color(card.btn, accent, 0);
    lv_obj_set_style_text_color(card.name, color_selector_selected_text, 0);
    lv_obj_set_style_text_color(card.sub, color_selector_selected_text, 0);
    lv_obj_set_style_text_opa(card.sub, LV_OPA_20, 0);

    if (card.top_bar == nullptr) {
        card.top_bar = lv_obj_create(card.btn);
        lv_obj_set_size(card.top_bar, lv_pct(100), 2);
        lv_obj_align(card.top_bar, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_radius(card.top_bar, 0, 0);
        lv_obj_set_style_border_width(card.top_bar, 0, 0);
        lv_obj_clear_flag(card.top_bar, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(card.top_bar, LV_OBJ_FLAG_CLICKABLE);
    }
    lv_obj_set_style_bg_color(card.top_bar, accent, 0);
}

void rd::Selector::refresh_alliance_buttons() {
    if (alliance == Alliance::RED) {
        lv_obj_set_style_border_color(alliance_red_btn, color_selector_red, 0);
        lv_obj_set_style_bg_color(alliance_red_btn, color_selector_red, 0);
        lv_obj_set_style_bg_opa(alliance_red_btn, LV_OPA_15, 0);
        lv_obj_set_style_text_color(alliance_red_txt, color_selector_selected_text, 0);
    } else {
        lv_obj_set_style_border_color(alliance_red_btn, color_selector_inactive_border, 0);
        lv_obj_set_style_bg_color(alliance_red_btn, color_selector_card_bg, 0);
        lv_obj_set_style_bg_opa(alliance_red_btn, LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(alliance_red_txt, color_selector_inactive_text, 0);
    }

    if (alliance == Alliance::BLUE) {
        lv_obj_set_style_border_color(alliance_blue_btn, color_selector_blue, 0);
        lv_obj_set_style_bg_color(alliance_blue_btn, color_selector_blue, 0);
        lv_obj_set_style_bg_opa(alliance_blue_btn, LV_OPA_15, 0);
        lv_obj_set_style_text_color(alliance_blue_txt, color_selector_selected_text, 0);
    } else {
        lv_obj_set_style_border_color(alliance_blue_btn, color_selector_inactive_border, 0);
        lv_obj_set_style_bg_color(alliance_blue_btn, color_selector_card_bg, 0);
        lv_obj_set_style_bg_opa(alliance_blue_btn, LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(alliance_blue_txt, color_selector_inactive_text, 0);
    }
}

void rd::Selector::update_transport_buttons() {
    const bool can_play = (selected_routine != nullptr) && (transport_state != TransportState::RUNNING);
    const bool can_pause = transport_state == TransportState::RUNNING;
    const bool can_stop = transport_state != TransportState::IDLE;

    apply_button_enabled_style(play_btn, play_icon, can_play, color_selector_green);
    apply_button_enabled_style(pause_btn, pause_icon, can_pause, color_selector_amber);
    apply_button_enabled_style(stop_btn, stop_icon, can_stop, color_selector_red);
}

void rd::Selector::update_transport_labels() {
    const int duration = std::max(1, current_duration());
    const bool overtime = elapsed > static_cast<uint32_t>(duration);

    lv_color_t state_color = color_selector_idle;
    const char* state_text = "IDLE";
    if (transport_state == TransportState::RUNNING && overtime) {
        state_color = color_selector_red;
        state_text = "OT";
    } else if (transport_state == TransportState::RUNNING) {
        state_color = color_selector_green;
        state_text = "RUN";
    } else if (transport_state == TransportState::PAUSED) {
        state_color = color_selector_amber;
        state_text = "PAUSE";
    } else if (transport_state == TransportState::DONE) {
        state_color = color_selector_red;
        state_text = "DONE";
    }

    lv_label_set_text(transport_state_label, state_text);
    lv_obj_set_style_text_color(transport_state_label, state_color, 0);
    lv_obj_set_style_text_color(timer_elapsed_label, state_color, 0);
    lv_obj_set_style_bg_color(transport_bar, state_color, LV_PART_INDICATOR);

    const uint32_t shown_ms = overtime ? (elapsed - duration) : elapsed;
    const uint32_t ds = shown_ms / 100;
    const uint32_t m = ds / 600;
    const uint32_t s = (ds / 10) % 60;
    const uint32_t d = ds % 10;

    char timer_text[24];
    std::snprintf(timer_text, sizeof(timer_text), overtime ? "-%02lu:%02lu.%lu" : "%02lu:%02lu.%lu",
                  static_cast<unsigned long>(m), static_cast<unsigned long>(s), static_cast<unsigned long>(d));
    lv_label_set_text(timer_elapsed_label, timer_text);

    const uint32_t dur_ds = static_cast<uint32_t>(duration) / 100;
    const uint32_t dur_m = dur_ds / 600;
    const uint32_t dur_s = (dur_ds / 10) % 60;
    char suffix_text[24];
    std::snprintf(suffix_text, sizeof(suffix_text), " / %02lu:%02lu.0", static_cast<unsigned long>(dur_m), static_cast<unsigned long>(dur_s));
    lv_label_set_text(timer_suffix_label, suffix_text);

    int pct = static_cast<int>((std::min(elapsed, static_cast<uint32_t>(duration)) * 100U) / static_cast<uint32_t>(duration));
    if (overtime) pct = 100;
    lv_bar_set_value(transport_bar, pct, LV_ANIM_OFF);
}

void rd::Selector::refresh_selection_styles() {
    for (int i = 0; i < static_cast<int>(cards.size()); i++) {
        if (i == selected_index) apply_card_selected(i);
        else apply_card_unselected(i);
    }
    update_selected_panel();
    refresh_alliance_buttons();
    update_transport_labels();
    update_transport_buttons();
}

void rd::Selector::stop_timer_task() {
    if (transport_timer != nullptr) {
        lv_timer_del(transport_timer);
        transport_timer = nullptr;
    }
}

void rd::Selector::stop_transport() {
    stop_timer_task();
    elapsed = 0;
    saved_elapsed = 0;
    transport_state = TransportState::IDLE;
    lv_bar_set_value(transport_bar, 0, LV_ANIM_OFF);
    update_transport_labels();
    update_transport_buttons();
}

void rd::Selector::mark_done() {
    if (transport_state == TransportState::RUNNING) {
        elapsed = saved_elapsed + lv_tick_elaps(start_tick);
        saved_elapsed = elapsed;
    }
    stop_timer_task();
    transport_state = TransportState::DONE;
    update_transport_labels();
    update_transport_buttons();
}

void rd::Selector::select_index(int idx, bool save_selection) {
    if (idx < 0 || idx >= static_cast<int>(routines.size())) return;

    if (selected_index == idx) {
        stop_transport();
        if (save_selection) sd_save();
        return;
    }

    if (selected_index >= 0) apply_card_unselected(selected_index);

    selected_index = idx;
    selected_routine = &routines[static_cast<size_t>(selected_index)];
    apply_card_selected(selected_index);
    update_selected_panel();
    run_callbacks();

    stop_transport();
    if (save_selection) sd_save();
}

void rd::Selector::sd_save() {
    FILE* save_file = std::fopen(kSaveFile, "a");
    if (save_file == nullptr) return;
    std::fclose(save_file);

    save_file = std::fopen(kSaveFile, "r");
    if (save_file == nullptr) return;

    std::fseek(save_file, 0L, SEEK_END);
    int file_size = static_cast<int>(std::ftell(save_file));
    std::rewind(save_file);

    std::vector<char> new_text(static_cast<size_t>(std::max(file_size + 1, 1)), '\0');
    char line[256];
    char saved_selector[256];

    while (std::fgets(line, sizeof(line), save_file)) {
        saved_selector[0] = '\0';
        std::sscanf(line, "%[^:]", saved_selector);
        if (std::strcmp(saved_selector, name.c_str()) == 0) continue;
        std::strncat(new_text.data(), line, new_text.size() - std::strlen(new_text.data()) - 1);
    }

    std::fclose(save_file);
    save_file = std::fopen(kSaveFile, "w");
    if (save_file == nullptr) return;

    std::fputs(new_text.data(), save_file);
    if (selected_routine != nullptr) {
        char file_data[384];
        std::snprintf(file_data, sizeof(file_data), "%s: %s\n", name.c_str(), selected_routine->id.c_str());
        std::fputs(file_data, save_file);
    }
    std::fclose(save_file);
}

void rd::Selector::sd_load() {
    FILE* save_file = std::fopen(kSaveFile, "r");
    if (save_file == nullptr) return;

    char line[256];
    char saved_selector[256] = {0};
    char saved_id[256] = {0};

    while (std::fgets(line, sizeof(line), save_file)) {
        saved_selector[0] = '\0';
        saved_id[0] = '\0';
        std::sscanf(line, "%[^:]: %[^\n\0]", saved_selector, saved_id);
        if (std::strcmp(saved_selector, name.c_str()) == 0) break;
    }

    std::fclose(save_file);
    if (std::strcmp(saved_id, "") == 0 || std::strcmp(saved_selector, name.c_str()) != 0) return;

    for (int i = 0; i < static_cast<int>(routines.size()); i++) {
        if (routines[static_cast<size_t>(i)].id == saved_id) {
            select_index(i, false);
            break;
        }
    }
}

void rd::Selector::run_callbacks() {
    for (select_action_t callback : select_callbacks) {
        if (selected_routine == nullptr) callback(std::nullopt);
        else callback(*selected_routine);
    }
}

void rd::Selector::run_auton() {
    if (selected_routine == nullptr) return;
    selected_routine->action();
    ScopedMutex lock(state_mutex);
    mark_done();
}

std::optional<rd::Selector::routine_t> rd::Selector::get_auton() {
    if (selected_routine == nullptr) return std::nullopt;
    return *selected_routine;
}

void rd::Selector::on_select(rd::Selector::select_action_t callback) { select_callbacks.push_back(callback); }

void rd::Selector::next_auton(bool wrap_around) {
    ScopedMutex lock(state_mutex);
    if (routines.empty()) return;
    if (selected_index < 0) {
        select_index(0, true);
        refresh_selection_styles();
        return;
    }
    int next = selected_index + 1;
    if (next >= static_cast<int>(routines.size())) {
        if (!wrap_around) return;
        next = 0;
    }
    select_index(next, true);
    refresh_selection_styles();
}

void rd::Selector::prev_auton(bool wrap_around) {
    ScopedMutex lock(state_mutex);
    if (routines.empty()) return;
    if (selected_index < 0) {
        select_index(0, true);
        refresh_selection_styles();
        return;
    }
    int prev = selected_index - 1;
    if (prev < 0) {
        if (!wrap_around) return;
        prev = static_cast<int>(routines.size()) - 1;
    }
    select_index(prev, true);
    refresh_selection_styles();
}

void rd::Selector::focus() { rd_view_focus(view); }

void rd::Selector::update() {
    // Controller support intentionally disabled while keeping API compatibility.
}

rd::Selector* rd::Selector::from_event(lv_event_t* event) {
    lv_obj_t* target = lv_event_get_target(event);
    return static_cast<rd::Selector*>(lv_obj_get_user_data(target));
}

void rd::Selector::card_cb(lv_event_t* event) {
    rd::Selector* self = from_event(event);
    if (self == nullptr) return;
    const int idx = static_cast<int>(reinterpret_cast<intptr_t>(lv_event_get_user_data(event)));
    ScopedMutex lock(self->state_mutex);
    self->select_index(idx, true);
    self->refresh_selection_styles();
}

void rd::Selector::alliance_cb(lv_event_t* event) {
    rd::Selector* self = from_event(event);
    if (self == nullptr) return;
    const int value = static_cast<int>(reinterpret_cast<intptr_t>(lv_event_get_user_data(event)));

    ScopedMutex lock(self->state_mutex);
    if (value == 1) self->alliance = Alliance::RED;
    else if (value == 2) self->alliance = Alliance::BLUE;
    else self->alliance = Alliance::NONE;
    self->refresh_selection_styles();
}

void rd::Selector::play_cb(lv_event_t* event) {
    rd::Selector* self = from_event(event);
    if (self == nullptr) return;

    ScopedMutex lock(self->state_mutex);
    if (self->selected_routine == nullptr) return;
    if (self->transport_state == TransportState::RUNNING) return;
    if (self->transport_state == TransportState::DONE) {
        self->elapsed = 0;
        self->saved_elapsed = 0;
    }

    self->start_tick = lv_tick_get();
    if (self->transport_timer == nullptr) {
        self->transport_timer = lv_timer_create(transport_timer_cb, 50, self);
    }
    self->transport_state = TransportState::RUNNING;
    self->update_transport_labels();
    self->update_transport_buttons();
}

void rd::Selector::pause_cb(lv_event_t* event) {
    rd::Selector* self = from_event(event);
    if (self == nullptr) return;

    ScopedMutex lock(self->state_mutex);
    if (self->transport_state != TransportState::RUNNING) return;
    self->elapsed = self->saved_elapsed + lv_tick_elaps(self->start_tick);
    self->saved_elapsed = self->elapsed;
    self->stop_timer_task();
    self->transport_state = TransportState::PAUSED;
    self->update_transport_labels();
    self->update_transport_buttons();
}

void rd::Selector::stop_cb(lv_event_t* event) {
    rd::Selector* self = from_event(event);
    if (self == nullptr) return;

    ScopedMutex lock(self->state_mutex);
    self->stop_transport();
}

void rd::Selector::transport_timer_cb(lv_timer_t* timer) {
    rd::Selector* self = static_cast<rd::Selector*>(timer->user_data);
    if (self == nullptr) return;

    ScopedMutex lock(self->state_mutex);
    if (self->transport_state != TransportState::RUNNING) return;
    self->elapsed = self->saved_elapsed + lv_tick_elaps(self->start_tick);
    self->update_transport_labels();
    self->update_transport_buttons();
}
