/**
 * @file selector.hpp
 * @brief Robodash Selector
 * @ingroup selector
 */

#pragma once
#include "robodash/api.h"
#include "pros/misc.hpp"
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace rd {

class Selector {
  public:
    typedef std::function<void()> routine_action_t;

    // Keep the first fields compatible with the previous initializer pattern used in main.cpp.
    typedef struct routine {
        std::string name;
        routine_action_t action;
        std::string img = "";
        int color_hue = -1;

        // Extended metadata for the custom selector.
        std::string sub = "";
        uint32_t duration_ms = 15000;
        std::string id = "";
    } routine_t;

    typedef std::function<void(std::optional<routine_t>)> select_action_t;

    Selector(std::string name, std::vector<routine_t> autons, pros::Controller* controller = nullptr);
    Selector(std::vector<routine_t> autons, pros::Controller* controller = nullptr);

    void run_auton();
    std::optional<routine_t> get_auton();
    void on_select(select_action_t callback);

    void next_auton(bool wrap_around = true);
    void prev_auton(bool wrap_around = true);

    void focus();
    void update();

  private:
    enum class Alliance {
        NONE,
        RED,
        BLUE,
    };

    enum class TransportState {
        IDLE,
        RUNNING,
        PAUSED,
        DONE,
    };

    struct CardRefs {
        lv_obj_t* btn = nullptr;
        lv_obj_t* name = nullptr;
        lv_obj_t* sub = nullptr;
        lv_obj_t* top_bar = nullptr;
    };

    rd_view_t* view;
    pros::Controller* controller;

    lv_obj_t* left_panel;
    lv_obj_t* right_panel;
    lv_obj_t* grid;

    lv_obj_t* selected_name_label;
    lv_obj_t* selected_sub_label;

    lv_obj_t* alliance_red_btn;
    lv_obj_t* alliance_blue_btn;
    lv_obj_t* alliance_red_txt;
    lv_obj_t* alliance_blue_txt;

    lv_obj_t* transport_state_label;
    lv_obj_t* transport_bar;
    lv_obj_t* timer_elapsed_label;
    lv_obj_t* timer_suffix_label;

    lv_obj_t* play_btn;
    lv_obj_t* pause_btn;
    lv_obj_t* stop_btn;
    lv_obj_t* play_icon;
    lv_obj_t* pause_icon;
    lv_obj_t* stop_icon;

    lv_timer_t* transport_timer;

    std::string name;
    std::vector<rd::Selector::routine_t> routines;
    std::vector<rd::Selector::select_action_t> select_callbacks;
    rd::Selector::routine_t* selected_routine;

    std::vector<CardRefs> cards;

    Alliance alliance;
    TransportState transport_state;

    int selected_index;
    uint32_t saved_elapsed;
    uint32_t elapsed;
    uint32_t start_tick;

    pros::Mutex state_mutex;

    void sd_save();
    void sd_load();
    void run_callbacks();

    void create_left_panel();
    void create_right_panel();

    void stop_timer_task();
    void stop_transport();
    void refresh_alliance_buttons();
    void refresh_selection_styles();
    void update_transport_buttons();
    void update_transport_labels();
    void update_selected_panel();
    void apply_card_unselected(int idx);
    void apply_card_selected(int idx);
    void select_index(int idx, bool save_selection);
    void mark_done();

    lv_color_t current_accent() const;
    int current_duration() const;
    std::string to_routine_id(const routine_t& routine) const;

    static Selector* from_event(lv_event_t* event);
    static void card_cb(lv_event_t* event);
    static void alliance_cb(lv_event_t* event);
    static void play_cb(lv_event_t* event);
    static void pause_cb(lv_event_t* event);
    static void stop_cb(lv_event_t* event);
    static void transport_timer_cb(lv_timer_t* timer);
};

} // namespace rd
