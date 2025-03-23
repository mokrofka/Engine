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
};

