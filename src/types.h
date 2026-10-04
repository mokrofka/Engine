#pragma once
#include "lib.h"

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IM_VEC2_CLASS_EXTRA                        \
	constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {} \
	operator v2() const { return v2(x, y); }
#include "imgui/imgui.h"
#undef assert

#define ImGuiWindow(str, ...) 						DeferLoop(ImGui::Begin(str, null, ##__VA_ARGS__), ImGui::End()) if (_i_)
#define ImGuiPushID(id) 												DeferLoop(ImGui::PushID(id), ImGui::PopID())
#define ImGuiPushFont(font, size) 		DeferLoop(ImGui::PushFont(font, size), ImGui::PopFont())
#define ImGuiTreeNode(str)										DeferLoopIf(ImGui::TreeNode(str), ImGui::TreePop())
#define ImGuiBeginToolTip()									DeferLoop(ImGui::BeginTooltip(), ImGui::EndTooltip())
#define ImGuiBeginTabBar(str)							DeferLoopIf(ImGui::BeginTabBar(str), ImGui::EndTabBar())
#define ImGuiBeginTabItem(str, ...)	DeferLoopIf(ImGui::BeginTabItem(str, null, ##__VA_ARGS__), ImGui::EndTabItem())
#define ImGuiPushClipRect(rect)   		DeferLoop(ImGui::PushClipRect(rect.min, rect.max, true), ImGui::PopClipRect())
struct _ImGuiPushClipRect {
	_ImGuiPushClipRect(Rng2 rect) {ImGui::PushClipRect(rect.min,rect.max, true);}
	~_ImGuiPushClipRect() 								{ImGui::PopClipRect();}
};
#define ImGuiPushClipRect2(rect) _ImGuiPushClipRect Glue(_Clip, __LINE__)(rect)

MakeId(OpaqueId);
MakeId(ThingId);
MakeId(R_TextureId);
MakeId(R_MeshId);
MakeId(R_MaterialId);
MakeId(R_FontId);
MakeId(R_LightId);

const u32 MaxEntities = KB(1);

#define ColorWhite        Color{ 255, 255, 255, 255 }
#define ColorBlack        Color{   0,   0,   0, 255 }
#define ColorGrey         Color{ 128, 128, 128, 255 }
#define ColorGreyDark     Color{  64,  64,  64, 255 }
#define ColorGreyLight    Color{ 191, 191, 191, 255 }
#define ColorGrey0        Color{  38,  38,  38, 255 }
#define ColorGrey1        Color{  89,  89,  89, 255 }
#define ColorGrey2        Color{ 153, 153, 153, 255 }
#define ColorGrey3        Color{ 217, 217, 217, 255 }

#define ColorRed          Color{ 227,  84,  59, 255 }
#define ColorOrange       Color{ 242, 166,  77, 255 }
#define ColorYellow       Color{ 230, 209,  89, 255 }
#define ColorGreen        Color{  36, 204,   3, 255 }
#define ColorCyan         Color{  89, 199, 209, 255 }
#define ColorBlue         Color{  97, 166, 242, 255 }
#define ColorPurple       Color{ 184, 140, 235, 255 }
#define ColorPink         Color{ 230, 115, 166, 255 }

#define ColorRedUi        Color{  77,  38,  38, 255 }
#define ColorRedUiBright  Color{ 105,  48,  54, 255 }
#define ColorGreenUi      Color{  77, 102,  51, 255 }
#define ColorBlueUi       Color{  38,  77, 128, 255 }
#define ColorOrangeUi     Color{ 102,  71,  38, 255 }
#define ColorYellowUi     Color{ 102, 110,  51, 255 }
#define ColorCyanUi       Color{  38, 102,  92, 255 }
#define ColorPurpleUi     Color{  82,  48, 105, 255 }

#define ColorBounds       Color{  97, 166, 242, 255 }
#define ColorSelection    Color{ 242, 166,  77, 255 }
#define ColorCollision    Color{ 219,  84,  84, 255 }

// Raylib col
#define LIGHTGRAY  Color{ 200, 200, 200, 255 }   // Light Gray
#define GRAY       Color{ 130, 130, 130, 255 }   // Gray
#define DARKGRAY   Color{ 80, 80, 80, 255 }      // Dark Gray
#define YELLOW     Color{ 253, 249, 0, 255 }     // Yellow
#define GOLD       Color{ 255, 203, 0, 255 }     // Gold
#define ORANGE     Color{ 255, 161, 0, 255 }     // Orange
#define PINK       Color{ 255, 109, 194, 255 }   // Pink
#define RED        Color{ 230, 41, 55, 255 }     // Red
#define MAROON     Color{ 190, 33, 55, 255 }     // Maroon
#define GREEN      Color{ 0, 228, 48, 255 }      // Green
#define LIME       Color{ 0, 158, 47, 255 }      // Lime
#define DARKGREEN  Color{ 0, 117, 44, 255 }      // Dark Green
#define SKYBLUE    Color{ 102, 191, 255, 255 }   // Sky Blue
#define BLUE       Color{ 0, 121, 241, 255 }     // Blue
#define DARKBLUE   Color{ 0, 82, 172, 255 }      // Dark Blue
#define PURPLE     Color{ 200, 122, 255, 255 }   // Purple
#define VIOLET     Color{ 135, 60, 190, 255 }    // Violet
#define DARKPURPLE Color{ 112, 31, 126, 255 }    // Dark Purple
#define BEIGE      Color{ 211, 176, 131, 255 }   // Beige
#define BROWN      Color{ 127, 106, 79, 255 }    // Brown
#define DARKBROWN  Color{ 76, 63, 47, 255 }      // Dark Brown

#define WHITE      Color{ 255, 255, 255, 255 }   // White
#define BLACK      Color{ 0, 0, 0, 255 }         // Black
#define BLANK      Color{ 0, 0, 0, 0 }           // Blank (Transparent)
#define MAGENTA    Color{ 255, 0, 255, 255 }     // Magenta
#define RAYWHITE   Color{ 245, 245, 245, 255 }   // My own White (raylib logo)

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

union Color {
	u32 v;
	struct {
		u8 r;
		u8 g;
		u8 b;
		u8 a;
	};
	Color() = default;
	Color(u32 col) : v(col) {}
	Color(u8 r, u8 g, u8 b, u8 a):r(r),g(g),b(b),a(a) {}
	// Color(v4 col) { rgba = u32_from_rgba(col); }
	// operator v4() { return rgba_from_u32(rgba); }
};

enum AnimPath {
	AnimPath_Pos,
	AnimPath_Rot,
	AnimPath_Scale,
};

struct Sampler {
	f32* times;
	void* values;
	u32 count;
	u32 interpolation_type;
};

struct Channel {
	u32 bone;
	u32 sampler;
	AnimPath path;
};

struct Animation {
	Slice<Sampler> samplers;
	Slice<Channel> channels;
	f32 duration;
};

struct Bone {
	u32 parent;
	Transform local;
};

struct Pose {
	Slice<Bone> bones;
};

