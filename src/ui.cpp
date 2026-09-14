#include "com.h"
#define FONT_SIZE 24

String ui_display_string(String full) {
	u64 idx = str_find_needle(full, S("##"));
	if(idx < full.size) return {full.str, idx};
	return full;
}

String ui_hash_string(String full) {
	u64 idx = str_find_needle(full, S("###"));
	if(idx < full.size) return {full.str+idx, full.size-idx};
	return full;
}

u64 ui_key_from_string(String str, u64 seed) {
	String h = ui_hash_string(str);
	if(h.size == 0) return 0;
	return hash(h, seed);
}

f32 ui_text_measure(String text, R_FontId font, f32 font_height) {
	var f = pool_get(st->r.fonts, font);
	f32 width = 0;
	Loop(i, text.size) {
		u32 advance = f.glyphs[text.str[i]-32].xadvance;
		width += advance;
	}
	f32 scale = font_height / f.font_height;
	return width * scale;
}

UI_Size ui_size_px(f32 v) 	{ return {UI_SizeType_Pixels, v}; }
UI_Size ui_size_text() 				{ return {UI_SizeType_TextContent, 0}; }
UI_Size ui_size_pct(f32 v) { return {UI_SizeType_PercentOfParent, v}; }
UI_Size ui_size_children() { return {UI_SizeType_ChildrenSum, 0}; }
UI_Size ui_size_null()  			{ return {UI_SizeType_Null, 0}; }

void ui_push_parent(UI_Box* box) {
	var& g = st->ui;
	array_push(g.parent_stack, box);
}

void ui_pop_parent() {
	var& g = st->ui;
	array_pop(g.parent_stack);
}

UI_Box* ui_top_parent() {
	var& g = st->ui;
	return g.parent_stack.count > 0 ? array_back(g.parent_stack) : null;
}

void ui_push_font(R_FontId id) {
	var& g = st->ui;
	g.style.font = id;
}

void ui_push_bg_color(v4 color) {
	var& g = st->ui;
	array_push(g.bg_color_stack, color);
}

void ui_pop_bg_color() {
	var& g = st->ui;
	array_pop(g.bg_color_stack);
}

v4 ui_current_bg_color() {
	var& g = st->ui;
	return g.bg_color_stack.count > 0 ? array_back(g.bg_color_stack) : g.style.bg_color;
}

UI_Box* ui_box_make(UI_BoxFlags flags, UI_Size size_x, UI_Size size_y, String string) {
	var& g = st->ui;
	UI_Box* parent = ui_top_parent();
	u64 seed = parent ? parent->key : 0;
	u64 key = ui_key_from_string(string, seed);
	UI_Box* box = ui_box_from_key(key);
	box->first = box->last = box->next = box->prev = null;
	box->parent = parent;
	if(parent) dll_list_push_back(parent, box);
	box->flags = flags;
	box->string = string;
	box->semantic_size[UI_Axis2_X] = size_x;
	box->semantic_size[UI_Axis2_Y] = size_y;
	box->child_layout_axis = UI_Axis2_X;
	box->background_color = ui_current_bg_color();
	box->text_color = g.style.text_color;
	box->border_color = g.style.border_color;
	box->last_frame_touched = st->current_frame;
	return box;
}

UI_Box* ui_box_from_key(u64 key) {
	var& g = st->ui;
	if(key == 0) {
		UI_Box* b = push_struct(g.frame_arena, UI_Box);
		*b = {};
		return b;
	} 
	var [b, ok] = map_get(g.box_map, key);
	if(!ok) {
		b = pool_push(g.boxes, {.key = key});
		map_set(g.box_map, key, b);
		u64 d = b - g.boxes.data;
		if(d < UI_KEY_TABLE_SIZE) {
			sparse_set_push(g.active_boxes, d);
		}
	}
	Assert(b > g.boxes.data && b <= g.boxes.data+g.boxes.max_idx);
	return b;
}

void ui_prune_stale_boxes() {
	var& g = st->ui;
	Loop(i, g.active_boxes.count) {
		u32 idx = g.active_boxes.dense[i];
		UI_Box* b = &g.boxes.data[idx];
		if(st->current_frame - b->last_frame_touched > UI_STALE_FRAMES) {
			sparse_set_remove(g.active_boxes, idx);
			pool_remove(g.boxes, b);
			map_remove(g.box_map, b->key);
		}
	}
}

void ui_init() {
	var& g = st->ui;
	g.frame_arena = arena_make();
	g.style.padding = 8.0f;
	g.style.gap = 4.0f;
	g.style.line_height = 18.0f;
	g.style.text_pad = 6.0f;
	g.style.bg_color = v4(0.16f, 0.16f, 0.18f, 1.0f);
	g.style.text_color = v4(0.92f, 0.92f, 0.92f, 1.0f);
	g.style.border_color = v4(0.30f, 0.30f, 0.33f, 1.0f);
	g.style.hot_color = v4(0.26f, 0.26f, 0.30f, 1.0f);
	g.style.active_color = v4(0.35f, 0.45f, 0.75f, 1.0f);
	g.style.accent_color = v4(0.30f, 0.55f, 0.90f, 1.0f);
}

void ui_begin_frame() {
	var& g = st->ui;
	g.prev_input = g.input;
	g.input = {
		.dt = time_dt,
		.mouse_pos = os_mouse_pos(),
		.mouse_down[0] = os_mouse_is_button_down(MouseButton_Left),
		.mouse_down[1] = os_mouse_is_button_down(MouseButton_Right),
		.mouse_down[2] = os_mouse_is_button_down(MouseButton_Middle),
	};
	arena_clear(g.frame_arena);
	array_clear(g.draw_cmds);
	g.root = ui_box_make(0, ui_size_px(os_window_width()), ui_size_px(os_window_height()), "###root");
	g.root->child_layout_axis = UI_Axis2_Y;
	ui_push_parent(g.root);
}

void ui_end_frame() {
	var& g = st->ui;
	ui_pop_parent();
	ui_layout_standalone(g.root);
	ui_layout_upward(g.root);
	ui_layout_downward(g.root);
	g.root->rect = Rng2(v2(0, 0), v2(g.root->computed_size[UI_Axis2_X], g.root->computed_size[UI_Axis2_Y]));
	ui_layout_positions(g.root);
	ui_build_draw_cmds(g.root);
	ui_prune_stale_boxes();
}

void ui_layout_standalone(UI_Box* box) {
	var& g = st->ui;
	LoopEnum(axis, UI_Axis2) {
		UI_Size sz = box->semantic_size[axis];
		if(sz.type == UI_SizeType_Pixels) {
			box->computed_size[axis] = sz.value;
		} else if(sz.type == UI_SizeType_TextContent) {
			if(axis == UI_Axis2_X)
				box->computed_size[axis] = ui_text_measure(ui_display_string(box->string), g.style.font, FONT_SIZE) + 2.0f * g.style.text_pad;
			else
				box->computed_size[axis] = g.style.line_height + 2.0f * g.style.text_pad;
		}
	}
	LoopNode(it, box->first) {
		ui_layout_standalone(it);
	}
}

void ui_layout_upward(UI_Box* box) {
	LoopEnum(axis, UI_Axis2) {
		UI_Size sz = box->semantic_size[axis];
		if(sz.type == UI_SizeType_PercentOfParent) {
			f32 parent_size = box->parent ? box->parent->computed_size[axis] : box->computed_size[axis];
			box->computed_size[axis] = parent_size * sz.value;
		}
	}
	LoopNode(it, box->first) {
		ui_layout_upward(it);
	}
}

void ui_layout_downward(UI_Box* box) {
	var& g = st->ui;
	LoopNode(it, box->first) {
		ui_layout_downward(it);
	}
	LoopEnum(axis, UI_Axis2) {
		UI_Size sz = box->semantic_size[axis];
		if(sz.type == UI_SizeType_ChildrenSum) {
			f32 sum = 0, mx = 0;
			u32 n = 0;
			LoopNode(it, box->first) {
				if(it->semantic_size[axis].type == UI_SizeType_PercentOfParent) continue;
				sum += it->computed_size[axis];
				mx = Max(mx, it->computed_size[axis]);
				n++;
			}
			if(axis == box->child_layout_axis) {
				f32 gaps = n > 1 ? (f32)(n - 1) * g.style.gap : 0;
				box->computed_size[axis] = sum + gaps + 2.0f * g.style.padding;
			} else {
				box->computed_size[axis] = mx + 2.0f * g.style.padding;
			}
		}
	}
}

void ui_layout_positions(UI_Box* box) {
	var& g = st->ui;
	f32 pad = g.style.padding;
	f32 gap = g.style.gap;
	f32 cursor = pad;
	LoopNode(it, box->first) {
		f32 main = cursor;
		f32 cross = pad;
		if(box->child_layout_axis == UI_Axis2_X) {
			it->rect.x0 = box->rect.x0 + main;
			it->rect.y0 = box->rect.y0 + cross;
		} else {
			it->rect.x0 = box->rect.x0 + cross;
			it->rect.y0 = box->rect.y0 + main;
		}
		it->rect.x1 = it->rect.x0 + it->computed_size[UI_Axis2_X];
		it->rect.y1 = it->rect.y0 + it->computed_size[UI_Axis2_Y];
		cursor += it->computed_size[box->child_layout_axis] + gap;
		ui_layout_positions(it);
	}
}

void ui_build_draw_cmds(UI_Box* box) {
	var& g = st->ui;
	if(box->flags & UI_BoxFlag_DrawBackground) {
		v4 color = box->background_color;
		if((box->flags & UI_BoxFlag_ActiveAnimation) && box->active_t > 0.001f)
			color = v3_to_v4(v3_lerp(color.xyz, box->active_t, g.style.active_color.xyz), 1);
		else if((box->flags & UI_BoxFlag_HotAnimation) && box->hot_t > 0.001f)
			color = v3_to_v4(v3_lerp(color.xyz, box->hot_t, g.style.hot_color.xyz), 1);
		array_push(g.draw_cmds, {UI_DrawCmdType_Rect, box->rect, color, true});
		r_draw_rect(box->rect, color, 0,0,0);
	}
	if(box->flags & UI_BoxFlag_DrawBorder) {
		array_push(g.draw_cmds, {UI_DrawCmdType_Rect, box->rect, box->border_color});
		r_draw_rect(box->rect, box->border_color, 0,4,0);
	}
	if(box->flags & UI_BoxFlag_DrawText) {
		array_push(g.draw_cmds, {UI_DrawCmdType_Text, box->rect, box->text_color, false, ui_display_string(box->string)});
		r_draw_text_ext(g.style.font, v2(box->rect.x0, box->rect.y1), box->string, box->text_color, FONT_SIZE);
	}
	LoopNode(it, box->first)
		ui_build_draw_cmds(it);
}

f32 ui_animate_towards(f32 current, f32 target, f32 dt, f32 rate_per_sec) {
	// f32 t = 1.0f - (f32)((f64)1.0 / (1.0 + rate_per_sec * dt)); /* simple, frame-rate-robust ease */
	// return Lerp(current, Clamp01(t), target);
	return exp_decay(current, target, rate_per_sec, dt);
}

UI_Signal ui_signal_from_box(UI_Box* box) {
	var& g = st->ui;
	UI_Signal sig = {};
	sig.box = box;

	if(!(box->flags & UI_BoxFlag_Clickable)) return sig;

	/* Hit test against LAST FRAME's rect -- this frame's layout for
	 * `box` hasn't run yet. This is the one-frame lag mentioned up top. */
	b32 hovering = rng2_contains(box->rect, g.input.mouse_pos);
	b32 mouse_down_now = g.input.mouse_down[0];
	b32 mouse_down_prev = g.prev_input.mouse_down[0];
	b32 mouse_pressed_edge = mouse_down_now && !mouse_down_prev;
	b32 mouse_released_edge = !mouse_down_now && mouse_down_prev;

	sig.hovering = hovering;

	if(hovering)
		g.hot_key = box->key;
	else if(g.hot_key == box->key)
		g.hot_key = 0;

	if(hovering && mouse_pressed_edge) {
		g.active_key = box->key;
		sig.pressed = true;
	}

	b32 is_active = (g.active_key == box->key);
	if(is_active && mouse_down_now) {
		sig.dragging = true;
		sig.drag_delta = g.input.mouse_pos - g.prev_input.mouse_pos;
	}
	if(is_active && mouse_released_edge) {
		sig.released = true;
		sig.clicked = hovering; /* only counts as a click if released back over the box */
		g.active_key = 0;
	}

	f32 hot_target = (g.hot_key == box->key) ? 1.0f : 0.0f;
	f32 active_target = (g.active_key == box->key) ? 1.0f : 0.0f;
	box->hot_t = ui_animate_towards(box->hot_t, hot_target, g.input.dt, 10.0f);
	box->active_t = ui_animate_towards(box->active_t, active_target, g.input.dt, 15.0f);

	return sig;
}

UI_Signal ui_label(String string) {
	UI_Box* box = ui_box_make(UI_BoxFlag_DrawText, ui_size_text(), ui_size_text(), string);
	return ui_signal_from_box(box); /* not clickable -- returns an all-false signal */
}

UI_Signal ui_button(String string) {
	UI_Box* box = ui_box_make(
		UI_BoxFlag_Clickable | UI_BoxFlag_DrawBorder | UI_BoxFlag_DrawBackground |
		UI_BoxFlag_DrawText | UI_BoxFlag_HotAnimation | UI_BoxFlag_ActiveAnimation,
		ui_size_text(), ui_size_text(), string);
	return ui_signal_from_box(box);
}

b32 ui_checkbox(String string, b32* value) {
	var& g = st->ui;
	UI_Box* box = ui_box_make(
		UI_BoxFlag_Clickable | UI_BoxFlag_DrawBorder | UI_BoxFlag_DrawBackground |
		UI_BoxFlag_HotAnimation | UI_BoxFlag_ActiveAnimation,
		ui_size_px(20), ui_size_px(20), string);
	UI_Signal sig = ui_signal_from_box(box);
	if(sig.clicked) *value = !*value;
	if(*value) box->background_color = g.style.accent_color;
	return *value;
}

f32 ui_slider(String string, f32* value, f32 min, f32 max) {
	var& g = st->ui;
	UI_Box* track = ui_box_make(
		UI_BoxFlag_Clickable | UI_BoxFlag_DrawBorder | UI_BoxFlag_DrawBackground |
		UI_BoxFlag_HotAnimation | UI_BoxFlag_ActiveAnimation,
		ui_size_pct(1.0f), ui_size_px(20), string);
	UI_Signal sig = ui_signal_from_box(track);
	if(sig.dragging || sig.pressed) {
		f32 w = rng2_width(track->rect);
		f32 t = w > 0.0f ? (g.input.mouse_pos.x - track->rect.x0) / w : 0.0f;
		t = Clamp01(t);
		*value = min + t * (max - min);
	}
	return *value;
}

void ui_spacer(UI_Size size_along_parent_axis) {
	UI_Box* parent = ui_top_parent();
	UI_Axis2 axis = parent ? parent->child_layout_axis : UI_Axis2_X;
	UI_Size sizes[2] = {ui_size_px(0), ui_size_px(0)};
	sizes[axis] = size_along_parent_axis;
	ui_box_make(0, sizes[UI_Axis2_X], sizes[UI_Axis2_Y], {});
}

UI_Box* ui_panel_begin_sized(String string, UI_Axis2 child_layout_axis, UI_Size size_x, UI_Size size_y) {
	UI_Box* box = ui_box_make(
		UI_BoxFlag_DrawBackground | UI_BoxFlag_DrawBorder,
		size_x, size_y, string);
	box->child_layout_axis = child_layout_axis;
	ui_push_parent(box);
	return box;
}

UI_Box* ui_panel_begin(String string, UI_Axis2 child_layout_axis) {
	return ui_panel_begin_sized(string, child_layout_axis, ui_size_children(), ui_size_children());
}
void ui_panel_end() { ui_pop_parent(); }

