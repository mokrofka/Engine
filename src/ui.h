/* ============================================================================
 * ui.h -- a single-header immediate-mode UI library
 *
 * Style / lineage: Handmade Hero + Ryan Fleury's "ui.c" article series
 * (the RAD Debugger UI system). If you haven't read those articles, the
 * short version of the philosophy this code follows is:
 *
 *   1. IMMEDIATE TREE, PERSISTENT WIDGETS.
 *      Every frame you call code like ui_button(S("Save")) and the tree
 *      it produces is thrown away and rebuilt next frame. But each box
 *      is looked up by a hash of its id string in a persistent table,
 *      so a button "remembers" its hover/press animation (hot_t/active_t)
 *      and its last-computed screen rect across frames, even though the
 *      *code* that built it has no memory of its own.
 *
 *   2. ONE-FRAME-LAGGED HIT TESTING.
 *      When ui_button() runs, this frame's layout hasn't happened yet.
 *      So it hit-tests the mouse against the box's rect *from last
 *      frame*. This is standard practice in immediate mode UI and is
 *      imperceptible at interactive frame rates.
 *
 *   3. AUTOLAYOUT VIA SIZE KINDS, NOT PIXEL MATH.
 *      Every axis of every box has a UI_Size: Pixels, TextContent,
 *      PercentOfParent, or ChildrenSum. Layout is solved with a small,
 *      fixed number of tree passes (standalone sizes -> upward-dependent
 *      sizes -> downward-dependent sizes -> final positions) instead of
 *      the caller computing coordinates by hand.
 *
 *   4. STACKS FOR EVERYTHING SCOPED.
 *      Parenting, colors, and other "ambient" parameters are pushed and
 *      popped on stacks. A `for`-loop-with-single-iteration trick
 *      (UI_Parent(box) { ... }) gives us RAII-like scoping in plain C.
 *
 *   5. NO RENDERING BACKEND.
 *      ui_end_frame() produces a flat UI_DrawCmd array (filled/stroked
 *      rects + text runs with final screen rects). You feed that to
 *      software rasterization, OpenGL, SDL_Renderer, GDI, whatever --
 *      this library never calls malloc for pixels or touches a window.
 *
 * USAGE
 * -----
 *   In exactly ONE .c file:
 *       #define UI_IMPLEMENTATION
 *       #include "ui.h"
 *
 *   Everywhere else, just:
 *       #include "ui.h"
 *
 * See demo.c in the same directory for a full worked example (runs
 * headless -- no window system required -- and prints the resulting
 * layout + draw commands + interaction events to stdout).
 * ==========================================================================
 */

#ifndef UI_H
#define UI_H

#include <stdint.h>

/* ---------------------------------------------------------------------
 * Base types (Handmade-style short names)
 * ------------------------------------------------------------------- */
typedef uint8_t   U8;
typedef uint16_t  U16;
typedef uint32_t  U32;
typedef uint64_t  U64;
typedef int8_t    S8;
typedef int16_t   S16;
typedef int32_t   S32;
typedef int64_t   S64;
typedef float     F32;
typedef double    F64;
typedef S32       B32;

/* ---------------------------------------------------------------------
 * Strings: length-based, non-owning views. No null-termination games.
 * ------------------------------------------------------------------- */
typedef struct String8 {
    U8 *str;
    U64 size;
} String8;

#define str8_lit(s) (String8){(U8*)(s), sizeof(s) - 1}
#undef S
#define S(s) str8_lit(s) /* short alias used throughout demo code */

String8 str8(U8 *str, U64 size);
String8 str8_cstring(const char *cstr);
B32     str8_match(String8 a, String8 b);

/* ---------------------------------------------------------------------
 * Arena: linear allocator. One arena backs each frame's transient
 * scratch data (draw command array etc). Reset (not freed) every frame.
 * ------------------------------------------------------------------- */
typedef struct UI_Arena {
    U8 *base;
    U64 size;
    U64 pos;
} UI_Arena;

UI_Arena  ui_arena_make(U64 size);
void      ui_arena_release(UI_Arena *arena);
void     *ui_arena_push(UI_Arena *arena, U64 size);
void      ui_arena_reset(UI_Arena *arena);
#define   ui_push_array(arena, type, count) \
              (type *)ui_arena_push((arena), sizeof(type) * (count))

/* ---------------------------------------------------------------------
 * Math primitives
 * ------------------------------------------------------------------- */
typedef struct UI_Vec2 { F32 x, y; } UI_Vec2;
typedef struct UI_Vec4 { F32 x, y, z, w; } UI_Vec4; /* rgba, 0..1 */
typedef struct UI_Rect { F32 x0, y0, x1, y1; } UI_Rect;

static inline F32 ui_rect_width (UI_Rect r) { return r.x1 - r.x0; }
static inline F32 ui_rect_height(UI_Rect r) { return r.y1 - r.y0; }
static inline F32 ui_lerp  (F32 a, F32 b, F32 t) { return a + (b - a) * t; }
static inline F32 ui_clamp (F32 v, F32 lo, F32 hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline F32 ui_clamp01(F32 v) { return ui_clamp(v, 0.0f, 1.0f); }
static inline B32 ui_point_in_rect(UI_Vec2 p, UI_Rect r) {
    return p.x >= r.x0 && p.x < r.x1 && p.y >= r.y0 && p.y < r.y1;
}

/* ---------------------------------------------------------------------
 * Widget identity: a UI_Key is a hash of a box's id string, seeded by
 * its parent's key, so two boxes with the same *display* text in
 * different parts of the tree don't collide. The convention (borrowed
 * from Fleury's ui.c / Dear ImGui) is:
 *
 *     "Save##save_button"   -> displays "Save", hashes "save_button"
 *     "###unique_id"        -> displays nothing, hashes "unique_id"
 *     "Save"                -> displays "Save", hashes "Save" (fine as
 *                              long as it's not built inside a loop)
 * ------------------------------------------------------------------- */
typedef U64 UI_Key;
UI_Key   ui_key_from_string(UI_Key seed, String8 string);
String8  ui_display_string(String8 full);

/* ---------------------------------------------------------------------
 * Size: one axis's sizing rule. `strictness` is a 0..1 knob for how
 * willing this size is to shrink under a size-violation-resolution
 * pass; this reference implementation computes it but does not yet
 * *resolve* violations (documented limitation, see ui_layout_* below).
 * ------------------------------------------------------------------- */
typedef enum UI_SizeKind {
    UI_SizeKind_Null,
    UI_SizeKind_Pixels,
    UI_SizeKind_TextContent,
    UI_SizeKind_PercentOfParent,
    UI_SizeKind_ChildrenSum,
} UI_SizeKind;

typedef struct UI_Size {
    UI_SizeKind kind;
    F32 value;
    F32 strictness;
} UI_Size;

static inline UI_Size ui_size_px      (F32 v, F32 strictness) { return (UI_Size){UI_SizeKind_Pixels, v, strictness}; }
static inline UI_Size ui_size_text    (F32 strictness)         { return (UI_Size){UI_SizeKind_TextContent, 0, strictness}; }
static inline UI_Size ui_size_pct     (F32 v, F32 strictness)  { return (UI_Size){UI_SizeKind_PercentOfParent, v, strictness}; }
static inline UI_Size ui_size_children(F32 strictness)         { return (UI_Size){UI_SizeKind_ChildrenSum, 0, strictness}; }
static inline UI_Size ui_size_null(void)                       { return (UI_Size){UI_SizeKind_Null, 0, 0}; }

typedef enum UI_Axis2 { UI_Axis2_X = 0, UI_Axis2_Y = 1, UI_Axis2_COUNT = 2 } UI_Axis2;

/* ---------------------------------------------------------------------
 * Box flags
 * ------------------------------------------------------------------- */
typedef U32 UI_BoxFlags;
enum {
    UI_BoxFlag_Clickable          = 1 << 0,
    UI_BoxFlag_DrawText           = 1 << 1,
    UI_BoxFlag_DrawBorder         = 1 << 2,
    UI_BoxFlag_DrawBackground     = 1 << 3,
    UI_BoxFlag_HotAnimation       = 1 << 4,
    UI_BoxFlag_ActiveAnimation    = 1 << 5,
};

/* ---------------------------------------------------------------------
 * Box: the one and only node type. Split conceptually into three groups
 * of fields -- see comments inline. A box is looked up by key from a
 * persistent hash table, so the SAME struct instance is reused frame to
 * frame; only the "per-frame build" fields get overwritten each frame,
 * while "persistent" fields survive until the box is pruned for not
 * having been touched in a while (see ui_end_frame).
 * ------------------------------------------------------------------- */
typedef struct UI_Box UI_Box;
struct UI_Box {
    /* --- tree links: rebuilt from scratch every single frame --- */
    UI_Box *first, *last, *next, *prev, *parent;

    /* --- hash table links: persistent, managed by the key table --- */
    UI_Box *hash_next, *hash_prev;
    UI_Key  key;
    U64     last_frame_touched;

    /* --- per-frame build inputs, set by ui_box_make() and widgets --- */
    UI_BoxFlags flags;
    String8     string;
    UI_Size     semantic_size[UI_Axis2_COUNT];
    UI_Axis2    child_layout_axis;
    UI_Vec4     background_color;
    UI_Vec4     text_color;
    UI_Vec4     border_color;

    /* --- layout outputs: computed each frame, but read back next
     *     frame for one-frame-lagged hit testing before being
     *     overwritten -- this is the persistence that makes
     *     immediate-mode interaction possible --- */
    F32     computed_size[UI_Axis2_COUNT];
    F32     computed_rel_position[UI_Axis2_COUNT];
    UI_Rect rect;

    /* --- persistent per-widget state, survives across frames --- */
    F32 hot_t;    /* 0..1 hover animation value    */
    F32 active_t; /* 0..1 press/active animation   */
};

/* ---------------------------------------------------------------------
 * Signal: the result of asking "what happened to this box this frame?"
 * Every interactive widget returns one of these (or derives a simpler
 * bool/value from it, as ui_button/ui_checkbox/ui_slider do).
 * ------------------------------------------------------------------- */
typedef struct UI_Signal {
    UI_Box *box;
    B32 hovering;
    B32 pressed;   /* mouse went down this frame while hovering        */
    B32 released;  /* mouse went up this frame while this box active   */
    B32 clicked;   /* pressed and released cleanly on the same box     */
    B32 dragging;  /* box is active and mouse is currently held down   */
    UI_Vec2 drag_delta;
} UI_Signal;

/* ---------------------------------------------------------------------
 * Input: fed in once per frame by the caller. This library does not
 * know about your windowing system -- translate your platform's events
 * into this struct yourself.
 * ------------------------------------------------------------------- */
typedef struct UI_Input {
    UI_Vec2 mouse_pos;
    B32     mouse_down[3]; /* 0 = left, 1 = right, 2 = middle */
    F32     dt;            /* seconds since last frame, for animation  */
} UI_Input;

/* ---------------------------------------------------------------------
 * Draw commands: the entire output of this library. Render these
 * however you like.
 * ------------------------------------------------------------------- */
typedef enum UI_DrawCmdKind { UI_DrawCmd_Rect, UI_DrawCmd_Text } UI_DrawCmdKind;

typedef struct UI_DrawCmd {
    UI_DrawCmdKind kind;
    UI_Rect  rect;
    UI_Vec4  color;
    B32      filled;  /* rects only: filled quad vs 1px stroked outline */
    String8  text;     /* text only */
} UI_DrawCmd;

/* ---------------------------------------------------------------------
 * Style: default colors/metrics new boxes pick up. Push/pop stacks let
 * you scope overrides -- e.g. UI_BgColor(red) { ui_button(S("Delete")); }
 * ------------------------------------------------------------------- */
typedef struct UI_Style {
    F32 padding;
    F32 gap;
    F32 line_height;
    F32 text_pad;
    UI_Vec4 bg_color;
    UI_Vec4 text_color;
    UI_Vec4 border_color;
    UI_Vec4 hot_color;
    UI_Vec4 active_color;
    UI_Vec4 accent_color;
} UI_Style;

typedef F32 (*UI_TextMeasureFunc)(String8 text);

#define UI_MAX_PARENT_STACK 64
#define UI_MAX_COLOR_STACK  64
#define UI_KEY_TABLE_SIZE   4096
#define UI_STALE_FRAMES     2   /* frames a box may go untouched before it's pruned */

typedef struct UI_State0 {
    UI_Arena frame_arena;

    UI_Box  *box_table[UI_KEY_TABLE_SIZE];
    UI_Box  *root;

    UI_Box  *parent_stack[UI_MAX_PARENT_STACK];
    U32      parent_stack_top;

    UI_Vec4  bg_color_stack[UI_MAX_COLOR_STACK];
    U32      bg_color_stack_top;

    UI_Key   hot_key;
    UI_Key   active_key;

    UI_Input input;
    UI_Input prev_input;
    U64      frame_index;

    UI_DrawCmd *draw_cmds;
    U32         draw_cmd_count;
    U32         draw_cmd_cap;

    UI_Style style;
    UI_TextMeasureFunc text_measure;
} UI_State0;

/* Global current-context pointer, in the spirit of keeping the call
 * sites (ui_button(...) etc) ergonomic. If you need multiple
 * independent UI instances (e.g. multiple windows), keep several
 * UI_State0 objects around and call ui_set_current_state() to switch
 * between them before building each one's tree. */
extern UI_State0 *ui_state;

UI_State0 *ui_init(void);
void      ui_shutdown(UI_State0 *ui);
void      ui_set_current_state(UI_State0 *ui);

void      ui_begin_frame(UI_Input input, F32 window_w, F32 window_h);
void      ui_end_frame(void);

/* Tree building */
UI_Box   *ui_box_make(UI_BoxFlags flags, UI_Size size_x, UI_Size size_y, String8 string);
void      ui_push_parent(UI_Box *box);
void      ui_pop_parent(void);
UI_Box   *ui_top_parent(void);

/* Scoping helper -- "defer loop" trick for RAII-like blocks in C.
 * Usage:
 *     UI_Parent(ui_panel_begin(S("panel"), UI_Axis2_Y)) {
 *         ui_label(S("hello"));
 *     }
 * (ui_panel_begin/end already push/pop for you; UI_Parent is exposed
 * for building your own container widgets the same way.)          */
#define UI_Parent(box_expr) \
    for (UI_Box *_ui_parent_ = (box_expr), *_ui_once_ = (ui_push_parent(_ui_parent_), (UI_Box*)1); \
         _ui_once_ != 0; \
         _ui_once_ = 0, ui_pop_parent())

#define UI_BgColor(color) \
    for (B32 _ui_once_ = (ui_push_bg_color(color), 1); _ui_once_; _ui_once_ = 0, ui_pop_bg_color())

void ui_push_bg_color(UI_Vec4 color);
void ui_pop_bg_color(void);

/* Widgets */
UI_Signal ui_label   (String8 string);
UI_Signal ui_button  (String8 string);
B32       ui_checkbox(String8 string, B32 *value);
F32       ui_slider  (String8 string, F32 *value, F32 min, F32 max);
void      ui_spacer  (UI_Size size_along_parent_axis);

UI_Box   *ui_panel_begin(String8 string, UI_Axis2 child_layout_axis);
UI_Box   *ui_panel_begin_sized(String8 string, UI_Axis2 child_layout_axis, UI_Size size_x, UI_Size size_y);
void      ui_panel_end(void);
/* NOTE: ui_panel_begin() already pushes itself as parent (and
 * ui_panel_end() pops), so this loops around begin/end directly rather
 * than composing with UI_Parent (which would push a second time). */
#define UI_Panel(name, axis) \
    for (UI_Box *_ui_panel_ = ui_panel_begin((name), (axis)); \
         _ui_panel_ != NULL; \
         ui_panel_end(), _ui_panel_ = NULL)

#endif /* UI_H */

/* ============================================================================
 *                              IMPLEMENTATION
 * ==========================================================================
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

UI_State0 *ui_state = NULL;

/* --------------------------- strings --------------------------- */
String8 str8(U8 *str, U64 size) { String8 s; s.str = str; s.size = size; return s; }
String8 str8_cstring(const char *cstr) { return str8((U8*)cstr, strlen(cstr)); }
B32 str8_match(String8 a, String8 b) {
    if (a.size != b.size) return false;
    return memcmp(a.str, b.str, a.size) == 0;
}

static S64 str8_find_substr(String8 haystack, String8 needle) {
    if (needle.size == 0 || needle.size > haystack.size) return -1;
    for (U64 i = 0; i + needle.size <= haystack.size; i++) {
        if (memcmp(haystack.str + i, needle.str, needle.size) == 0) return (S64)i;
    }
    return -1;
}

String8 ui_display_string(String8 full) {
    String8 sep = str8_lit("##");
    S64 idx = str8_find_substr(full, sep);
    if (idx < 0) return full;
    return str8(full.str, (U64)idx);
}

/* Everything from the first "##" onward (inclusive) is the hash source,
 * so "Save##save_btn" and "Delete##save_btn" would collide -- by design,
 * this mirrors ImGui/Fleury conventions: put the *unique* part after ##. */
static String8 ui_hash_string(String8 full) {
    String8 sep = str8_lit("##");
    S64 idx = str8_find_substr(full, sep);
    if (idx < 0) return full;
    return str8(full.str + idx, full.size - (U64)idx);
}

/* --------------------------- arena --------------------------- */
UI_Arena ui_arena_make(U64 size) {
    UI_Arena a;
    a.base = (U8*)malloc(size);
    a.size = size;
    a.pos = 0;
    return a;
}
void ui_arena_release(UI_Arena *arena) { free(arena->base); arena->base = NULL; arena->size = arena->pos = 0; }
void *ui_arena_push(UI_Arena *arena, U64 size) {
    /* 8-byte align */
    U64 aligned = (arena->pos + 7u) & ~(U64)7u;
    assert(aligned + size <= arena->size && "UI_Arena out of space -- grow it in ui_init()");
    void *p = arena->base + aligned;
    arena->pos = aligned + size;
    memset(p, 0, size);
    return p;
}
void ui_arena_reset(UI_Arena *arena) { arena->pos = 0; }

/* --------------------------- hashing / keys --------------------------- */
static U64 ui_fnv1a(U64 seed, U8 *data, U64 size) {
    U64 h = seed ^ 0xcbf29ce484222325ULL;
    for (U64 i = 0; i < size; i++) {
        h ^= data[i];
        h *= 0x100000001b3ULL;
    }
    return h;
}
UI_Key ui_key_from_string(UI_Key seed, String8 string) {
    String8 hash_part = ui_hash_string(string);
    if (hash_part.size == 0) return 0; /* empty id -> not identity-tracked (e.g. spacers) */
    return ui_fnv1a(seed, hash_part.str, hash_part.size);
}

/* --------------------------- state / lifecycle --------------------------- */
UI_State0 *ui_init(void) {
    UI_State0 *ui = (UI_State0*)calloc(1, sizeof(UI_State0));
    ui->frame_arena = ui_arena_make(4 * 1024 * 1024);

    ui->style.padding      = 8.0f;
    ui->style.gap          = 4.0f;
    ui->style.line_height  = 18.0f;
    ui->style.text_pad     = 6.0f;
    ui->style.bg_color     = (UI_Vec4){0.16f, 0.16f, 0.18f, 1.0f};
    ui->style.text_color   = (UI_Vec4){0.92f, 0.92f, 0.92f, 1.0f};
    ui->style.border_color = (UI_Vec4){0.30f, 0.30f, 0.33f, 1.0f};
    ui->style.hot_color    = (UI_Vec4){0.26f, 0.26f, 0.30f, 1.0f};
    ui->style.active_color = (UI_Vec4){0.35f, 0.45f, 0.75f, 1.0f};
    ui->style.accent_color = (UI_Vec4){0.30f, 0.55f, 0.90f, 1.0f};

    /* Default text metric: monospace-ish average width. Replace with a
     * real font measurement function (e.g. via stb_truetype) for
     * anything beyond a demo. */
    extern F32 ui__default_text_measure(String8);
    ui->text_measure = ui__default_text_measure;

    ui_set_current_state(ui);
    return ui;
}

F32 ui__default_text_measure(String8 text) { return (F32)text.size * 7.0f; }

void ui_set_current_state(UI_State0 *ui) { ui_state = ui; }

static void ui_free_box_table(UI_State0 *ui) {
    for (U32 i = 0; i < UI_KEY_TABLE_SIZE; i++) {
        UI_Box *b = ui->box_table[i];
        while (b) { UI_Box *next = b->hash_next; free(b); b = next; }
        ui->box_table[i] = NULL;
    }
}

void ui_shutdown(UI_State0 *ui) {
    ui_arena_release(&ui->frame_arena);
    ui_free_box_table(ui);
    if (ui_state == ui) ui_state = NULL;
    free(ui);
}

/* --------------------------- box hash table --------------------------- */
static UI_Box *ui_box_from_key(UI_State0 *ui, UI_Key key) {
    if (key == 0) {
        /* Anonymous box (no id) -- never shared across frames, always
         * fresh. Used for spacers and other non-interactive filler. */
        UI_Box *b = ui_push_array(&ui->frame_arena, UI_Box, 1);
        b->key = 0;
        return b;
    }
    U32 slot = (U32)(key % UI_KEY_TABLE_SIZE);
    for (UI_Box *b = ui->box_table[slot]; b; b = b->hash_next) {
        if (b->key == key) return b;
    }
    UI_Box *b = (UI_Box*)calloc(1, sizeof(UI_Box));
    b->key = key;
    b->hash_next = ui->box_table[slot];
    if (ui->box_table[slot]) ui->box_table[slot]->hash_prev = b;
    ui->box_table[slot] = b;
    return b;
}

static void ui_prune_stale_boxes(UI_State0 *ui) {
    for (U32 i = 0; i < UI_KEY_TABLE_SIZE; i++) {
        UI_Box *b = ui->box_table[i];
        while (b) {
            UI_Box *next = b->hash_next;
            if (ui->frame_index - b->last_frame_touched > UI_STALE_FRAMES) {
                if (b->hash_prev) b->hash_prev->hash_next = b->hash_next;
                else ui->box_table[i] = b->hash_next;
                if (b->hash_next) b->hash_next->hash_prev = b->hash_prev;
                if (ui->hot_key == b->key) ui->hot_key = 0;
                if (ui->active_key == b->key) ui->active_key = 0;
                free(b);
            }
            b = next;
        }
    }
}

/* --------------------------- parent / color stacks --------------------------- */
void ui_push_parent(UI_Box *box) {
    assert(ui_state->parent_stack_top < UI_MAX_PARENT_STACK);
    ui_state->parent_stack[ui_state->parent_stack_top++] = box;
}
void ui_pop_parent(void) {
    assert(ui_state->parent_stack_top > 0);
    ui_state->parent_stack_top--;
}
UI_Box *ui_top_parent(void) {
    return ui_state->parent_stack_top > 0
        ? ui_state->parent_stack[ui_state->parent_stack_top - 1]
        : NULL;
}
void ui_push_bg_color(UI_Vec4 color) {
    assert(ui_state->bg_color_stack_top < UI_MAX_COLOR_STACK);
    ui_state->bg_color_stack[ui_state->bg_color_stack_top++] = color;
}
void ui_pop_bg_color(void) {
    assert(ui_state->bg_color_stack_top > 0);
    ui_state->bg_color_stack_top--;
}
static UI_Vec4 ui_current_bg_color(void) {
    return ui_state->bg_color_stack_top > 0
        ? ui_state->bg_color_stack[ui_state->bg_color_stack_top - 1]
        : ui_state->style.bg_color;
}

/* --------------------------- box building --------------------------- */
UI_Box *ui_box_make(UI_BoxFlags flags, UI_Size size_x, UI_Size size_y, String8 string) {
    UI_State0 *ui = ui_state;
    UI_Box *parent = ui_top_parent();
    UI_Key seed = parent ? parent->key : 0;
    UI_Key key = ui_key_from_string(seed, string);
    UI_Box *box = ui_box_from_key(ui, key);

    /* Tree links rebuilt fresh every frame */
    box->first = box->last = box->next = box->prev = NULL;
    box->parent = parent;
    if (parent) {
        if (!parent->first) { parent->first = parent->last = box; }
        else { parent->last->next = box; box->prev = parent->last; parent->last = box; }
    }

    box->flags = flags;
    box->string = string;
    box->semantic_size[UI_Axis2_X] = size_x;
    box->semantic_size[UI_Axis2_Y] = size_y;
    box->child_layout_axis = UI_Axis2_X;
    box->background_color = ui_current_bg_color();
    box->text_color = ui->style.text_color;
    box->border_color = ui->style.border_color;
    box->last_frame_touched = ui->frame_index;

    return box;
}

/* --------------------------- frame lifecycle --------------------------- */
void ui_begin_frame(UI_Input input, F32 window_w, F32 window_h) {
    UI_State0 *ui = ui_state;
    ui->prev_input = ui->input;
    ui->input = input;
    ui->frame_index++;
    ui->parent_stack_top = 0;
    ui->bg_color_stack_top = 0;
    ui_arena_reset(&ui->frame_arena);

    ui->draw_cmd_cap = 4096;
    ui->draw_cmds = ui_push_array(&ui->frame_arena, UI_DrawCmd, ui->draw_cmd_cap);
    ui->draw_cmd_count = 0;

    UI_Box *root = ui_box_make(0, ui_size_px(window_w, 1), ui_size_px(window_h, 1), str8_lit("###root"));
    root->child_layout_axis = UI_Axis2_Y;
    ui->root = root;
    ui_push_parent(root);
}

/* Pass 1: standalone sizes (Pixels, TextContent). Order-independent. */
static void ui_layout_standalone(UI_Box *box) {
    for (int axis = 0; axis < 2; axis++) {
        UI_Size sz = box->semantic_size[axis];
        if (sz.kind == UI_SizeKind_Pixels) {
            box->computed_size[axis] = sz.value;
        } else if (sz.kind == UI_SizeKind_TextContent) {
            String8 disp = ui_display_string(box->string);
            if (axis == UI_Axis2_X)
                box->computed_size[axis] = ui_state->text_measure(disp) + 2.0f * ui_state->style.text_pad;
            else
                box->computed_size[axis] = ui_state->style.line_height + 2.0f * ui_state->style.text_pad;
        }
    }
    for (UI_Box *c = box->first; c; c = c->next) ui_layout_standalone(c);
}

/* Pass 2: upward-dependent sizes (PercentOfParent). Parent before children.
 * NOTE (documented limitation): if the parent's size on this axis is
 * itself ChildrenSum, it hasn't been computed yet at this point in the
 * traversal -- percent-of-parent nested inside children-sum-on-the-
 * same-axis will read a stale/zero value. Fleury's real implementation
 * handles this with an extra fixup pass; omitted here for clarity. */
static void ui_layout_upward(UI_Box *box) {
    for (int axis = 0; axis < 2; axis++) {
        UI_Size sz = box->semantic_size[axis];
        if (sz.kind == UI_SizeKind_PercentOfParent) {
            F32 parent_size = box->parent ? box->parent->computed_size[axis] : box->computed_size[axis];
            box->computed_size[axis] = parent_size * sz.value;
        }
    }
    for (UI_Box *c = box->first; c; c = c->next) ui_layout_upward(c);
}

/* Pass 3: downward-dependent sizes (ChildrenSum). Children before parent. */
static void ui_layout_downward(UI_Box *box) {
    for (UI_Box *c = box->first; c; c = c->next) ui_layout_downward(c);
    for (int axis = 0; axis < 2; axis++) {
        UI_Size sz = box->semantic_size[axis];
        if (sz.kind == UI_SizeKind_ChildrenSum) {
            F32 sum = 0, mx = 0;
            U32 n = 0;
            for (UI_Box *c = box->first; c; c = c->next) {
                sum += c->computed_size[axis];
                if (c->computed_size[axis] > mx) mx = c->computed_size[axis];
                n++;
            }
            if ((UI_Axis2)axis == box->child_layout_axis) {
                F32 gaps = n > 1 ? (F32)(n - 1) * ui_state->style.gap : 0;
                box->computed_size[axis] = sum + gaps + 2.0f * ui_state->style.padding;
            } else {
                box->computed_size[axis] = mx + 2.0f * ui_state->style.padding;
            }
        }
    }
}

/* Pass 4: final rects. Stack children along child_layout_axis, starting
 * flush against padding on both axes (start-aligned; no cross-axis
 * centering/growth in this reference implementation). */
static void ui_layout_positions(UI_Box *box) {
    F32 pad = ui_state->style.padding;
    F32 gap = ui_state->style.gap;
    F32 cursor = pad;
    for (UI_Box *c = box->first; c; c = c->next) {
        F32 main = cursor;
        F32 cross = pad;
        if (box->child_layout_axis == UI_Axis2_X) {
            c->rect.x0 = box->rect.x0 + main;
            c->rect.y0 = box->rect.y0 + cross;
        } else {
            c->rect.x0 = box->rect.x0 + cross;
            c->rect.y0 = box->rect.y0 + main;
        }
        c->rect.x1 = c->rect.x0 + c->computed_size[UI_Axis2_X];
        c->rect.y1 = c->rect.y0 + c->computed_size[UI_Axis2_Y];
        cursor += c->computed_size[box->child_layout_axis] + gap;
        ui_layout_positions(c);
    }
}

static void ui_push_draw_cmd(UI_State0 *ui, UI_DrawCmd cmd) {
    if (ui->draw_cmd_count < ui->draw_cmd_cap) {
        ui->draw_cmds[ui->draw_cmd_count++] = cmd;
    }
}

static void ui_build_draw_cmds(UI_Box *box) {
    UI_State0 *ui = ui_state;
    if (box->flags & UI_BoxFlag_DrawBackground) {
        UI_Vec4 color = box->background_color;
        if ((box->flags & UI_BoxFlag_ActiveAnimation) && box->active_t > 0.001f)
            color = (UI_Vec4){ ui_lerp(color.x, ui->style.active_color.x, box->active_t),
                                ui_lerp(color.y, ui->style.active_color.y, box->active_t),
                                ui_lerp(color.z, ui->style.active_color.z, box->active_t),
                                1.0f };
        else if ((box->flags & UI_BoxFlag_HotAnimation) && box->hot_t > 0.001f)
            color = (UI_Vec4){ ui_lerp(color.x, ui->style.hot_color.x, box->hot_t),
                                ui_lerp(color.y, ui->style.hot_color.y, box->hot_t),
                                ui_lerp(color.z, ui->style.hot_color.z, box->hot_t),
                                1.0f };
        ui_push_draw_cmd(ui, (UI_DrawCmd){ UI_DrawCmd_Rect, box->rect, color, true, {0} });
    }
    if (box->flags & UI_BoxFlag_DrawBorder) {
        ui_push_draw_cmd(ui, (UI_DrawCmd){ UI_DrawCmd_Rect, box->rect, box->border_color, false, {0} });
    }
    if (box->flags & UI_BoxFlag_DrawText) {
        ui_push_draw_cmd(ui, (UI_DrawCmd){ UI_DrawCmd_Text, box->rect, box->text_color, false, ui_display_string(box->string) });
    }
    for (UI_Box *c = box->first; c; c = c->next) ui_build_draw_cmds(c);
}

void ui_end_frame(void) {
    UI_State0 *ui = ui_state;
    ui_pop_parent(); /* matches the root push in ui_begin_frame */

    ui_layout_standalone(ui->root);
    ui_layout_upward(ui->root);
    ui_layout_downward(ui->root);
    ui->root->rect = (UI_Rect){0, 0, ui->root->computed_size[UI_Axis2_X], ui->root->computed_size[UI_Axis2_Y]};
    ui_layout_positions(ui->root);

    ui_build_draw_cmds(ui->root);
    ui_prune_stale_boxes(ui);
}

/* --------------------------- interaction / signals --------------------------- */
static F32 ui_animate_towards(F32 current, F32 target, F32 dt, F32 rate_per_sec) {
    F32 dt_use = dt > 0.0f ? dt : (1.0f / 60.0f);
    F32 t = 1.0f - (F32)((double)1.0 / (1.0 + rate_per_sec * dt_use)); /* simple, frame-rate-robust ease */
    return ui_lerp(current, target, ui_clamp01(t));
}

static UI_Signal ui_signal_from_box(UI_Box *box) {
    UI_State0 *ui = ui_state;
    UI_Signal sig = {0};
    sig.box = box;

    if (!(box->flags & UI_BoxFlag_Clickable)) return sig;

    /* Hit test against LAST FRAME's rect -- this frame's layout for
     * `box` hasn't run yet. This is the one-frame lag mentioned up top. */
    B32 hovering = ui_point_in_rect(ui->input.mouse_pos, box->rect);
    B32 mouse_down_now  = ui->input.mouse_down[0];
    B32 mouse_down_prev = ui->prev_input.mouse_down[0];
    B32 mouse_pressed_edge  = mouse_down_now && !mouse_down_prev;
    B32 mouse_released_edge = !mouse_down_now && mouse_down_prev;

    sig.hovering = hovering;

    if (hovering) ui->hot_key = box->key;
    else if (ui->hot_key == box->key) ui->hot_key = 0;

    if (hovering && mouse_pressed_edge) {
        ui->active_key = box->key;
        sig.pressed = true;
    }

    B32 is_active = (ui->active_key == box->key);
    if (is_active && mouse_down_now) {
        sig.dragging = true;
        sig.drag_delta.x = ui->input.mouse_pos.x - ui->prev_input.mouse_pos.x;
        sig.drag_delta.y = ui->input.mouse_pos.y - ui->prev_input.mouse_pos.y;
    }
    if (is_active && mouse_released_edge) {
        sig.released = true;
        sig.clicked = hovering; /* only counts as a click if released back over the box */
        ui->active_key = 0;
    }

    F32 hot_target = (ui->hot_key == box->key) ? 1.0f : 0.0f;
    F32 active_target = (ui->active_key == box->key) ? 1.0f : 0.0f;
    box->hot_t = ui_animate_towards(box->hot_t, hot_target, ui->input.dt, 20.0f);
    box->active_t = ui_animate_towards(box->active_t, active_target, ui->input.dt, 25.0f);

    return sig;
}

/* --------------------------- widgets --------------------------- */
UI_Signal ui_label(String8 string) {
    UI_Box *box = ui_box_make(UI_BoxFlag_DrawText, ui_size_text(1), ui_size_text(1), string);
    return ui_signal_from_box(box); /* not clickable -- returns an all-false signal */
}

UI_Signal ui_button(String8 string) {
    UI_Box *box = ui_box_make(
        UI_BoxFlag_Clickable | UI_BoxFlag_DrawBorder | UI_BoxFlag_DrawBackground |
        UI_BoxFlag_DrawText | UI_BoxFlag_HotAnimation | UI_BoxFlag_ActiveAnimation,
        ui_size_text(1), ui_size_text(1), string);
    return ui_signal_from_box(box);
}

B32 ui_checkbox(String8 string, B32 *value) {
    UI_Box *box = ui_box_make(
        UI_BoxFlag_Clickable | UI_BoxFlag_DrawBorder | UI_BoxFlag_DrawBackground |
        UI_BoxFlag_HotAnimation | UI_BoxFlag_ActiveAnimation,
        ui_size_px(20, 1), ui_size_px(20, 1), string);
    UI_Signal sig = ui_signal_from_box(box);
    if (sig.clicked) *value = !*value;
    if (*value) box->background_color = ui_state->style.accent_color;
    return *value;
}

F32 ui_slider(String8 string, F32 *value, F32 min, F32 max) {
    UI_Box *track = ui_box_make(
        UI_BoxFlag_Clickable | UI_BoxFlag_DrawBorder | UI_BoxFlag_DrawBackground |
        UI_BoxFlag_HotAnimation | UI_BoxFlag_ActiveAnimation,
        ui_size_pct(1.0f, 0), ui_size_px(20, 1), string);
    UI_Signal sig = ui_signal_from_box(track);
    if (sig.dragging || sig.pressed) {
        F32 w = ui_rect_width(track->rect);
        F32 t = w > 0.0f ? (ui_state->input.mouse_pos.x - track->rect.x0) / w : 0.0f;
        t = ui_clamp01(t);
        *value = min + t * (max - min);
    }
    return *value;
}

void ui_spacer(UI_Size size_along_parent_axis) {
    UI_Box *parent = ui_top_parent();
    UI_Axis2 axis = parent ? parent->child_layout_axis : UI_Axis2_X;
    UI_Size sizes[2] = { ui_size_px(0, 0), ui_size_px(0, 0) };
    sizes[axis] = size_along_parent_axis;
    ui_box_make(0, sizes[UI_Axis2_X], sizes[UI_Axis2_Y], str8_lit(""));
}

UI_Box *ui_panel_begin_sized(String8 string, UI_Axis2 child_layout_axis, UI_Size size_x, UI_Size size_y) {
    UI_Box *box = ui_box_make(
        UI_BoxFlag_DrawBackground | UI_BoxFlag_DrawBorder,
        size_x, size_y, string);
    box->child_layout_axis = child_layout_axis;
    ui_push_parent(box);
    return box;
}
UI_Box *ui_panel_begin(String8 string, UI_Axis2 child_layout_axis) {
    /* Fits itself to its children on both axes. NOTE: if you put a
     * PercentOfParent child inside a ChildrenSum panel on the SAME
     * axis, you'll hit the ordering limitation described above
     * ui_layout_upward() -- use ui_panel_begin_sized() with an
     * explicit Pixels/PercentOfParent size on that axis instead, so
     * the size is known before children are laid out. */
    return ui_panel_begin_sized(string, child_layout_axis, ui_size_children(1), ui_size_children(1));
}
void ui_panel_end(void) { ui_pop_parent(); }

/* demo.c -- exercises ui.h without any real window or renderer.
 *
 * It runs several frames with a scripted fake UI_Input (mouse moving
 * onto a button, clicking it, then dragging a slider), builds a small
 * panel of widgets each frame, and prints:
 *   - the resulting layout tree (box name + rect)
 *   - the flat draw command list ui_end_frame() produced
 *   - interaction events (button clicks, checkbox/slider state)
 *
 * This is the kind of test you'd run before ever wiring up SDL/OpenGL/
 * GDI: prove the tree, layout and interaction logic is correct first,
 * then plug in a backend that just draws UI_DrawCmd rects/text.
 */

#include "stdio.h"

static void print_tree(UI_Box *box, int depth) {
    for (int i = 0; i < depth; i++) printf("  ");
    String8 disp = ui_display_string(box->string);
    printf("[%.*s] rect=(%.0f,%.0f)-(%.0f,%.0f)\n",
           (int)disp.size, disp.size ? (char*)disp.str : "root",
           box->rect.x0, box->rect.y0, box->rect.x1, box->rect.y1);
    for (UI_Box *c = box->first; c; c = c->next) print_tree(c, depth + 1);
}

static void print_draw_cmds(UI_State0 *ui) {
    for (U32 i = 0; i < ui->draw_cmd_count; i++) {
        UI_DrawCmd *cmd = &ui->draw_cmds[i];
        if (cmd->kind == UI_DrawCmd_Rect) {
            printf("  RECT  %s (%.0f,%.0f)-(%.0f,%.0f) rgba(%.2f,%.2f,%.2f,%.2f)\n",
                   cmd->filled ? "fill  " : "stroke",
                   cmd->rect.x0, cmd->rect.y0, cmd->rect.x1, cmd->rect.y1,
                   cmd->color.x, cmd->color.y, cmd->color.z, cmd->color.w);
        } else {
            printf("  TEXT  \"%.*s\" at (%.0f,%.0f)\n",
                   (int)cmd->text.size, (char*)cmd->text.str, cmd->rect.x0, cmd->rect.y0);
        }
    }
}

int ui_main(void) {
    ui_init();

    static B32 dark_mode = false;
    static F32 volume = 0.3f;
    S32 click_count = 0;

    /* A tiny scripted "input replay": frame -> (mouse_x, mouse_y, left_down) */
    struct { F32 x, y; B32 down; const char *note; } script[] = {
        {   0,   0, false, "mouse far away"                       },
        {  30,  60, false, "mouse hovers the button"               },
        {  30,  60, true,  "mouse presses down on the button"      },
        {  30,  60, false, "mouse releases on the button -> click" },
        {  60, 130, false, "mouse moves down to the slider"        },
        {  60, 130, true,  "mouse presses down on the slider"      },
        { 180, 130, true,  "dragging the slider to the right"      },
        { 180, 130, false, "release the slider"                    },
    };
    int n_frames = (int)(sizeof(script) / sizeof(script[0]));

    for (int f = 0; f < n_frames; f++) {
        UI_Input input = {0};
        input.mouse_pos = (UI_Vec2){ script[f].x, script[f].y };
        input.mouse_down[0] = script[f].down;
        input.dt = 1.0f / 60.0f;

        printf("================ frame %d: %s ================\n", f, script[f].note);

        ui_begin_frame(input, 400, 300);
        {
            /* ui_panel_begin_sized() pushes itself as the current parent;
             * ui_panel_end() pops it. Fixed pixel width here (rather than
             * ui_size_children) is what lets the Volume slider below use
             * PercentOfParent safely -- see the note on ui_panel_begin(). */
            ui_panel_begin_sized(S("main_panel##mp"), UI_Axis2_Y,
                                  ui_size_px(200, 1), ui_size_children(1));
            {
                ui_label(S("Settings"));
                ui_spacer(ui_size_px(4, 1));

                UI_Signal btn = ui_button(S("Click me##btn1"));
                if (btn.clicked) click_count++;

                ui_checkbox(S("Dark mode##dm"), &dark_mode);

                ui_spacer(ui_size_px(4, 1));
                ui_slider(S("Volume##vol"), &volume, 0.0f, 1.0f);
            }
            ui_panel_end();
        }
        ui_end_frame();

        printf("-- layout tree --\n");
        print_tree(ui_state->root, 0);
        printf("-- draw commands (%u) --\n", ui_state->draw_cmd_count);
        print_draw_cmds(ui_state);
        printf("-- state -- clicks=%d dark_mode=%d volume=%.3f\n\n",
               click_count, dark_mode, volume);
    }

    printf("FINAL: clicks=%d dark_mode=%d volume=%.3f\n", click_count, dark_mode, volume);

    ui_shutdown(ui_state);
    return 0;
}

#undef S
#define S(str) str_make((u8*)str, sizeof(str))
