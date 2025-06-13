#pragma once
#include "defines.h"

union v2 {
  f32 e[2];
  struct {
    union {
      f32 x;
    };
    union {
      f32 y;
    };
  };
  v2 () = default;
  INLINE v2(f32 x_, f32 y_) { x = x_, y = y_; }
};

union v2i {
  i32 e[2];
  struct {
    union {
      i32 x;
    };
    union {
      i32 y;
    };
  };
  v2i () = default;
  INLINE v2i(i32 x_, i32 y_) { x = x_, y = y_; }
};

union v3 {
  f32 e[3];
  struct {
    union {
      f32 x;
    };
    union {
      f32 y;
    };
    union {
      f32 z;
    };
  };
  v3 () = default;
  INLINE v3 (f32 scale) {
    x = scale; y = scale; z = scale;
  }
  INLINE v3(f32 x_, f32 y_, f32 z_) { x = x_, y = y_, z = z_; }
};

union v4 {
  f32 e[4];
  struct {
    union {
      f32 x;
    };
    union {
      f32 y;
    };
    union {
      f32 z;
    };
    union {
      f32 w;
    };
  };
  v4 () = default;
  INLINE v4(f32 x_, f32 y_, f32 z_, f32 w_) { x = x_, y = y_, z = z_, w = w_; }
};

typedef v4 quat;

union mat4 {
  f32 data[16];
  
  v4 rows[4];
};

struct Vertex3D {
  v3 position;
  v2 texcoord;
};

struct Vertex2D {
  v2 position;
  v2 texcoord;
};

struct Rect {
  u32 x, y;
  u32 w, h;
};

enum Axis{
  Axis_X,
  Axis_Y,
  Axis_Z,
  Axis_W
};

struct Transform {
  v3 pos;
  v3 rot;
  v3 scale;
};
