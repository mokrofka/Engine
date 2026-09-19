#pragma once
#include "lib.h"
#include "types.h"
#include "gfx.h"
#include "render.h"
#include "meta.h"

#define STBI_NO_STDIO
#include "stb_image.h"
#include "stb_truetype.h"

// TODO:
// dummy assets/null 
// UI
// memory visualisation thread safe, and gpu memory
// thread graph visualisation
// make wayland backend work
// console
// thread safe allocator
// glb loader
// obj mouse selection
// gpu profiler, memory

#define MESH_LIST \
	X(Cube) \
	X(MonkeyGlb) \
	X(Triangle) \
	X(Grid) \
	X(Axis) \
	X(Sphere) \
	X(CubeGlft) \
	X(GreeMan) \
	X(Barrack) \

#define TEXTURE_LIST \
	X(Dummy) \
	X(Orange) \
	X(Container) \
	X(Barrack) \
	X(Black) \
	X(Black1) \
	X(Black2) \
	X(Bricks) \

#define MATERIAL_LIST \
	X(Dummy) \
	X(Orange) \
	X(Container) \
	X(Axis) \
	X(Line) \
	X(Barrack) \

enum MeshEnum {
#define X(name) Glue(Mesh_, name),
	MESH_LIST
#undef X
	Mesh_COUNT,
};

enum TextureEnum {
#define X(name) Glue(Texture_, name),
	TEXTURE_LIST
#undef X
	Texture_COUNT,
};

enum MaterialEnum {
#define X(name) Glue(Material_, name),
	MATERIAL_LIST
#undef X
	Material_COUNT,
};

struct MemberDefinition {
	u32 type;
	String name;
	u64 offset;
};

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

struct ImGuiImage {
	ImTextureRef h;
	b32 loaded;
};

enum JsType {
	JsType_Null,
	JsType_Bool,
	JsType_Number,
	JsType_Str,
	JsType_Array,
	JsType_Obj,
};

struct JsField {
	String key;
	struct JsVal* val;
};

struct JsObj {
	Slice<JsField> fields;
};

struct JsVal {
	JsType type;
	union {
		b32 boolean;
		f64 number;
		String str;
		Slice<JsVal*> array;
		JsObj obj;
	};
};

struct JsParser {
	Allocator arena;
	String str;
	u32 cursor;
};

struct InputState {
	b8 consumed[Key_COUNT];
};

enum WatchOp {
	WatchOp_NotifyHotreload = 1,
	WatchOp_RecompileShader,
	WatchOp_ShaderReload,
};

struct WatchFile {
	String path;
	u64 modified;
	WatchOp op;
};

struct WatchDirectory {
	String path;
	OS_Watch watch;
	WatchOp op;
};

Introspect struct Camera {
	v3 pos;
	v3 dir;
	f32 yaw;
	f32 pitch;
	f32 fov;
	f32 accel;
	v3 vel;
	f32 vel_friction;
};

typedef u32 EntityFlags;
enum {
	EntityFlag_Referenced = 1,
	EntityFlag_NotRender,
};

struct EntityThing {
	String name;
	EntityFlags flags;
};

struct ThingDesc {
	v3 pos;
	v4 rot;
	v3 scale;
	v3 vel;
	u32 color;
	Rng3 aabb;
	MeshEnum mesh;
	MaterialEnum mat;
};

typedef u32 ThingFlags;
enum {
	ThingFlag_1 = 1,
	ThingFlag_2,
	ThingFlag_3,
};

struct ThingList {
	ThingId first;
	ThingId last;
};

Introspect struct Thing {
	ThingId parent;
	ThingId next;
	ThingId prev;
	ThingId first;
	ThingId last;
	String name;
	EntityFlags flags;
	ThingFlags tflags;
	v3 pos;
	v4 rot;
	v3 scale;
	Rng3 aabb;
	v3 vel;
	R_MeshId mesh;
	R_MaterialId mat;
	u32 color;
	f32 elapsed;
	u8 buf0[64];
	f32 modify0;
	u8 buf1[64];
	f32 modify1;
	u8 buf2[64];
	f32 modify2;
	u8 buf3[64];
	f32 modify3;
	f32 angle;
};

typedef u32 ThingState;
enum {
	ThingState_OnFire = 1<<0,
	ThingState_Flying = 1<<1,
	ThingState_Poisoned = 1<<2,
};

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
	UI_BoxFlag_Clickable 						= 1<<1,
	UI_BoxFlag_DrawText 							= 1<<2,
	UI_BoxFlag_DrawBorder 					= 1<<2,
	UI_BoxFlag_DrawBackground 	= 1<<3,
	UI_BoxFlag_HotAnimation 			= 1<<4,
	UI_BoxFlag_ActiveAnimation = 1<<5,
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

struct GlobalState {
	Arena arena;
	Arena frame_arena;
	Alloc gpa;
	b32 should_hotreload;
	m4x4 view;
	m4x4 projection;
	v4 ambient_color;

	R_MeshId meshes_ids[Mesh_COUNT];
	R_TextureId textures_ids[Texture_COUNT];
	R_MaterialId materials_ids[Material_COUNT];
	R_FontId font;

	String asset_dir;
	String shader_dir;
	String shader_compiled_dir;
	String models_dir;
	String textures_dir;
	Map<R_TextureId, R_MaxTextures> str_to_texture;
	Map<R_MeshId, R_MaxMeshes> str_to_mesh;
	Map<R_MaterialId, R_MaxMaterials> str_to_material;
	Array<String, R_MaxTextures> texture_to_str;
	Array<String, R_MaxMeshes> mesh_to_str;
	Array<String, R_MaxMaterials> material_to_str;

	InputState input;
	R_State r;
	Gfx_State gfx;
	UI_State ui;

	Slice<OS_Handle> shader_module_compilation_pids;
	Slice<String> shaders_to_compile;

	Array<WatchFile, 128> watches;
	Array<WatchDirectory, 128> watch_directories;

	struct {
		DebugWindow win;
		ScrollState root_scroll_state;
		ScrollState frames_scroll_state;
		ScrollState launch_time_scroll_state;
		ScrollState mem_scroll_state;
		ProfTabActive active_tab;
		f32 frame_avg_time;
		f32 frame_min_time;
		f32 frame_max_time;
		ProfColors colors;
	} prof_win;
	DebugWindow game_win;
	b32 imgui_demo_open;

	Camera cam;
	R_Camera r_cam;
	b32 fps_camera;

	u32 entities_count;
	// PoolLinkList<Thing, MaxEntities, ThingId> entities;
	// Pool<Thing, MaxEntities, ThingId> entities;
	// SparseSet<MaxEntities> active_entities;
	PoolIterative<Thing, MaxEntities, ThingId> entities;

	DArray<ThingId> moving_cubes;
	Map<ThingId, 32> find_entity;

	ThingId axis_attached_to_cam_id;
	ThingId monkey0;
	ThingId cube0;
	ThingId cube1;
	ThingId cube2;
	ThingId cube3;
	ThingId cube4;
	ThingId cube5;

	ThingId cube_root;
	ThingId monkey1;
	v3 pos_target;

	Coroutine co;
	ImTextureID imgui_dummy;
};

extern GlobalState* st;

void test();
f64 tsc_to_ms(u64 tsc);
// f32 time_dt;
// f32 time_now;
b32 time_on_interval(f64 time, f32 delta, f32 interval, f32 offset);
u32 time_on_interval_steps(f64 time, f32 delta, f32 interval, f32 offset);
f64 time_next_interval(f64 time, f32 interval, f32 offset);
f64 time_prev_interval(f64 time, f32 interval, f32 offset);
b32 time_on_time(f64 time, f64 timestamp, f64 dt);
b32 time_on_between_interval(f64 time, f32 interval, f32 offset);
f32 time_percent(f64 time, f64 start, f64 duration);
f32 time_lerp_delta(f32 current, f32 target, f32 rate, f32 delta);
b32 time_on_frame_interval(u32 frame, u32 n, u32 offset);
f64 time_saw_wave(f64 time, f32 interval, f32 offset);
f32 time_sine_wave(f64 time, f32 period);
f32 time_smooth_wave(f64 time, f32 period);
f32 time_triangle_wave(f64 time, f32 period);
b32 time_pulse_wave(f64 time, f32 period, f32 duration, f32 offset);
u32 time_frame(f64 time, f32 frame_duration, u32 frame_count);
b32 time_elapsed(f64 time, f32 start, f32 duration);
b32 time_between(f64 time, f32 start, f32 end);
b32 time_on_interval(f32 interval, f32 offset = 0);
b32 time_on_between_interval(f32 interval, f32 offset = 0);
f64 time_since(f64 timestamp);
f64 time_until(f64 timestamp);

// void ui_draw_rect(Rng2 rect, v4 color);
// b32 ui_button(u32 id, v2 pos);
// b32 ui_slider(u32 id, v2 pos, i32 max, i32& v);
// void ui_begin();
// void ui_end();

void imgui_draw_rect(ImDrawList* draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0, f32 thickness = 1);
void imgui_draw_rect_filled(ImDrawList* draw, Rng2 rect, v4 col, f32 rounding = 0, ImDrawFlags flags = 0);
void imgui_draw_push_clip_rect(ImDrawList* draw, Rng2 rect);
void imgui_draw_pop_clip_rect(ImDrawList* draw);
void imgui_draw_line(ImDrawList* draw, v2 p0, v2 p1, v4 col, f32 thickness = 1);
void imgui_draw_text(ImDrawList* draw, v2 pos, v4 col, String fmt, ...);
void imgui_draw_text(ImDrawList* draw, f32 font_size, v2 pos, v4 col, String fmt, ...); struct ImGuiDrawText_Params {f32 font_size; v2 pos; v4 col = ColorWhite;};
void imgui_text(String fmt, ...);
v2 imgui_calc_text_size(String str);

Rng2 debug_window_get_rect(DebugWindow win);
void debug_window_apply_state(DebugWindow& win);
void debug_window_track_state(DebugWindow& win);
void debug_window_toggle_fullscreen(DebugWindow& win);
void dev_init();
void dev_update();

R_MeshDesc load_obj(Allocator arena, String name);
R_MeshDesc load_gltf(Allocator arena, String path, b32 is_glb);

JsParser js_parse_make(Allocator arena, String str);
u8 js_peek(JsParser* p);
void js_advance(JsParser* p);
void js_skip_ws(JsParser* p);
String js_parse_str(JsParser* p);
f64 js_parse_number(JsParser* p);
JsVal js_parse(JsParser* p);
JsVal js_get_val(JsVal val, String key);
b32 js_get_bool(JsObj obj, String key);
f64 js_get_number(JsObj obj, String key);
String js_get_str(JsObj obj, String key);
Slice<JsVal*> js_get_array(JsObj obj, String key);
JsObj js_get_obj(JsObj obj, String key);

Slice<Token> tokens_from_str(Allocator arena, String string);
Parser parser_make(Slice<Token> tokens);
Token tok_peek(Parser& p);
Token tok_prev(Parser& p);
Token tok_advance(Parser& p);
b32 tok_match(Parser& p, TokenType type);
b32 tok_match_name(Parser& p, String name);
Token tok_expect(Parser& p, TokenType type);
Token tok_expect_name(Parser& p, String name);
f32 parse_f32(Parser& p);
f32 parse_u32(Parser& p);
f32 parse_i32(Parser& p);
v3 parse_v3(Parser& p);

b32 key_pressed(Key key);
b32 key_pressed_consume(Key key);
b32 key_down(Key key);
b32 key_down_consume(Key key);
void key_consume(Key key);

ScrollState scroll_state_make(f32 scale);
void scroll_state_update(ScrollState& s, ScrollType type = ScrollType_Default);

void watch_add(String watch_name, WatchOp op);
void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags = OS_WatchFlag_Modify);
void watch_update();

ThingDesc default_thing_desc();
R_MaterialProps default_material_props();
Thing& get_thing(ThingId id);
void push_child_thing(ThingId parent, ThingId id);
R_MeshId get_mesh(MeshEnum id);
R_MaterialId get_material(MaterialEnum id);
void mesh_set(MeshEnum mesh_enum, R_MeshId id);

String dumb_struct(Allocator arena, Slice<MemberDefinition> members, void* ptr, EntityFlags flags = {});
void dumb_struct_load(Slice<MemberDefinition> members, void* ptr, Parser* parser);

void init();
shared_function void com(HotReloadData* data);

R_MeshDesc generate_sphere(Allocator arena);
R_MeshDesc generate_grid(Allocator arena, u32 size, f32 step);

// ThingId e_alloc_bare();
// ThingId e_alloc(R_MeshId mesh_id, R_MaterialId material_id, EntityThing thing = {});
// ThingId e_alloc(MeshEnum mesh_id, MaterialEnum material_id, EntityThing thing = {});
ThingId make_thing(ThingDesc desc);
void destroy_thing(ThingId id);
PoolIterativeIter<Thing, MaxEntities, ThingId> things_begin();
HNodeIter<Thing, ThingId> thing_node_begin(ThingId first);
void select_obj();
void save_game_state();
void load_game_state();
void init_game();
void update_game();

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



