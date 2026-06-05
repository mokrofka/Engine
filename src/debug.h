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

