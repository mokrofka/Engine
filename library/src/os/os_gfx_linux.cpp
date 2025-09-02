#include "lib.h"

#if OS_LINUX

#undef global
#include "vendor/xdg-shell-client-protocol.h"
#include <wayland-client.h>
#define global static

#include <linux/input-event-codes.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "event.h"

internal u32 keycode_translate(u32 code);
internal u32 mouse_buttoncode_translate(u32 code);

internal void wl_seat_capabilities(void* data, wl_seat* seat, u32 capabilities);

internal void xdg_shell_ping(void* data, xdg_wm_base* shell, u32 serial);

struct WaylandState {
  // Global interfaces
  wl_display* wl_display;
  wl_registry* wl_registry;

  // Global interfaces
  wl_compositor* wl_compositor;
  wl_shm* wl_shm;
  wl_seat* wl_seat;
  xdg_wm_base* xdg_wm_base;

  // Callbaacks
  // wl_callback_listener wl_callback_listener;

  // Input
  wl_pointer* wl_pointer;
  wl_keyboard* wl_keyboard;

  wl_surface* wl_surface;
  xdg_surface* xdg_surface;
  xdg_toplevel* xdg_toplevel;

  // wl_buffer* wl_buffer;
  // Arena* resize_arena;
  // OS_Handle fd;
  
  i32 width = 900;
  i32 height = 900;
  i32 channel = 4;
  i32 stride = width*channel;
  u8* pixels;
  b8 is_running = true;
  b8 is_showned;
  
  ////////////////////////////////////////////////////////////////////////
  // Input
  struct KeyboardState {
    b8 keys[256];
  };
  struct MouseState {
    u16 x;
    u16 y;
    b8 buttons[MouseButton_COUNT];
  };
  struct {
    KeyboardState keyboard_current;
    KeyboardState keyboard_previous;
    MouseState mouse_current;
    MouseState mouse_previous;
  } input;
};

global WaylandState st;
#undef global // because of: wl_registry_listener.global;

internal void wl_registry_global_handler(void* data, wl_registry* registry, u32 name, const char* interface, u32 version) {
  // Info("Compositor says: interface %s (ver %u) with id %u", String(interface), version, name);

  if (str_match(interface, wl_compositor_interface.name)) {
    Assign(st.wl_compositor, wl_registry_bind(registry, name, &wl_compositor_interface, version));
  }
  else if (str_match(interface, wl_shm_interface.name)) {
    // Assign(st.wl_shm, wl_registry_bind(registry, name, &wl_shm_interface, 1));
  }
  else if (str_match(interface, wl_seat_interface.name)) {
    Assign(st.wl_seat, wl_registry_bind(registry, name, &wl_seat_interface, 1));
    local wl_seat_listener wl_seat_listener = {
      .capabilities = wl_seat_capabilities,
      .name = [](void* data, wl_seat* seat, const char* name) {},
    };
    wl_seat_add_listener(st.wl_seat, &wl_seat_listener, 0);
  }
  else if (str_match(interface, xdg_wm_base_interface.name)) {
    Assign(st.xdg_wm_base, wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
    local xdg_wm_base_listener xdg_wm_base_listener = {
      .ping = [](void* data, xdg_wm_base* shell, u32 serial){ xdg_wm_base_pong(shell, serial); },
    };
    xdg_wm_base_add_listener(st.xdg_wm_base, &xdg_wm_base_listener, 0);
  }
}

internal void wl_key(void* data, wl_keyboard* keyboard, u32 serial, u32 time, u32 key, u32 state) {
  u32 my_key = keycode_translate(key);
  // Info("key '%c' %s.\n", (char)my_key, state ? String("pressed") : String("released"));
  st.input.keyboard_current.keys[my_key] = state;
}

void wl_pointer_motion(void* data, wl_pointer* pointer, u32 time, wl_fixed_t sx, wl_fixed_t sy) {
  f32 x = wl_fixed_to_double(sx);
  f32 y = wl_fixed_to_double(sy);
  st.input.mouse_current.x = x;
  st.input.mouse_current.y = y;

  // Info("prev x: %i", st.input.mouse_previous.x);

  // Info("x = %f, y = %f", x, y);
}

void wl_mouse_button(void* data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
  #define WL_LBUTTON 272
  #define WL_RBUTTON 273
  u32 my_button = mouse_buttoncode_translate(button);
  st.input.mouse_current.buttons[my_button] = state;
  // if (my_button == MouseButton_Left) {
  //   Info("Left button was pressed");
  // }
  // if (my_button == MouseButton_Right) {
  //   Info("Right button was pressed");
  // }
}

internal void wl_seat_capabilities(void* data, wl_seat* seat, u32 capabilities) {
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    st.wl_keyboard = wl_seat_get_keyboard(seat);
    local wl_keyboard_listener wl_keyboard_listener = {
      .keymap = [](void* data, wl_keyboard* keyboard, u32 format, i32 fd, u32 size){},
      .enter = [](void* data, wl_keyboard* keyboard, u32 serial, wl_surface* surface, wl_array* keys){},
      .leave = [](void* data, wl_keyboard* keyboard, u32 serial, wl_surface* surface){},
      .key = wl_key,
      .modifiers = [](void* data, wl_keyboard* keyboard, u32 serial, u32 mods_depressed, u32 mods_latched, u32 mods_locked, u32 group){},
    };
    wl_keyboard_add_listener(st.wl_keyboard, &wl_keyboard_listener, data);
  }
  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    st.wl_pointer = wl_seat_get_pointer(seat);
    local wl_pointer_listener pointer_listener = {
      .enter = [](void* data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y){},
      .leave = [](void* data, struct wl_pointer* wl_pointer, uint32_t serial, struct wl_surface* surface){},
      .motion = wl_pointer_motion,
      .button = wl_mouse_button,
    };
    wl_pointer_add_listener(st.wl_pointer, &pointer_listener, data);
  }
}

// internal void wl_draw() {
//   wl_surface_attach(st.wl_surface, st.wl_buffer, 0, 0);
//   wl_surface_damage(st.wl_surface, 0, 0, st.width, st.height);
//   wl_surface_commit(st.wl_surface);
// }

// internal void wl_frame_new(void* data, wl_callback* callback, u32 a) {
//   wl_callback_destroy(callback);
//   callback = wl_surface_frame(st.wl_surface);
//   wl_callback_add_listener(callback, &st.wl_callback_listener, null);
//   wl_draw();
// }

internal void resize() {
  // u64 size = st.stride * st.height;

  // arena_clear(st.resize_arena);
  // push_buffer(st.resize_arena, size);

  // wl_shm_pool* pool = wl_shm_create_pool(st.wl_shm, st.fd, size + ARENA_HEADER_SIZE);
  // st.wl_buffer = wl_shm_pool_create_buffer(pool, ARENA_HEADER_SIZE, st.width, st.height, st.stride, WL_SHM_FORMAT_ARGB8888);
  // wl_shm_pool_destroy(pool);
}

internal void xdg_surface_configure(void* data, xdg_surface* xdg_surface, u32 serial) {
  xdg_surface_ack_configure(xdg_surface, serial);

  if (!st.is_showned) {
    st.is_showned = true;
    resize();
  }

  // wl_draw();
}

internal void xdg_top_configure(void* data, xdg_toplevel* top, i32 width, i32 height, wl_array* stat) {
  if (!width || !height) {
    return;
  }

  if (st.width != width || st.height != height) {
    st.width = width;
    st.height = height;
    st.stride = st.width*st.channel;
    resize();
    EventContext data = {
      .i32[0] = width,
      .i32[1] = height,
    };
    event_fire(EventCode_Resized, null, data);
  }
}

void os_gfx_init() {
  Scratch scratch;

  // st.resize_arena = arena_shm_alloc(&st.fd);
  // st.pixels = push_buffer(st.resize_arena, st.height*st.stride);

  st.wl_display = wl_display_connect(null);
  st.wl_registry = wl_display_get_registry(st.wl_display);
  local wl_registry_listener wl_registry_listener = {
    .global = wl_registry_global_handler,
  };
  wl_registry_add_listener(st.wl_registry, &wl_registry_listener, null);
  wl_display_roundtrip(st.wl_display);

  st.wl_surface = wl_compositor_create_surface(st.wl_compositor);
  // wl_callback* callback = wl_surface_frame(st.wl_surface);
  // st.wl_callback_listener = {
  //   .done = wl_frame_new,
  // };
  // wl_callback_add_listener(callback, &st.wl_callback_listener, null);

  st.xdg_surface = xdg_wm_base_get_xdg_surface(st.xdg_wm_base, st.wl_surface);
  local xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure
  };
  xdg_surface_add_listener(st.xdg_surface, &xdg_surface_listener, null);

  st.xdg_toplevel = xdg_surface_get_toplevel(st.xdg_surface);
  local xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_top_configure,
    .close = [](void* data, xdg_toplevel* top){ st.is_running = false; },
  };
  xdg_toplevel_add_listener(st.xdg_toplevel, &xdg_toplevel_listener, null);

  xdg_toplevel_set_title(st.xdg_toplevel, "wayland client");

  wl_surface_commit(st.wl_surface);
}

void os_gfx_shutdown() {
  if (st.wl_keyboard) {
    wl_keyboard_destroy(st.wl_keyboard);
  }
  wl_seat_release(st.wl_seat);
  // if (st.wl_buffer) {
  //   wl_buffer_destroy(st.wl_buffer);
  // }
  xdg_toplevel_destroy(st.xdg_toplevel);
  xdg_surface_destroy(st.xdg_surface);
  wl_surface_destroy(st.wl_surface);
  wl_display_disconnect(st.wl_display);
}

void os_pump_messages()            { wl_display_dispatch(st.wl_display); }
b32 os_window_should_close()       { return st.is_running; }

u8*  os_window_get_buffer()        { return st.pixels; }
v2i  os_get_window_size()          { return v2i(st.width, st.height); }
v2i  os_get_mouse_pos()            { return v2i(st.input.mouse_current.x, st.input.mouse_current.y); }
void* os_get_wl_display()          { return st.wl_display; }
void* os_get_wl_surface()          { return st.wl_surface; }

////////////////////////////////////////////////////////////////////////
// keyboard
void os_input_update() {
  MemCopyStruct(&st.input.keyboard_previous, &st.input.keyboard_current);
  MemCopyStruct(&st.input.mouse_previous, &st.input.mouse_current);
}

b32 os_is_key_down(Key key)       { return st.input.keyboard_current.keys[key] == true; }
b32 os_is_key_up(Key key)         { return st.input.keyboard_current.keys[key] == false; }
b32 os_was_key_down(Key key)      { return st.input.keyboard_previous.keys[key] == true; }
b32 os_was_key_up(Key key)        { return st.input.keyboard_previous.keys[key] == false; }
b32 os_is_key_pressed(Key key)    { return os_is_key_down(key) && os_was_key_up(key); }
b32 os_is_key_released(Key key)   { return os_is_key_up(key) && os_was_key_down(key); }

////////////////////////////////////////////////////////////////////////
// mouse
b32 os_is_button_down(MouseButtons button)          { return st.input.mouse_current.buttons[button] == true; }
b32 os_is_button_up(MouseButtons button)            { return st.input.mouse_current.buttons[button] == false; }
b32 os_was_button_down(MouseButtons button)         { return st.input.mouse_previous.buttons[button] == true; }
b32 os_was_button_up(MouseButtons button)           { return st.input.mouse_previous.buttons[button] == false; }
b32 os_is_button_pressed(MouseButtons button)       { return os_is_button_down(button) && os_was_button_up(button); }
b32 os_is_button_released(MouseButtons button)      { return os_is_button_up(button) && os_was_button_down(button); }

internal u32 keycode_translate(u32 code) {
  switch (code) {
    // Control keys
    case KEY_BACKSPACE:   return Key_Backspace;
    case KEY_ENTER:       return Key_Enter;
    case KEY_TAB:         return Key_Tab;
    case KEY_LEFTSHIFT:   return Key_LShift;
    case KEY_RIGHTSHIFT:  return Key_RShift;
    case KEY_LEFTCTRL:    return Key_LControl;
    case KEY_RIGHTCTRL:   return Key_RControl;
    case KEY_LEFTALT:     return Key_LAlt;
    case KEY_RIGHTALT:    return Key_RAlt;
    case KEY_ESC:         return Key_Escape;
    case KEY_CAPSLOCK:    return Key_Capslock;

    // Navigation
    case KEY_SPACE:       return Key_Space;
    case KEY_PAGEUP:      return Key_Pageup;
    case KEY_PAGEDOWN:    return Key_Pagedown;
    case KEY_END:         return Key_End;
    case KEY_HOME:        return Key_Home;
    case KEY_LEFT:        return Key_Left;
    case KEY_UP:          return Key_Up;
    case KEY_RIGHT:       return Key_Right;
    case KEY_DOWN:        return Key_Down;

    // Special keys
    case KEY_PAUSE:       return Key_Pause;
    case KEY_PRINT:       return Key_Print;
    case KEY_SYSRQ:       return Key_Printscreen;
    case KEY_DELETE:      return Key_Delete;
    case KEY_LEFTMETA:    return Key_Lsuper;
    case KEY_RIGHTMETA:   return Key_Rsuper;
    case KEY_MENU:        return Key_Apps;
    case KEY_NUMLOCK:     return Key_Numlock;

    // Numbers
    case KEY_0:           return Key_0;
    case KEY_1:           return Key_1;
    case KEY_2:           return Key_2;
    case KEY_3:           return Key_3;
    case KEY_4:           return Key_4;
    case KEY_5:           return Key_5;
    case KEY_6:           return Key_6;
    case KEY_7:           return Key_7;
    case KEY_8:           return Key_8;
    case KEY_9:           return Key_9;

    // Letters
    case KEY_A:           return Key_A;
    case KEY_B:           return Key_B;
    case KEY_C:           return Key_C;
    case KEY_D:           return Key_D;
    case KEY_E:           return Key_E;
    case KEY_F:           return Key_F;
    case KEY_G:           return Key_G;
    case KEY_H:           return Key_H;
    case KEY_I:           return Key_I;
    case KEY_J:           return Key_J;
    case KEY_K:           return Key_K;
    case KEY_L:           return Key_L;
    case KEY_M:           return Key_M;
    case KEY_N:           return Key_N;
    case KEY_O:           return Key_O;
    case KEY_P:           return Key_P;
    case KEY_Q:           return Key_Q;
    case KEY_R:           return Key_R;
    case KEY_S:           return Key_S;
    case KEY_T:           return Key_T;
    case KEY_U:           return Key_U;
    case KEY_V:           return Key_V;
    case KEY_W:           return Key_W;
    case KEY_X:           return Key_X;
    case KEY_Y:           return Key_Y;
    case KEY_Z:           return Key_Z;

    // Function keys
    case KEY_F1:          return Key_F1;
    case KEY_F2:          return Key_F2;
    case KEY_F3:          return Key_F3;
    case KEY_F4:          return Key_F4;
    case KEY_F5:          return Key_F5;
    case KEY_F6:          return Key_F6;
    case KEY_F7:          return Key_F7;
    case KEY_F8:          return Key_F8;
    case KEY_F9:          return Key_F9;
    case KEY_F10:         return Key_F10;
    case KEY_F11:         return Key_F11;
    case KEY_F12:         return Key_F12;

    // Symbols
    case KEY_SEMICOLON:   return Key_Semicolon;
    case KEY_APOSTROPHE:  return Key_Apostrophe;
    case KEY_EQUAL:       return Key_Equal;
    case KEY_COMMA:       return Key_Comma;
    case KEY_MINUS:       return Key_Minus;
    case KEY_DOT:         return Key_Dot;
    case KEY_SLASH:       return Key_Slash;
    case KEY_GRAVE:       return Key_Grave;
    case KEY_LEFTBRACE:   return Key_LBracket;
    case KEY_RIGHTBRACE:  return Key_RBracket;
    case KEY_BACKSLASH:   return Key_Backslash;

    default: return Key_COUNT;
  }
}

internal u32 mouse_buttoncode_translate(u32 code) {
  switch (code) {
    case WL_LBUTTON: return MouseButton_Left;
    case WL_RBUTTON: return MouseButton_Right;
    
    default: return MouseButton_COUNT;
  }
}

#endif
