#pragma once
#include "lib.h"

#define IM_VEC2_CLASS_EXTRA                               \
        constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {} \
        operator v2() const { return v2(x,y); }
#include "imgui/imgui.h"

MakeId(OpaqueId)
MakeId(ThingId)

const u32 MaxEntities = KB(1);

const v4 ColorWhite       = v4(1,    1,    1,    1);
const v4 ColorBlack       = v4(0,    0,    0,    1);
const v4 ColorGrey        = v4(0.5,  0.5,  0.5,  1);
const v4 ColorGreyDark    = v4(0.25, 0.25, 0.25, 1);
const v4 ColorGreyLight   = v4(0.75, 0.75, 0.75, 1);
const v4 ColorGrey0       = v4(0.15, 0.15, 0.15, 1);
const v4 ColorGrey1       = v4(0.35, 0.35, 0.35, 1);
const v4 ColorGrey2       = v4(0.60, 0.60, 0.60, 1);
const v4 ColorGrey3       = v4(0.85, 0.85, 0.85, 1);

const v4 ColorRed         = v4(0.89, 0.33, 0.23, 1.0);
const v4 ColorOrange      = v4(0.95, 0.65, 0.30, 1.0);
const v4 ColorYellow      = v4(0.90, 0.82, 0.35, 1.0);
const v4 ColorGreen       = v4(0.14, 0.8,  0.01, 1.0);
const v4 ColorCyan        = v4(0.35, 0.78, 0.82, 1.0);
const v4 ColorBlue        = v4(0.38, 0.65, 0.95, 1.0);
const v4 ColorPurple      = v4(0.72, 0.55, 0.92, 1.0);
const v4 ColorPink        = v4(0.90, 0.45, 0.65, 1.0);

const v4 ColorRedUi        = v4(0.3, 0.15, 0.15, 1);
const v4 ColorRedUiBright  = v4(0.41, 0.19, 0.21, 1);
const v4 ColorGreenUi      = v4(0.3, 0.4, 0.2, 1);
const v4 ColorBlueUi       = v4(0.15, 0.3, 0.5, 1);
const v4 ColorOrangeUi     = v4(0.4, 0.28, 0.15, 1);
const v4 ColorYellowUi     = v4(0.4, 0.43, 0.2, 1);
const v4 ColorCyanUi       = v4(0.15, 0.4, 0.36, 1);
const v4 ColorPurpleUi     = v4(0.32, 0.19, 0.41, 1);

const v4 ColorBounds     = v4(0.38, 0.65, 0.95, 1);
const v4 ColorSelection  = v4(0.95, 0.65, 0.30, 1);
const v4 ColorCollision  = v4(0.86, 0.33, 0.33, 1);

enum LightType {
  LightType_Point,
  LightType_Dir,
  LightType_Spot,
};

struct Light {
  LightType type;
  v3 pos;
  v3 dir;
  v3 color;
  f32 intensity;
  f32 radius;
  f32 cone_angle;
  b32 cast_shadow;
};

struct Timer {
  f32 interval;
  f32 acc;
};

