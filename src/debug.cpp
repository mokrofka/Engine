#include "debug.h"

ImGui_DrawList imgui_get_window_drawlist() {
  ImGui_DrawList res = {
    .draw = ImGui::GetWindowDrawList(),
  };
  return res;
}
void imgui_draw_rect(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding, ImDrawFlags flags, f32 thickness) {
  draw.draw->AddRect(rect.min, rect.max, u32_from_rgba(col), rounding, flags, thickness);
}
void imgui_draw_rect_filled(ImGui_DrawList draw, Rng2 rect, v4 col, f32 rounding, ImDrawFlags flags) {
  draw.draw->AddRectFilled(rect.min, rect.max, u32_from_rgba(col));
}
void imgui_draw_push_clip_rect(ImGui_DrawList draw, Rng2 rect) {
  draw.draw->PushClipRect(rect.min, rect.max, true);
}
void imgui_draw_pop_clip_rect(ImGui_DrawList draw) {
  draw.draw->PopClipRect();
}
void imgui_draw_line(ImGui_DrawList draw, v2 p0, v2 p1, v4 col, f32 thickness) {
  draw.draw->AddLine(p0, p1, u32_from_rgba(col));
}
void imgui_draw_text(ImGui_DrawList draw, v2 pos, v4 col, String fmt, ...) {
  Scratch scratch;
  VaList args;
  va_start(args, fmt);
  String formateted = push_strfv(scratch, fmt, args);
  va_end(args);
  draw.draw->AddText(pos, u32_from_rgba(col), (char*)formateted.str, (char*)(formateted.str + formateted.size));
}
void imgui_draw_text(ImGui_DrawList draw, ImFont* font, f32 font_size, v2 pos, v4 col, String fmt, ...) {
  Scratch scratch;
  VaList args;
  va_start(args, fmt);
  String formateted = push_strfv(scratch, fmt, args);
  va_end(args);
  draw.draw->AddText(font, font_size, pos, u32_from_rgba(col), (char*)formateted.str, (char*)(formateted.str + formateted.size));
}
void imgui_text(String fmt, ...) {
  Scratch scratch;
  VaList args;
  va_start(args, fmt);
  String formateted = push_strfv(scratch, fmt, args);
  va_end(args);
  ImGui::TextUnformatted((char*)formateted.str);
}
v2 imgui_calc_text_size(String str) {
  return ImGui::CalcTextSize((char*)str.str, (char*)str.str+str.size);
}
void imgui_begin_tab_item(String str) {
  Scratch scratch;
  String str_c = push_str_copy(scratch, str);
  ImGui::BeginTabItem((char*)str_c.str);
}

void debug_window_apply_state(DebugWindow& win) {
  if (win.toggle_fullscreen) {
    if (!win.fullscreen) {
      win.fullscreen = true;
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      win.flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    } else {
      win.fullscreen = false;
      ImGui::SetNextWindowPos(win.pos);
      ImGui::SetNextWindowSize(win.size);
      win.flags = NoFlags;
    }
    win.toggle_fullscreen = false;
  }
}

void debug_window_track_state(DebugWindow& win) {
  if (!win.fullscreen) {
    win.pos = ImGui::GetWindowPos();
    win.size = ImGui::GetWindowSize();
  }
}

void debug_window_toggle_fullscreen(DebugWindow& win) {
  win.toggle_fullscreen = 1;
}

void debug_init() {
  DebugState& g = st->debug;
  g.prof_win = {
    .root_scroll_state = scroll_state_make(1),
    .frames_scroll_state = scroll_state_make(1),
    .launch_time_scroll_state = scroll_state_make(1),
    .mem_scroll_state = scroll_state_make(1),
    .win.open = false,
  };
  g.imgui_demo_open = false;
  vk_imgui_init();
  ImGuiIO& io = ImGui::GetIO();
  g.font = io.Fonts->AddFontDefault();
}

void debug_update() {
  DebugState& g = st->debug;
  if (key_pressed(Key_F1)) g.prof_win.win.open = !g.prof_win.win.open;
  if (key_pressed(Key_F2)) g.imgui_demo_open = !g.imgui_demo_open;
  if (key_pressed(Key_F3)) g.game_win.open = !g.game_win.open;

  if (g.imgui_demo_open) ImGui::ShowDemoWindow();

  debug_prof_view();
  debug_game();
}

void debug_game() {
  Scratch scratch;
  GameState& g = st->game;
  DebugWindow& win = st->debug.game_win;
  Camera& cam = g.cam;
  if (win.open) {
    debug_window_apply_state(win);
    ImGui::Begin("Game");

    ImGui::ColorEdit4("color picker", g.color.v, ImGuiColorEditFlags_Float);
    local f32 f = 0;
    local i32 i = 0;
    ImGui::SliderFloat("sliderf32", &f, -20, 20);
    ImGui::SliderInt("slideri32", &i, -20, 20);
    f = wrap_f32(-10, f, 10);
    i = wrap_i32(-10, i, 10);

    if (ImGui::IsWindowHovered()) {
      if (key_pressed(Key_V)) {
        debug_window_toggle_fullscreen(win);
      }
    }

    v3 pos = cam.pos;
    ImGui::Text("entities: %u, static entities: %u", g.entities_count, g.static_entities_count);
    ImGui::Text("Camera: x: %.2f y: %.2f z: %.2f", pos.x, pos.y, pos.z);
    ImGui::DragFloat("speed", &cam.speed, 1);
    {
      imgui_text(push_str_copy(scratch, dumb_struct(scratch, slice(members_of_Camera), &g.cam)));
    }
    {
      Entity& e = get_entity(g.axis_attached_to_cam_id);
      imgui_text(push_str_copy(scratch, dumb_struct(scratch, slice(members_of_Entity), &e, e.flags)));
    }

    if (ImGui::Button("save state")) {
      game_save_state();
    }
    if (ImGui::Button("load state")) {
      game_load_state();
    }
    if (ImGui::Button("clear moving cubes")) {
      Loop (i, g.moving_cubes.count) {
        EntityId e =  g.moving_cubes[i];
        e_free(e);
      }
      array_clear(g.moving_cubes);
    }

    ImGui::End();
  }
}

void debug_prof_view() {
  Scratch scratch;
  DebugState& debug = st->debug;
  ProfState& prof = prof_get();
  DebugProfWindow& prof_win = st->debug.prof_win;

  // Avg, min, max
  ProfFrame prev_frame = prof_get_prev_frame(st->current_frame);
  var anchors = prev_frame.anchors;
  u64 cpu_freq = cpu_frequency();
  u64 tsc_start = prev_frame.frame_time.tsc_start;
  u64 tsc_end = prev_frame.frame_time.tsc_end;
  u64 tsc_elapsed = tsc_end - tsc_start;
  u64 tsc_elapsed_sum = 0;
  u64 tsc_elapsed_max = prof.frames_times[0].tsc_end - prof.frames_times[0].tsc_start;
  u64 tsc_elapsed_min = prof.frames_times[0].tsc_end - prof.frames_times[0].tsc_start;
  for EachElement(i, prof.frames_times) {
    ProfFrameTime frame = prof.frames_times[i];
    u64 elapsed = frame.tsc_end - frame.tsc_start;
    tsc_elapsed_sum += elapsed;
    tsc_elapsed_max = Max(tsc_elapsed_max, elapsed);
    tsc_elapsed_min = Min(tsc_elapsed_min, elapsed);
  }
  prof.frame_avg_time = tsc_to_ms(tsc_elapsed_sum / 120);
  prof.frame_max_time = tsc_to_ms(tsc_elapsed_max);
  prof.frame_min_time = tsc_to_ms(tsc_elapsed_min);

  if (key_pressed(Key_H)) {
    ImGui::SetNextWindowFocus(); 
  }

  if (prof_win.win.open) {
    f32 thread_name_text_size = 20;
    f32 time_bar_text_size = 15;

    debug_window_apply_state(prof_win.win);

    if (ImGui::Begin("Profiler", null, prof_win.win.flags)) {
      debug_window_track_state(prof_win.win);

      Rng2 win_rect = rng2_make(prof_win.win.pos, prof_win.win.size);
      ImGui::PushClipRect(win_rect.min, win_rect.max, false);

      if (key_pressed(Key_1)) prof.future_active_tab = ProfileTabActive_Root;
      if (key_pressed(Key_2)) prof.future_active_tab = ProfileTabActive_Frames;
      if (key_pressed(Key_3)) prof.future_active_tab = ProfileTabActive_Time;
      if (key_pressed(Key_4)) prof.future_active_tab = ProfileTabActive_LaunchTime;
      if (key_pressed(Key_5)) prof.future_active_tab = ProfileTabActive_Memory;
      if (key_pressed(Key_P)) prof.paused = !prof.paused;
      if (ImGui::IsWindowHovered()) {
        if (key_pressed(Key_V)) {
          debug_window_toggle_fullscreen(prof_win.win);
        }
      }

      if (ImGui::BeginTabBar("MyTabBar")) {
        ImGui_DrawList draw = imgui_get_window_drawlist();
        v2 cursor_pos = ImGui::GetCursorScreenPos();
        v2 mouse_pos = os_mouse_get_pos();
        v2 win_pos = ImGui::GetWindowPos();
        v2 avail_size = ImGui::GetWindowSize();
        avail_size.x -= (cursor_pos - win_pos).x * 2;

        enum UI_ItemType {
          UI_ItemType_Bar,
          UI_ItemType_NextThread,
        };
        struct UI_Item {
          UI_ItemType type;
          Rng2 rect;
          ProfAnchor anchor;
        };

        var draw_frame_graph = [&](Slice<Slice<ProfAnchor>> slices, ProfFrameTime time, f32 width_off, ScrollState scroll_state, b32 wrap = false) {
          Scratch scratch;
          var items = darray_make<UI_Item>(scratch);
          Loop (i, slices.count) {
            var anchors = slices[i];

            ///////////////////////////////////
            // Build rect layout
            {
              // ProfFrameTime time = g.frames_times[anchors_idx];
              u64 tsc_start = time.tsc_start;
              u64 tsc_end = time.tsc_end;
              u64 tsc_elapsed = tsc_end - tsc_start;
              Loop(i, anchors.count) {
                ProfAnchor anchor = anchors[i];
                u64 var_tsc_elapsed_incl = anchor.tsc_elapsed_incl;
                u64 var_tsc_start = anchor.tsc_start;
                
                // Handle async anchors
                if (wrap) {
                  if (!anchor.was_poped) {
                    var_tsc_elapsed_incl = tsc_end - anchor.tsc_start;
                    anchor.tsc_elapsed_incl = var_tsc_elapsed_incl;
                  }
                  if (anchor.tsc_start < tsc_start) {
                    var_tsc_start = tsc_start;
                    anchor.tsc_start = var_tsc_start;
                  }
                }
                else {
                  if (!anchor.was_poped) {
                    break;
                  }
                }

                f64 width_t = (f64)var_tsc_elapsed_incl / tsc_elapsed;
                f64 width_t_off = Unlerp((f64)tsc_start, var_tsc_start, tsc_end);
                if (anchor.tsc_elapsed_incl != anchor.tsc_elapsed_excl) {
                  width_t = (f64)anchor.tsc_elapsed_incl / tsc_elapsed;
                }
                f32 height = 30;
                f32 height_off = anchor.depth * height;
                f32 width = width_t * avail_size.x;
                f32 width_off = width_t_off * avail_size.x;
                Rng2 rect = rng2_make(v2(width_off, height_off), v2(width, height));
                UI_Item item = {
                  .type = UI_ItemType_Bar,
                  .rect = rect,
                  .anchor = anchor,
                };
                array_push(items, item);
              }
            }
            array_push(items, {.type = UI_ItemType_NextThread});
          }

          ///////////////////////////////////
          // Anchors and thread offsets
          f32 height_off = 0;
          f32 thread_height_off = 200;
          Loop (i, items.count) {
            UI_Item& item = items[i];
            switch (item.type) {
              case UI_ItemType_Bar: {
                item.rect = rng2_shift(item.rect, v2(width_off, height_off));
              } break;
              case UI_ItemType_NextThread: {
                height_off += thread_height_off;
              } break;
            }
          }

          // Scroll
          Loop (i, items.count) {
            Rng2& rect = items[i].rect;
            rect = rng2_shift(rect, cursor_pos);
            rect = rng2_scale(rect, scroll_state.scale);
            rect = rng2_shift(rect, scroll_state.offset);
          }

          ///////////////////////////////////
          // Drawing
          Loop (i, items.count) {
            UI_Item item = items[i];
            switch (item.type) {
              default:{}break;
              case UI_ItemType_Bar: {
                v4 color = {};
                String str = {};
                ProfAnchor anchor = item.anchor;
                Rng2 rect = item.rect;
                switch (anchor.type) {
                  case ProfType_Default: {
                    color = ColorGreyDark;
                    str = "work";
                  } break;
                  case ProfType_Sleep: {
                    color = ColorGreenUi;
                    str = "sleep";
                  } break;
                  case ProfType_Worker: {
                    color = ColorOrangeUi;
                    str = "job";
                  } break;
                }
                imgui_draw_rect_filled(draw, rect, color);
                imgui_draw_rect(draw, rect, ColorGreyLight);
                if (rng2_contains(rect, mouse_pos)) {
                  ImGui::BeginTooltip();
                  imgui_text("Label: %s", anchor.label.str);
                  imgui_text("Percent: %f%%", rng2_dim(rect).x / avail_size.x * 100);
                  imgui_text("Time: %fms", tsc_to_ms(anchor.tsc_elapsed_incl));
                  imgui_text("Time exclusive: %fms", tsc_to_ms(anchor.tsc_elapsed_excl));
                  imgui_text("Type: %s", str.str);
                  ImGui::EndTooltip();
                }

                // Text
                {
                  String str = push_strf(scratch, "%s %.3f", anchor.label, tsc_to_ms(anchor.tsc_elapsed_incl));
                  v2 text_size = imgui_calc_text_size(str);
                  if (rng2_dim(rect).x < 30.1 || scroll_state.scale.y < 0.3) {
                    continue;
                  }
                  v2 text_pos = {};
                  if (text_size.x > rng2_dim(rect).x) {
                    text_pos.x = rect.min.x;
                    text_pos.y = rect.min.y + (rng2_dim(rect).y - text_size.y) * 0.5;
                  } else {
                    text_pos = rng2_align_dim_at_center(rect, text_size).min;
                  }
                  imgui_draw_push_clip_rect(draw, rect);
                  imgui_draw_text(draw, debug.font, time_bar_text_size, text_pos, ColorWhite, str);
                  imgui_draw_pop_clip_rect(draw);
                }
              } break;
            }
          }
        };

        imgui_text("%.1ffps %.1fms CPU %.1fGhz, Recording: %s", 1000 / tsc_to_ms(tsc_elapsed), tsc_to_ms(tsc_elapsed), (f64)cpu_freq / Billion(1), prof.paused ? "off" : "on");
        imgui_text("avg %.1fms, max %.1f, min %.1f", prof.frame_avg_time, prof.frame_max_time, prof.frame_min_time);
        f32 info_height = 60;
        cursor_pos.y += info_height;

        ///////////////////////////////////
        // Draw thread names
        var draw_threads = [&](ScrollState scroll_state) {
          f32 thread_height = 200;
          f32 thread_height_offset = 0;
          f32 text_off_above = -40;
          {
            String str = push_strf(scratch, "Main thread");
            v2 text_pos = (v2(0, text_off_above) + cursor_pos);
            text_pos.y *= scroll_state.scale.y;
            text_pos.y += scroll_state.offset.y;
            // imgui_draw_text(draw, text_pos, ColorWhite, str);
            imgui_draw_text(draw, debug.font, thread_name_text_size, text_pos, ColorWhite, str);
            // draw.draw->AddText(debug.font, 20, text_pos, u32_from_rgba(ColorWhite), (char*)str.str);
          }
          {
            Loop (i, THREAD_COUNT) {
              thread_height_offset += thread_height;
              String str = push_strf(scratch, "Worker %i", i);
              v2 text_pos = v2(0, thread_height_offset + text_off_above) + cursor_pos;
              text_pos.y *= scroll_state.scale.y;
              text_pos.y += scroll_state.offset.y;
              // imgui_draw_text(draw, text_pos, ColorWhite, str);
              imgui_draw_text(draw, debug.font, thread_name_text_size, text_pos, ColorWhite, str);
            }
            thread_height_offset = 0;
          }
        };

        ///////////////////////////////////
        // Tab mouse click
        var tab_mouse_click_handle = [&](String name, ProfTabActive tab) {
          Scratch scratch;
          String str = push_strf(scratch, name);
          imgui_begin_tab_item(str);
          Rng2 tab_rect = Rng2(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
          // if (str_match(name, "root")) {
          //   Debug("root");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "frames")) {
          //   Debug("frames");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "time")) {
          //   Debug("time");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "launch")) {
          //   Debug("launch");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          // if (str_match(name, "memory")) {
          //   Debug("memory");
          //   Info("min: %f %f", tab_rect.min.x, tab_rect.min.y);
          //   Info("max: %f %f", tab_rect.max.x, tab_rect.max.y);
          // }
          if (os_mouse_is_button_pressed(MouseButton_Left)) {
            if (rng2_contains(tab_rect, mouse_pos)) {
              switch (tab) {
                case ProfileTabActive_Root: prof.future_active_tab = ProfileTabActive_Root; break;
                case ProfileTabActive_Frames: prof.future_active_tab = ProfileTabActive_Frames; break;
                case ProfileTabActive_Time: prof.future_active_tab = ProfileTabActive_Time; break;
                case ProfileTabActive_LaunchTime: prof.future_active_tab = ProfileTabActive_LaunchTime; break;
                case ProfileTabActive_Memory: prof.future_active_tab = ProfileTabActive_Memory; break;
              }
            }
          }
        };

        ///////////////////////////////////
        // Tabs
        tab_mouse_click_handle("root", ProfileTabActive_Root);
        tab_mouse_click_handle("frames", ProfileTabActive_Frames);
        tab_mouse_click_handle("time", ProfileTabActive_Time);
        tab_mouse_click_handle("launch", ProfileTabActive_LaunchTime);
        tab_mouse_click_handle("memory", ProfileTabActive_Memory);
        switch (prof.active_tab) {
          case ProfileTabActive_Root: {
            ScrollState& scroll_state = prof_win.root_scroll_state;
            if (ImGui::IsWindowHovered()) {
              scroll_state_update(scroll_state, ScrollType_PowClamp);
            }
            cursor_pos.y += 30;
            draw_threads(scroll_state);
            u32 idx = (st->current_frame-1) % ArrayCount(prof.frames_times);
            Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
            for EachElement(i, prof.prof_threads) {
              slices[i] = slice(prof.prof_threads[i].recorded_anchors[idx]);
            }
            ProfFrameTime time = prof.frames_times[idx];
            draw_frame_graph(slice(slices), time, 0, scroll_state);
            ImGui::EndTabItem();
          } break;
          case ProfileTabActive_Frames: {
            ScrollState& scroll_state = prof_win.frames_scroll_state;
            if (ImGui::IsWindowHovered()) {
              scroll_state_update(scroll_state, ScrollType_PowClamp);
            }
            f32 width_size = avail_size.x;

            ///////////////////////////////////
            // Little bars
            Loop (i, ArrayCount(prof.frames_times)) {
              ProfFrameTime frame_time = prof.frames_times[i];
              f32 max_height = 40;
              f32 max_ms = 30;
              f64 frame_ms = tsc_to_ms(frame_time.tsc_end - frame_time.tsc_start);
              f64 height = max_height / (max_ms / frame_ms);
              v2 size = v2(avail_size.x / ArrayCount(prof.frames_times), height);
              v2 min = cursor_pos + v2(i*size.x, -height + max_height);
              Rng2 rect = rng2_make(min, size);
              if (rng2_contains(rect, mouse_pos)) {
                ImGui::BeginTooltip();
                ImGui::Text("frame: %i", i);
                ImGui::EndTooltip();
                if (os_mouse_is_button_pressed(MouseButton_Left)) {
                  prof_win.frames_scroll_state.offset.x = -width_size * i;
                  prof_win.frames_scroll_state.scale = v2_splat(1);
                }
              }
              v4 color = ColorGreen;
              if (rng1_contains(Rng1(17, 21), frame_ms)) {
                color = ColorYellow;
              } else if (frame_ms > 20) {
                color = ColorRed;
              }
              if (i == st->current_frame % ArrayCount(prof.frames_times)) {
                color = ColorGrey3;
              }

              if (i == st->current_frame % ArrayCount(prof.frames_times)) {
                imgui_draw_rect_filled(draw, rect, color);
              } else {
                imgui_draw_rect_filled(draw, rect, color);
                imgui_draw_rect(draw, rect, v4_set_w(ColorGrey0, 0.3));
              }
            }
            cursor_pos.y += 80;

            ///////////////////////////////////
            // Draw lines and current rect
            {
              f32 width_offset = 0;
              Loop (i, ArrayCount(prof.frames_times)) {
                f32 line_height = 1000;
                f32 thick = 1;
                v2 base = cursor_pos + v2(width_offset, 0);
                v2 p0 = base + v2(0, -line_height / 2);
                v2 p1 = base + v2(0, line_height);
                v2 p2 = base + v2(width_size, 0);
                v2 p3 = base + v2(width_size, 0) + v2(0, line_height);
                p0 = p0 * scroll_state.scale.x + scroll_state.offset;
                p1 = p1 * scroll_state.scale.x + scroll_state.offset;
                p2 = p2 * scroll_state.scale.x + scroll_state.offset;
                p3 = p3 * scroll_state.scale.x + scroll_state.offset;
                imgui_draw_line(draw, p0, p1, ColorGrey3, thick);
                if (i == st->current_frame % ArrayCount(prof.frames_times)) {
                  imgui_draw_rect_filled(draw, Rng2(p0, p3), v4(0.4,0.4,0.4,0.4));
                }
                width_offset += width_size;
              }
            }

            draw_threads(scroll_state);

            ///////////////////////////////////
            // Draw graph per thread
            for EachElement(j, prof.frames_times) {
              Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
              for EachElement(i, prof.prof_threads) {
                slices[i] = slice(prof.prof_threads[i].recorded_anchors[j]);
              }
              ProfFrameTime time = prof.frames_times[j];
              draw_frame_graph(slice(slices), time, j * width_size, scroll_state);
            }
            ImGui::EndTabItem();
          } break;
          case ProfileTabActive_Time: {
            var sorted_anchors = slice_clone(scratch, anchors);
            sort_insert(sorted_anchors, [](ProfAnchor a, ProfAnchor b) { return a.tsc_elapsed_excl > b.tsc_elapsed_excl; });

            Loop (i, anchors.count) {
              ImGui::PushID(i);
              ProfAnchor anchor = sorted_anchors[i];
              f64 width_exclusive_percent = (f64)anchor.tsc_elapsed_excl / tsc_elapsed;
              f32 width_exclusive = avail_size.x * 0.8;
              f32 height = 30;
              width_exclusive *= width_exclusive_percent;

              v2 offset = v2(0,  i * height) + cursor_pos;
              v2 size = v2(width_exclusive, height);
              Rng2 rect = Rng2(offset, size + offset);

              imgui_draw_rect_filled(draw, rect, ColorGreyDark);
              imgui_draw_rect(draw, rect, ColorGreyLight);

              String name_str = push_strf(scratch, "%s", anchor.label);
              String ms_str = push_strf(scratch, "%.3fms", (f64)anchor.tsc_elapsed_excl / cpu_freq * 1000);
              v2 name_offset = v2(0, height * i) + cursor_pos;
              v2 ms_offset = v2(avail_size.x * 0.82, height * i) + cursor_pos;

              imgui_draw_text(draw, name_offset, ColorWhite, name_str);
              imgui_draw_text(draw, ms_offset, ColorWhite, ms_str);

              ImGui::PopID();
            }
            ImGui::EndTabItem();
            } break;
          case ProfileTabActive_LaunchTime: {
            ScrollState& scroll_state = prof_win.launch_time_scroll_state;
            if (ImGui::IsWindowHovered()) {
              scroll_state_update(scroll_state, ScrollType_PowClamp);
            }

            draw_threads(scroll_state);

            Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
            for EachElement(i, prof.prof_threads) {
              slices[i] = slice(prof.prof_threads[i].launch_anchors);
            }
            ProfFrameTime time = prof.launch_time;
            draw_frame_graph(slice(slices), time, 0, scroll_state, true);
            ImGui::EndTabItem();
          } break;
          case ProfileTabActive_Memory: {
            ScrollState& scroll_state = prof_win.mem_scroll_state;
            if (ImGui::IsWindowHovered()) {
              scroll_state_update(scroll_state);
            }
            enum UI_ItemType {
              UI_ItemType_MemUsage,
              UI_ItemType_MemLevel,
              UI_ItemType_Arena,
              UI_ItemType_Child,
            };
            struct UI_Item {
              UI_ItemType type;
              Rng2 rect;
              u32 depth;
              AllocatorInfo* info;
              u32 mem_level;
            };
            var items = darray_make<UI_Item>(scratch);
            AllocatorInfoList infos = get_allocators_info();
            var infos_sorted = sort_list_insert(scratch, infos.first, [](var a, var b) { return a->pos > b->pos; });
            f64 mem_usage = 0;
            Loop (i, infos_sorted.count) {
              AllocatorInfo* x = infos_sorted[i];
              mem_usage += x->cmt;
            }
            f32 mem_levels[] = {KB(1), KB(10), KB(100), MB(1), MB(10), MB(100), GB(1)};

            ///////////////////////////////////
            // Layout
            {
              f32 row_h = 30;
              Rng2Cursor curs = {};
              // mem usage
              {
                UI_Item item = {
                  .type = UI_ItemType_MemUsage,
                  .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
                };
                array_push(items, item);
              }

              b32 level_drawn[ArrayCount(mem_levels)] = {};
              Loop (i, infos_sorted.count) {
                var& info = *infos_sorted[i];
    
                //  Mem level
                u32 mem_level = 0;
                for EachElement(i, mem_levels) {
                  if (info.pos < mem_levels[i]) {
                    mem_level = i;
                    break;
                  }
                }
                if (!level_drawn[mem_level]) {
                  level_drawn[mem_level] = true;
                  UI_Item item = {
                    .type = UI_ItemType_MemLevel,
                    .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
                    .mem_level = mem_level,
                  };
                  array_push(items, item);
                }
    
                // Arena
                {
                  UI_Item item = {
                    .type = UI_ItemType_Arena,
                    .rect = layout_row(curs, Rng1(0, avail_size.x), row_h),
                    .info = &info,
                    .mem_level = mem_level,
                  };
                  array_push(items, item);
                }
    
                // Children
                u32 depth = 1;
                struct StackEntry {
                  AllocatorInfo* node;
                  u32 depth;
                };
                var stack = darray_make<StackEntry>(scratch);
                Slice sorted_children = sort_list_insert(scratch, info.first, [](var a, var b) { return a->pos > b->pos; });
                ReverseLoop (i, sorted_children.count) {
                  array_push(stack, {sorted_children[i], 1});
                }
                while (stack.count) {
                  StackEntry entry = array_pop(stack);
                  var child = entry.node;
                  UI_Item item = {
                    .type = UI_ItemType_Child,
                    .depth = entry.depth,
                    .info = child,
                    .mem_level = mem_level,
                  };
                  item.rect = Rng2(
                    v2(depth * 10, curs.pos.y),
                    v2(avail_size.x, curs.pos.y + row_h * 0.6)
                  );
                  array_push(items, item);
                  layout_next(curs, row_h * 0.6);
                  if (child->first) {
                    ++depth;
                    Slice sorted_children = sort_list_insert(scratch, child->first, [](var a, var b) { return a->pos > b->pos; });
                    ReverseLoop (i, sorted_children.count) {
                      array_push(stack, {sorted_children[i], entry.depth + 1});
                    }
                  }
                }
              }
            }

            Loop (i, items.count) {
              UI_Item& item = items[i];
              item.rect = rng2_shift(item.rect, cursor_pos);
              item.rect = rng2_scale(item.rect, scroll_state.scale);
              item.rect = rng2_shift(item.rect, scroll_state.offset);
            }

            Rng2 rounding_edge = rng2_shift(rng2_scale(rng2_make(cursor_pos, avail_size), scroll_state.scale.x), scroll_state.offset);
            rounding_edge = rng2_pad(rounding_edge, 10);
            imgui_draw_rect(draw, rounding_edge, ColorGreyLight);

            ///////////////////////////////////
            // Drawing
            Loop (i, items.count) {
              UI_Item item = items[i];
              AllocatorInfo& info = *item.info;

              switch (item.type) {
                case UI_ItemType_MemUsage: {
                  MemFormatSize mem_fmt = mem_format_size(mem_usage);
                  String mem_usage_str = push_strf(scratch, "mem usage: %.2f%s", mem_fmt.size, mem_fmt.format);
                  imgui_draw_text(draw, cursor_pos, ColorWhite, mem_usage_str);
                } break;
                case UI_ItemType_MemLevel: {
                  MemFormatSize mem_fmt = mem_format_size(mem_levels[item.mem_level]);
                  String str = push_strf(scratch, "%.0f%s", mem_fmt.size, mem_fmt.format);
                  v2 text_size = imgui_calc_text_size(str);
                  Rng2 rect = item.rect;
                  Rng2 text_rect = rng2_align_dim_at_center(rect, text_size);
                  Rng2 pad_text_rect = rng2_pad(text_rect, 5);
                  imgui_draw_rect_filled(draw, pad_text_rect, v4(0.3, 3.5, 0.5, 0.5));
                  imgui_draw_text(draw, text_rect.min, ColorWhite, str);
                } break;
                case UI_ItemType_Arena: {
                  f32 t_w = rng2_dim(item.rect).x;
                  f32 t_pos = info.pos / mem_levels[item.mem_level];
                  // f32 t_cap = info.cap / mem_levels[item.mem_level];
                  f32 t_excl = info.exclusive_pos / mem_levels[item.mem_level];
                  f32 w_pos = t_w * t_pos;
                  // f32 w_cap = t_w * t_cap;
                  f32 w_excl = t_w * t_excl;
                  Rng2 excl_rect = rng2_subrng_x(item.rect, Rng1(0, w_excl));
                  Rng2 incl_rect = rng2_subrng_x(item.rect, Rng1(w_excl, w_pos));

                  imgui_draw_rect_filled(draw, excl_rect, ColorGreenUi);
                  imgui_draw_rect(draw, excl_rect, ColorGreyLight);
                  imgui_draw_rect_filled(draw, incl_rect, ColorBlueUi);
                  imgui_draw_rect(draw, incl_rect, ColorGreyLight);

                  MemFormatSize pos = mem_format_size(info.pos);
                  MemFormatSize pos_exclusive = mem_format_size(info.exclusive_pos);
                  MemFormatSize cmt = mem_format_size(info.cmt);

                  String name_str = push_strf(scratch, "%s", info.name);
                  String mem_str = push_strf(scratch, "%.2f%s pos, %.2f%s cmt", pos.size, pos.format, cmt.size, cmt.format);

                  imgui_draw_text(draw, excl_rect.min, ColorWhite, name_str);
                  imgui_draw_text(draw, rng2_subrng_x01(item.rect, Rng1(0.2, 1)).min, ColorWhite, mem_str);
                  
                  if (rng2_contains(rng2_union(incl_rect, excl_rect), mouse_pos)) {
                    ImGui::BeginTooltip();
                    imgui_text(push_strf(scratch, "inclusive: %.2f%s", pos.size, pos.format));
                    imgui_text(push_strf(scratch, "exclusive: %.2f%s", pos_exclusive.size, pos_exclusive.format));
                    ImGui::EndTooltip();
                  }
                } break;
                case UI_ItemType_Child: {
                  f32 t_w = rng2_dim(item.rect).x;
                  f32 t_pos = info.pos / mem_levels[item.mem_level];
                  f32 t_cap = info.cap / mem_levels[item.mem_level];
                  // f32 t_excl = info.exclusive_pos / mem_levels[item.mem_level];
                  f32 w_pos = t_w * t_pos;
                  f32 w_cap = t_w * t_cap;
                  // f32 w_excl = t_w * t_excl;
                  Rng2 child_rect = rng2_subrng_x(item.rect, Rng1(0, w_pos));
                  Rng2 child_rect_cap = rng2_subrng_x(item.rect, Rng1(w_pos, w_cap));

                  // pos
                  imgui_draw_rect_filled(draw, child_rect, ColorGreyDark);
                  imgui_draw_rect(draw, child_rect, ColorGreyLight);
                  
                  // cap
                  imgui_draw_rect_filled(draw, child_rect_cap, ColorRedUi);
                  imgui_draw_rect(draw, child_rect_cap, ColorGrey);

                  MemFormatSize pos = mem_format_size(info.pos);
                  MemFormatSize cap = mem_format_size(info.cap);
                  String child_name_str = push_strf(scratch, "%s", info.name);
                  String child_meta_str = push_strf(scratch, "%.2f%s pos, %.2f%s cap, alloc count: %u, free count: %u, current alloc count: %u", pos.size, pos.format, cap.size, cap.format, info.allocs, info.frees, info.current_allocs);
                  imgui_draw_text(draw, child_rect.min, ColorWhite, child_name_str);
                  imgui_draw_text(draw, rng2_subrng_x01(child_rect, Rng1(0.3, 1)).min, ColorWhite, child_meta_str);
                } break;
              }
            }
            ImGui::EndTabItem();
          } break;
        }
        ImGui::EndTabBar();
      }

      ImGui::PopClipRect();
    } ImGui::End();
  }

  prof.active_tab = prof.future_active_tab;
}
