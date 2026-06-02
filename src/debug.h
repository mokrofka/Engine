#pragma once

struct DebugWindow {
  v2 pos;
  v2 size;
  b32 toggle_fullscreen;
  b32 fullscreen;
  b32 open;
  ImGuiWindowFlags flags;
};

struct DebugProfWindow {
  DebugWindow win;
  ScrollState root_scroll_state;
  ScrollState frames_scroll_state;
  ScrollState launch_time_scroll_state;
  ScrollState mem_scroll_state;
};

struct DebugState {
  DebugProfWindow prof_win;
  DebugWindow game_win;
  b32 imgui_demo_open;
  ImFont* font;
};

void debug_window_apply_state(DebugWindow& win);
void debug_window_track_state(DebugWindow& win);
void debug_window_toggle_fullscreen(DebugWindow& win);
void debug_init();
void debug_update();
void debug_game();
void debug_prof_view();

