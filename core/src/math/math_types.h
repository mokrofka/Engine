#pragma once

#include "defines.h"

union v2 {
  f32 e[2];
  struct {
    union {
      f32 x,r,u;
    };
    union {
      f32 y,g,v;
    };
  };
  v2 () = default;
  v2(f32 x_, f32 y_) { x = x_, y = y_; }
  v2(f32 x_) { x = x_, y = x_; }

  inline void operator+=(v2 vec) {
    x += vec.x; 
    y += vec.y; 
  }
  inline void operator-=(v2 vec) {
    x -= vec.x; 
    y -= vec.y; 
  }
  inline void operator*=(f32 scalar) {
    x *= scalar;
    y *= scalar;
  }
  inline void operator/=(f32 scalar) {
    x /= scalar;
    y /= scalar;
  }
};

union v2i {
  i32 e[2];
  struct {
    union {
      f32 x,r,u;
    };
    union {
      f32 y,g,v;
    };
  };
  v2i () = default;
  v2i(i32 x_, i32 y_) { x = x_, y = y_; }
};

union v3 {
  f32 e[3];
  struct {
    union {
      f32 x,r,u;
    };
    union {
      f32 y,g,v;
    };
    union {
      f32 z,b,w;
    };
  };
  v3 () = default;
  v3(f32 x_, f32 y_, f32 z_) { x = x_, y = y_, z = z_; }

  inline void operator+=(v3 vec) {
    x += vec.x; 
    y += vec.y; 
    z += vec.z; 
  }
  inline void operator-=(v3 vec) {
    x -= vec.x; 
    y -= vec.y; 
    z -= vec.z; 
  }
  inline void operator*=(f32 scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
  }
  inline void operator/=(f32 scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
  }
  inline v3 operator-() {
    return v3(-x,-y,-z);
  }
};

union v4 {
  f32 e[4];
  struct {
    union {
      f32 x,r,u;
    };
    union {
      f32 y,g,v;
    };
    union {
      f32 z,b,p;
    };
    union {
      f32 w,a,q;
    };
  };
  v4 () = default;
  v4(f32 x_, f32 y_, f32 z_, f32 w_) { x = x_, y = y_, z = z_, w = w_; }
  inline void operator+=(v4 vec) {
    x += vec.x; 
    y += vec.y; 
    z += vec.z; 
    w += vec.w; 
  }
  inline void operator-=(v4 vec) {
    x -= vec.x; 
    y -= vec.y; 
    z -= vec.z; 
    w -= vec.w; 
  }
};

typedef v4 quat;

union mat4 {
  f32 data[16];
  
  v4 rows[4];
  void operator*=(mat4 mat);
};

struct Vertex3D {
  v3 position;
};
