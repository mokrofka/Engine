#pragma once
#include "types.h"

enum ScrollType {
  ScrollType_Default,
  ScrollType_PowClamp,
};

struct ScrollState {
  v2 offset;
  v2 scale;
  f32 scale_level;
};

struct DebugWindow {
  v2 pos;
  v2 size;
  b32 toggle_fullscreen;
  b32 fullscreen;
  b32 open;
  ImGuiWindowFlags flags;
};

Rng2 debug_window_get_rect(DebugWindow win);

enum ProfTabActive {
  ProfileTabActive_Root,
  ProfileTabActive_Frames,
  ProfileTabActive_Time,
  ProfileTabActive_LaunchTime,
  ProfileTabActive_Memory,
};

struct ProfColors {
  v4 work;
  v4 sleep;
  v4 job;
  v4 async;
  v4 selected;
  v4 hovered;
  v4 border;
  v4 text;
  v4 text_dim;
  v4 current_frame;
  v4 frame_ok;     // < 16ms
  v4 frame_warn;   // 16-20ms
  v4 frame_bad;    // > 20ms
  v4 mem_used;
  v4 mem_committed;
  v4 mem_cap;
};

struct DebugProfWindow {
  DebugWindow win;
  ScrollState root_scroll_state;
  ScrollState frames_scroll_state;
  ScrollState launch_time_scroll_state;
  ScrollState mem_scroll_state;
  ProfTabActive active_tab;
  ProfTabActive future_active_tab;
  f32 frame_avg_time;
  f32 frame_min_time;
  f32 frame_max_time;
  ProfColors colors;
};

struct DebugState {
  DebugProfWindow prof_win;
  DebugWindow game_win;
  b32 imgui_demo_open;
  ImFont* font;
};

struct ImGui_DrawList {
  ImDrawList* draw;
};

ImGui_DrawList imgui_get_window_drawlist();
void imgui_draw_rect(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0, f32 thickness = 1);
void imgui_draw_rect_filled(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0);
void imgui_draw_push_clip_rect(ImGui_DrawList draw, Rng2 rect);
void imgui_draw_pop_clip_rect(ImGui_DrawList draw);
void imgui_draw_line(ImGui_DrawList draw, v2 p0, v2 p1, v4 col, f32 thickness = 1);
void imgui_draw_text(ImGui_DrawList draw, v2 pos, v4 col, String fmt, ...);
void imgui_draw_text(ImGui_DrawList draw, ImFont* font, f32 font_size, v2 pos, v4 col, String fmt, ...);
void imgui_text(String fmt, ...);
v2 imgui_calc_text_size(String str);
void imgui_begin_tab_item(String str);

void debug_window_apply_state(DebugWindow& win);
void debug_window_track_state(DebugWindow& win);
void debug_window_toggle_fullscreen(DebugWindow& win);
void debug_init();
void debug_update();
void debug_game();
void debug_prof_view();

