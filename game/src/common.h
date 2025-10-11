#pragma once
#include <lib.h>

#include "event.h"

struct Entity {
  union {
    struct {
      f32 x;
      f32 y;
      f32 width;
      f32 height;
    };

    Rect rect;
  };
  i32 tilemap_x;
  i32 tilemap_y;

  f32 vx, vy;
  u32 color;
};

struct GameState {
  Arena* arena;
  u32* pixels;
  i32 width;
  i32 height;
  Entity player;
};

GameState* st;

b32 resize_callback(u32 code, void* sender, void* listener_inst, EventContext data) {
  v2i win_size = os_get_window_size();
  f32 aspect = (f32)win_size.x / win_size.y;
  st->width = st->height * aspect;
  arena_clear(st->arena);
  push_buffer(st->arena, st->width*4*st->height * 2);
  return false;
};
