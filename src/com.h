#pragma once
#include "base/base.h"
#include "gfx.h"
#include "meta.h"

#define STBI_NO_STDIO
#include "../vendor/stb_image.h"
#include "../vendor/stb_truetype.h"

// make wayland backend work
// console
// obj mouse selection

#define CONST const
#include "shader_header.h"

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IM_VEC2_CLASS_EXTRA                        \
	constexpr ImVec2(const v2& f) : x(f.x), y(f.y) {} \
	operator v2() const { return v2(x, y); }
#include "../vendor/imgui/imgui.h"

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
struct TextureId {u32 idx;};
struct MeshId {u32 idx;};
struct MaterialId {u32 idx;};
struct FontId {u32 idx;};

const u32 MaxThings 				= KB(1);
const u32 MaxMaterials  = 16;
const u32 MaxMeshes     = 16;
const u32 MaxTextures   = 16;
const u32 MaxCubemaps   = 16;
const u32 MaxFonts      = 16;
const u32 MaxDebugLines = KB(2);
const u32 MaxSkinMatrices = 16;
const u32 MaxBones 							= 16;
const u32 MaxRuntimeBones = 16;

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

struct UI_Rect {
	v2 dst_p0;
	v2 dst_p1;
	v2 src_p0;
	v2 src_p1;
	v2 tex_size;
	Color colors[4];
	u32 texture;
	u32 flags;
	f32 corner_radius;
	f32 edge_softness;
	f32 border_thickness;
};

struct TextureDesc {
	String name;
	u32 width;
	u32 height;
	union {
		u8* data;
		u8* cube[6];
	};
	Gfx_Format format;
	b32 is_cube;
};

struct Texture {
	Gfx_Image image;
	Gfx_View view;
	b32 is_ready;
	ImTextureID imgui_id;
};

struct MeshDesc {
	String name;
	Slice<Vertex> vertices;
	Slice<u16> indices;
	f32 bounds_min;
	f32 bounds_max;
	f32 bounds_rad;
};

struct MaterialProps {
	v3 ambient;
	v3 diffuse;
	v3 specular;
	f32 shininess;
};

struct Material {
	ShaderType type;
	u32 batch;
	MaterialProps props;
	TextureId base_color;
	u32 idx;
};

struct Glyph {
	Rng2u rect;
	f32 xoff, yoff, xadvance;
};

struct FontDesc {
	String name;
	u32 font_height;
	u8* data;
};

struct Font {
	u32 font_height;
	TextureId texture;
	Glyph glyphs[96];
};

struct DrawCall {
	v3 pos;
	v4 rot;
	v3 scale;
	Color color;
	MeshId mesh;
	MaterialId mat;
	ShaderType type;
	u32 flags;
};

struct DrawBatch {
	Gfx_PipelineState state;
	DArray<DrawCall> draws;
};

struct ShaderModuleWithPipelines {
	Gfx_Shader shd;
	Array<Gfx_Pipeline, 4> pipelines;
};

// typedef u32 AttachUsage;
// enum {
// 	AttachUsage_Color = 1<<0,
// 	AttachUsage_Depth = 1<<1,
// 	AttachUsage_Resolve = 1<<2,
// 	AttachUsage_Sampled = 1<<3,
// };

// struct AttachmentDesc {
// 	Gfx_Format fmt;
// 	u32 samples;
// 	// AttachUsage usage;
// };

struct Attachment {
	Gfx_Image image;
	Gfx_View view;
	Gfx_View sampled;
};

enum SizeMode {Size_Swapchain, Size_Absolute};

struct RenderTargetDesc {
	SizeMode size_mode;
	v2u size;
	b32 color;
	b32 coolor_sample;
	b32 depth;
	b32 depth_sample;
	u32 samples;
};

struct RenderTarget {
	RenderTargetDesc desc;
	v2u size;
	Attachment color;
	Attachment resolve;
	Attachment depth;
};

struct RenderPassDesc {
	RenderTarget* target;
	Gfx_LoadOp color_load;
	Gfx_StoreOp color_store;
	v4 color_clear;
	Gfx_LoadOp depth_load;
	Gfx_StoreOp depth_store;
	f32 depth_clear;
};

enum PushToGpuType {
	PushToGpuType_Texture,
	PushToGpuType_Mesh,
};

struct FinishedGpuTask {
	PushToGpuType type;
	TextureId texture_id;
	MeshId mesh_id;
	Gfx_Mesh mesh;
	Texture texture;
	u32 counter;
};

struct WaitingFont {
	FontId id;
	Font font;
};

#define MESH_LIST     \
	X(Dummy)             \
	X(Cube)              \
	X(MonkeyGlb)         \
	X(Triangle)          \
	X(Axis)              \
	X(Sphere)            \
	X(CubeGlft)          \
	X(GreeMan)           \
	X(Barrack)           \
	X(Bubble)            \
	X(T)                 \

#define TEXTURE_LIST  \
	X(Dummy)             \
	X(DummyCubeMap)      \
	X(Orange)            \
	X(Container)         \
	X(Barrack)           \
	X(Bricks)            \

#define MATERIAL_LIST \
	X(Dummy)             \
	X(Orange)            \
	X(Container)         \
	X(Axis)              \
	X(Line)              \
	X(Barrack)           \

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

enum FontEnum {
	Font_Dummy,
	Font_COUNT,
};

struct MemberDefinition {
	u32 type;
	String name;
	u64 offset;
};

struct Camera2 {
	v2 pos;
	f32 zoom;
	v2 zoom2;
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
	Color work;
	Color sleep;
	Color job;
	Color async;
	Color selected;
	Color hovered;
	Color border;
	Color text;
	Color text_dim;
	Color current_frame;
	Color frame_ok;     // < 16ms
	Color frame_warn;   // 16-20ms
	Color frame_bad;    // > 20ms
	Color mem_used;
	Color mem_committed;
	Color mem_cap;
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
	Color color;
	Rng3 aabb;
	MeshId mesh_id;
	MaterialId mat_id;
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

#define FIELD
#define UI_FIELD
#define PUSH_FIELD
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
	MeshId mesh;
	MaterialId mat;
	Color color;
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
	u32 serialized_idx;
};

typedef u32 ThingState;
enum {
	ThingState_OnFire = 1<<0,
	ThingState_Flying = 1<<1,
	ThingState_Poisoned = 1<<2,
};

#define THING_LIST \
	X(Thing_Cube0) \
	X(Thing_Cube1) \
	X(Thing_Monkey) \

enum ThingEnum {
	Thing_Null,
#define X(x) x,
	THING_LIST
#undef X
	ThingEnum_COUNT,
};

struct GltfInfo {
	MeshDesc mesh;
	Slice<Bone> bones;
	Slice<m4x4> inv_matrices;
	Animation anim;
};

struct ThingIter {
	Thing* data;
	u32 count;
	u32* dense;
	u8* gens;
	u32 idx;
	ThingId id;
};

struct GlobalState {
	Arena arena;
	Arena frame_arena;
	Alloc gpa;
	b32 should_hotreload;
	m4x4 view;
	m4x4 proj;
	m4x4 proj_view;
	m4x4 inv_view;
	m4x4 inv_proj;
	m4x4 inv_view_proj;
	v4 ambient_color;
	u32 fps;
	
	b32 visible;
	v3 euler;
	Array<v3, KB(1)> lines;

	Gfx_State gfx;

	MeshId meshes_ids[Mesh_COUNT];
	TextureId textures_ids[Texture_COUNT];
	MaterialId materials_ids[Material_COUNT];
	FontId fonts_ids[Font_COUNT];

	String asset_dir;
	String shader_dir;
	String shader_compiled_dir;
	String models_dir;
	String textures_dir;
	Map<TextureId, MaxTextures> str_to_texture_id;
	Map<MeshId, MaxMeshes> str_to_mesh_id;
	Map<MaterialId, MaxMaterials> str_to_material_id;
	Array<String, MaxTextures> texture_id_to_str;
	Array<String, MaxMeshes> mesh_id_to_str;
	Array<String, MaxMaterials> material_id_to_str;

	Slice<OS_Handle> shader_module_compilation_pids;
	Slice<String> shaders_to_compile;

	Array<WatchFile, 8> watches;
	Array<WatchDirectory, 8> watch_directories;

	struct {
		DebugWindow win;
		Camera2 root_cam;
		Camera2 frames_cam;
		Camera2 launch_cam;
		Camera2 mem_cam;
		ProfTabActive active_tab;
		ProfColors colors;
	} prof_win;
	DebugWindow game_win;
	b32 imgui_demo_open;

	struct {
		v3 pos;
		v3 dir;
		f32 yaw;
		f32 pitch;
		f32 fov;
		f32 accel;
		v3 vel;
		f32 vel_friction;
	} cam;

	b32 fps_camera;

	Pool<Thing, MaxThings, ThingId> things;
	SparseSetIdx<MaxThings> things_set;

	f32 scale;
	f32 old_scale;
	
	Map<ShaderModuleWithPipelines, Gfx_MaxShaders> shader_to_module;
	Array<Material, MaxMaterials> materials;
	Array<DrawBatch, 8> batches;
	Array<Bone, MaxBones> bones;
	Array<m4x4, MaxSkinMatrices> inv_matrices;
	Array<m4x4, MaxSkinMatrices> global_matrices;
	Array<m4x4, MaxSkinMatrices> skin_matrices;
	Array<Bone, MaxRuntimeBones> runtime_bones;
	Animation anim;

	Array<Gfx_Mesh, MaxMeshes> meshes;
	Array<Texture, MaxTextures> textures;
	Array<Font, MaxFonts> fonts;

	Gfx_Pipeline uber_pip;
	Gfx_Pipeline ui_rect_pip;
	Gfx_Pipeline cur_pip;
	Gfx_Pipeline prev_pip;
	Gfx_PipelineState prev_pip_state;
	Gfx_Sampler sampler;

	RenderTarget world_rt;

	Array<Vertex, MaxDebugLines> draw_lines;
	Array<Vertex, MaxDebugLines> draw_lines_persistent;
	Array<UI_Rect, MaxDebugLines> draw_rects;

	QueueSPSC<FinishedGpuTask, 32> finished_gpu_queue;
	Mutex finished_gpu_mutex;
	Mutex texture_mutex;
	QueueSPSC<WaitingFont, 32> waiting_fonts;
	Mutex font_mutex;

	TextureId cur_cubemap;

	Gfx_Buffer cpu_vert_buf;
	Gfx_Buffer cpu_drawcall_buf;
	Gfx_Buffer gpu_vert_buf;
	Gfx_Buffer gpu_index_buf;
	Gfx_Buffer cpu_state_buf;
	Gfx_Buffer cpu_materials_buf;
	Gfx_Buffer cpu_software_render_buf;
	Gfx_Buffer cpu_ui_rect_buf;

	Vertex* cpu_vertices;
	GpuState* gpu_state;
	u32* gpu_entities_indices;
	GpuMaterial* gpu_materials;
	GpuDrawCall* gpu_drawcalls;
	u32* gpu_software_render;
	GpuUI_Rect* gpu_ui_rects;
	m4x4* gpu_skin_matrices;
};

extern GlobalState* st;

DrawBatch make_draw_batch(Allocator alloc, Gfx_Pipeline pip);

Attachment make_attachment(v2u size, Gfx_Format fmt, u32 samples, b32 sampled);
void destroy_attachment(Attachment& attachment);
v2u resolve_size(RenderTargetDesc d);
RenderTarget make_render_target(RenderTargetDesc desc);
void destroy_render_target(RenderTarget rt);
void resize_render_target(RenderTarget& rt);
void begin_render_pass(RenderPassDesc p);

TextureDesc load_texture(String name);
TextureDesc load_cubemap(String dir);
MeshDesc load_mesh(Allocator arena, String name);
TextureId make_texture(TextureDesc desc);
TextureId make_texture_idle(TextureDesc desc);
MeshId make_mesh(MeshDesc desc);
MeshId make_mesh_idle(MeshDesc desc);
u32 make_pipeline_state(Gfx_PipelineState s);
MaterialId make_material(Material mat);
Gfx_Pipeline make_pipeline(String name, Gfx_PipelineDesc desc);
FontId make_font(FontDesc desc);
FontId make_font_idle(FontDesc desc);

u32 texture_descriptor_idx(TextureId id);
void set_cubemap(TextureId cubemap);

void update_mesh(MeshId mesh, MeshDesc desc);

void reload_shader(String name);
void shaders_compile(Allocator arena);
void shaders_compile_join();
void default_pip_state(Gfx_PipelineState* s);
void apply_pip_state_raw(Gfx_PipelineState s);
void apply_pip_state(Gfx_PipelineState s);

void r_init();
void r_shutdown();
void drawcall_set_mesh(GpuDrawCall& draw, Gfx_Mesh mesh);
void r_end();

void draw_mesh(MeshId mesh, MaterialId mat, v3 pos, u32 flags = 0);
void draw_mesh_trs(MeshId mesh, MaterialId mat, v3 pos, v4 rot, v3 scale, u32 flags = 0);
void draw_thing(ThingId id);

void draw_line(v3 a, v3 b, Color color);
void draw_line_persistent(v3 a, v3 b, Color color);
void draw_grid(v3 center, u32 slices, f32 spacing, Color color);
void draw_cuboid(Rng3 rng, Color color);
void draw_rect(Rng2 rect, Color color, f32 corner_radius = 0, f32 border_thickness = 0, f32 edge_softness = 0);
void draw_texture(Rng2 dst, Rng2 src, TextureId tex, Color color, f32 corner_radius = 0, f32 border_thickness = 0, f32 edge_softness = 0);
void draw_texture(Rng2 rect, TextureId tex);
void draw_text(FontId font, v2 pos, String str, Color color, u32 font_height);

void imgui_init();
void imgui_begin_frame();
void imgui_end_frame();
void imgui_image(TextureId id, v2 pos);
void imgui_draw_rect(ImDrawList* draw, Rng2 rect, Color col, f32 rounding = 0, ImDrawFlags flags = 0, f32 thickness = 1);
void imgui_draw_rect_filled(ImDrawList* draw, Rng2 rect, Color col, f32 rounding = 0, ImDrawFlags flags = 0);
void imgui_draw_line(ImDrawList* draw, v2 p0, v2 p1, Color col, f32 thickness = 1);
void imgui_draw_text(ImDrawList* draw, v2 pos, Color col, String fmt, ...);
void imgui_draw_text(ImDrawList* draw, f32 font_size, v2 pos, Color col, String fmt, ...);
void imgui_text(String fmt, ...);
v2 imgui_calc_text_size(String str);

void test();
f64 tsc_to_ms(u64 tsc);
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

void debug_window_apply_state(DebugWindow& win);
void debug_window_track_state(DebugWindow& win);
void debug_window_toggle_fullscreen(DebugWindow& win);
void dev_init();
void dev_update();

MeshDesc load_obj(Allocator arena, String name);
GltfInfo load_gltf(Allocator arena, String name, b32 is_glb);
v3 sample_v3(Sampler s, f32 time);
v4 sample_quat(Sampler s, f32 time);
void update_animation(Animation anim, Pose pose, f32 time);
f32 animation_duration(Animation anim);

JsParser js_parse_make(Allocator arena, String str);
b32 js_is_end(JsParser& p);
u8 js_peek(JsParser& p);
void js_skip_ws(JsParser& p);
String js_parse_str(JsParser& p);
f64 js_parse_number(JsParser& p);
JsVal js_parse(JsParser& p);

b32 js_val(JsVal val, String key, JsVal* out);
b32 js_bool(JsObj obj, String key, b32* out);
b32 js_f64(JsObj obj, String key, f64* out);
b32 js_f32(JsObj obj, String key, f32* out);
b32 js_u64(JsObj obj, String key, u64* out);
b32 js_u32(JsObj obj, String key, u32* out);
b32 js_str(JsObj obj, String key, String* out);
b32 js_array(JsObj obj, String key, Slice<JsVal*>* out);
b32 js_obj(JsObj obj, String key, JsObj* out);
b32 js_v3(JsObj obj, String key, v3* out);
b32 js_v4(JsObj obj, String key, v4* out);

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
v4 parse_v4(Parser& p);
String write_v3(Allocator alloc, v3 v);
String write_v4(Allocator alloc, v4 v);

v2 world_to_screen(Camera2 c, v2 p, v2 screen_center);
Rng2 world_to_screen(Camera2 c, Rng2 rect, v2 screen_center);
v2 world_to_screen2(Camera2 c, v2 p, v2 screen_center);
v2 screen_to_world(Camera2 c, v2 p, v2 screen_center);
v2 screen_to_world2(Camera2 c, v2 p, v2 screen_center);
void camera_zoom_at(Camera2& camera, v2 mouse, v2 screen_center, f32 factor);
void camera_zoom_at2(Camera2& c, v2 mouse, v2 screen_center, f32 factor);
void camera_zoom_at_pow_clamp(Camera2& c, v2 mouse, v2 screen_center, f32 factor);
void camera_pan(Camera2& camera, v2 screen_delta);
void camera_pan2(Camera2& camera, v2 screen_delta);
void camera_zoom_and_pan(Camera2& c, v2 screen_center, b32 linear = false);

void watch_add(String watch_name, WatchOp op);
void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags = OS_WatchFlag_Modify);
void watch_update();

b32 iter_next(ThingIter& it);
ThingIter things_begin();
ThingDesc default_thing_desc();
MaterialProps default_material_props();
Thing& get_thing(ThingId id);
void push_child_thing(ThingId parent, ThingId id);

String dumb_struct(Allocator arena, Slice<MemberDefinition> members, void* ptr, EntityFlags flags = {});
void dumb_struct_load(Slice<MemberDefinition> members, void* ptr, Parser* parser);

void init();
shared_function void com(HotReloadData* data);

MeshDesc generate_sphere(Allocator arena);
MeshDesc generate_grid(Allocator arena, u32 size, f32 step);

ThingId make_thing(ThingDesc desc);
void destroy_thing(ThingId id);
void select_obj();
void save_game_state();
void load_game_state();
void init_game();
void update_game();
