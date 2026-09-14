#pragma once
#include "types.h"

enum UI_SizeType {
	UI_SizeType_Null,
	UI_SizeType_Pixels,
	UI_SizeType_TextContent,
	UI_SizeType_PercentOfParent,
	UI_SizeType_ChildrenSum,
};

struct UI_Size {
	UI_SizeType type;
	f32 value;
	// f32 strictness;
};

enum UI_Axis2 {
	UI_Axis2_X,
	UI_Axis2_Y,
	UI_Axis2_COUNT,
};

typedef u32 UI_BoxFlags;
enum {
	UI_BoxFlag_Clickable 						= Bit(1),
	UI_BoxFlag_DrawText 							= Bit(2),
	UI_BoxFlag_DrawBorder 					= Bit(2),
	UI_BoxFlag_DrawBackground 	= Bit(3),
	UI_BoxFlag_HotAnimation 			= Bit(4),
	UI_BoxFlag_ActiveAnimation = Bit(5),
};

struct UI_Box {
	UI_Box *first, *last, *next, *prev, *parent;

	u64 key;
	u64 last_frame_touched;

	UI_BoxFlags flags;
	String string;
	UI_Size semantic_size[UI_Axis2_COUNT];
	UI_Axis2 child_layout_axis;
	v4 background_color;
	v4 text_color;
	v4 border_color;

	f32 computed_size[UI_Axis2_COUNT];
	f32 computed_rel_position[UI_Axis2_COUNT];
	Rng2 rect;

	f32 hot_t;
	f32 active_t;
};

struct UI_Signal {
	UI_Box* box;
	b32 hovering;
	b32 pressed;
	b32 released;
	b32 clicked;
	b32 dragging;
	v2 drag_delta;
};

struct UI_Input {
	v2 mouse_pos;
	b32 mouse_down[3];
	f32 dt;
};

enum UI_DrawCmdType {
	UI_DrawCmdType_Rect,
	UI_DrawCmdType_Text
};

struct UI_DrawCmd {
	UI_DrawCmdType kind;
	Rng2 rect;
	v4 color;
	b32 filled;
	String text;
};

struct UI_Style {
	R_FontId font;
	f32 padding;
	f32 gap;
	f32 line_height;
	f32 text_pad;
	v4 bg_color;
	v4 text_color;
	v4 border_color;
	v4 hot_color;
	v4 active_color;
	v4 accent_color;
};

#define UI_MAX_PARENT_STACK 64
#define UI_MAX_COLOR_STACK 64
#define UI_KEY_TABLE_SIZE 32
#define UI_STALE_FRAMES 2

struct UI_State {
	Arena frame_arena;

	Map<UI_Box*, UI_KEY_TABLE_SIZE> box_map;
	PoolPtr<UI_Box, UI_KEY_TABLE_SIZE> boxes;
	SparseSet<UI_KEY_TABLE_SIZE> active_boxes;
	UI_Box* root;

	Array<UI_Box*, UI_MAX_PARENT_STACK> parent_stack;
	Array<v4, UI_MAX_COLOR_STACK> bg_color_stack;

	u64 hot_key;
	u64 active_key;

	UI_Input input;
	UI_Input prev_input;

	Array<UI_DrawCmd, KB(4)> draw_cmds;
	UI_Style style;
};

String ui_display_string(String full);
String ui_hash_string(String full);
u64 ui_key_from_string(String str, u64 seed);
f32 ui_text_measure(String text, R_FontId font, f32 font_height);

UI_Size ui_size_px(f32 v);
UI_Size ui_size_text();
UI_Size ui_size_pct(f32 v);
UI_Size ui_size_children();
UI_Size ui_size_null();

void ui_push_parent(UI_Box* box);
void ui_pop_parent();
UI_Box* ui_top_parent();
void ui_push_font(R_FontId id);
void ui_push_bg_color(v4 color);
void ui_pop_bg_color();
v4 ui_current_bg_color();

UI_Box* ui_box_make(UI_BoxFlags flags, UI_Size size_x, UI_Size size_y, String string);
UI_Box* ui_box_from_key(u64 key);
void ui_prune_stale_boxes();

void ui_init();
void ui_begin_frame();
void ui_end_frame();

void ui_layout_standalone(UI_Box* box);
void ui_layout_upward(UI_Box* box);
void ui_layout_downward(UI_Box* box);
void ui_layout_positions(UI_Box* box);

void ui_build_draw_cmds(UI_Box* box);

f32 ui_animate_towards(f32 current, f32 target, f32 dt, f32 rate_per_sec);
UI_Signal ui_signal_from_box(UI_Box* box);

UI_Signal ui_label(String string);
UI_Signal ui_button(String string);
b32 ui_checkbox(String string, b32* value);
f32 ui_slider(String string, f32* value, f32 min, f32 max);
void ui_spacer(UI_Size size_along_parent_axis);
UI_Box* ui_panel_begin_sized(String string, UI_Axis2 child_layout_axis, UI_Size size_x, UI_Size size_y);
UI_Box* ui_panel_begin(String string, UI_Axis2 child_layout_axis);
void ui_panel_end();

#define UI_Parent(str, axis) DeferLoop(ui_panel_begin(str, axis), ui_pop_parent())

