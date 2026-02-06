#include "lib.h"

#if OS_LINUX && !GFX_X11

#undef global
#include "wayland_extensions/xdg-shell-client-protocol.h"
#include "wayland_extensions/xdg-shell-protocol.c"
#include <wayland-client.h>
#define global static

#include <linux/input-event-codes.h>

struct WaylandState {
  wl_display* display;
  wl_registry* registry;
  wl_registry_listener  registry_list;
  wl_compositor* compositor;
  wl_surface* surface;
  wl_seat* seat;
  wl_seat_listener seat_listener;
  wl_pointer* pointer;
  wl_pointer_listener pointer_listener;
  wl_keyboard* keyboard;
  wl_keyboard_listener keyboard_listener;
  xdg_wm_base* xdg_wm_base;
  xdg_wm_base_listener xdg_wm_base_listener;
  xdg_surface* xdg_surface;
  xdg_surface_listener xdg_surface_listener;
  xdg_toplevel* xdg_toplevel;
  xdg_toplevel_listener xdg_toplevel_listener;
  
  i32 width = 1;
  i32 height = 1;
  b8 should_close = false;
  struct KeyboardState {
    b8 keys[256];
  };
  struct MouseState {
    f32 x;
    f32 y;
    b8 buttons[MouseButton_COUNT];
  };
  struct {
    KeyboardState keyboard_current;
    KeyboardState keyboard_previous;
    MouseState mouse_current;
    MouseState mouse_previous;
  } input;
};

global WaylandState wl_st;
#undef global // because of: wl_registry_listener.global;

intern u32 lnx_keycode_translate(u32 code) {
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

#define WL_LBUTTON 272
#define WL_RBUTTON 273
intern u32 lnx_mouse_buttoncode_translate(u32 code) {
  switch (code) {
    case WL_LBUTTON: return MouseButton_Left;
    case WL_RBUTTON: return MouseButton_Right;
    default: return MouseButton_COUNT;
  }
}

intern void wl_seat_capabilities(void* data, wl_seat* seat, u32 capabilities) {
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD && !wl_st.keyboard) {
    wl_st.keyboard = wl_seat_get_keyboard(seat);
    wl_st.keyboard_listener = {
      .keymap = [](void* data, wl_keyboard* keyboard, u32 format, i32 fd, u32 size){},
      .enter = [](void* data, wl_keyboard* keyboard, u32 serial, wl_surface* surface, wl_array* keys){},
      .leave = [](void* data, wl_keyboard* keyboard, u32 serial, wl_surface* surface){},
      .key = [](void* data, wl_keyboard* keyboard, u32 serial, u32 time, u32 key, u32 state) {
        u32 my_key = lnx_keycode_translate(key);
        wl_st.input.keyboard_current.keys[my_key] = state;
      },
      .modifiers = [](void* data, wl_keyboard* keyboard, u32 serial, u32 mods_depressed, u32 mods_latched, u32 mods_locked, u32 group){},
    };
    wl_keyboard_add_listener(wl_st.keyboard, &wl_st.keyboard_listener, data);
  }

  if (capabilities & WL_SEAT_CAPABILITY_POINTER && !wl_st.pointer) {
    wl_st.pointer = wl_seat_get_pointer(seat);
    wl_st.pointer_listener = {
      .enter = [](void* data, struct wl_pointer* wl_pointer, u32 serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y){},
      .leave = [](void* data, struct wl_pointer* wl_pointer, u32 serial, struct wl_surface* surface){},
      .motion = [](void* data, wl_pointer* pointer, u32 time, wl_fixed_t sx, wl_fixed_t sy) {
        f32 x = wl_fixed_to_double(sx);
        f32 y = wl_fixed_to_double(sy);
        wl_st.input.mouse_current.x = x;
        wl_st.input.mouse_current.y = y;
      },
      .button = [](void* data, struct wl_pointer* wl_pointer, u32 serial, u32 time, u32 button, u32 state) {
        #define WL_LBUTTON 272
        #define WL_RBUTTON 273
        u32 my_button = lnx_mouse_buttoncode_translate(button);
        wl_st.input.mouse_current.buttons[my_button] = state;
      },
    };
    wl_pointer_add_listener(wl_st.pointer, &wl_st.pointer_listener, data);
  }
}

intern void wl_registry_global_handler(void* data, wl_registry* registry, u32 name, const char* interface, u32 version) {
  if (str_match(interface, wl_compositor_interface.name)) {
    wl_st.compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, version);
  }
  else if (str_match(interface, wl_seat_interface.name)) {
    wl_st.seat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
    wl_st.seat_listener = {
      .capabilities = wl_seat_capabilities,
      .name = [](void* data, wl_seat* seat, const char* name) {},
    };
    wl_seat_add_listener(wl_st.seat, &wl_st.seat_listener, null);
  }
  else if (str_match(interface, xdg_wm_base_interface.name)) {
    wl_st.xdg_wm_base = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    wl_st.xdg_wm_base_listener = {
      .ping = [](void* data, xdg_wm_base* shell, u32 serial){ xdg_wm_base_pong(shell, serial); },
    };
    xdg_wm_base_add_listener(wl_st.xdg_wm_base, &wl_st.xdg_wm_base_listener, null);
  }
}

void os_gfx_init() {
  {
    wl_st.display = wl_display_connect(null);
    wl_st.registry = wl_display_get_registry(wl_st.display);
    wl_registry_listener wl_registry_listener = {.global = wl_registry_global_handler};
    wl_registry_add_listener(wl_st.registry, &wl_registry_listener, null);
    wl_display_roundtrip(wl_st.display);
    wl_st.surface = wl_compositor_create_surface(wl_st.compositor);
  }

  {
    wl_st.xdg_surface = xdg_wm_base_get_xdg_surface(wl_st.xdg_wm_base, wl_st.surface);
    wl_st.xdg_surface_listener = {
      .configure = [](void* data, xdg_surface* xdg_surface, u32 serial) {
        xdg_surface_ack_configure(xdg_surface, serial);
      }
    };
    xdg_surface_add_listener(wl_st.xdg_surface, &wl_st.xdg_surface_listener, null);
  }

  {
    wl_st.xdg_toplevel = xdg_surface_get_toplevel(wl_st.xdg_surface);
    wl_st.xdg_toplevel_listener = {
      .configure = [](void* data, xdg_toplevel* top, i32 width, i32 height, wl_array* stat) {
        if (!width || !height) {
          return;
        }
        wl_st.width = width;
        wl_st.height = height;
      },
      .close = [](void* data, xdg_toplevel* top){ wl_st.should_close = true; },
    };
    xdg_toplevel_add_listener(wl_st.xdg_toplevel, &wl_st.xdg_toplevel_listener, null);
  }

  xdg_toplevel_set_title(wl_st.xdg_toplevel, "wayland client");
  wl_surface_commit(wl_st.surface);
}

void os_gfx_shutdown() {
  wl_keyboard_destroy(wl_st.keyboard);
  wl_pointer_destroy(wl_st.pointer);
  wl_seat_release(wl_st.seat);
  xdg_toplevel_destroy(wl_st.xdg_toplevel);
  xdg_surface_destroy(wl_st.xdg_surface);
  wl_surface_destroy(wl_st.surface);
  wl_display_disconnect(wl_st.display);
}

void os_pump_messages() { 
  wl_display_dispatch_pending(wl_st.display);
}

b32 os_window_should_close() {
  return wl_st.should_close;
}

v2i os_get_window_size() {
  return v2i(wl_st.width, wl_st.height);
}

void os_get_gfx_api_handlers(void* out) {
  struct Surface {
    struct wl_display* wl_display;
    struct wl_surface* wl_surface;
  };
  *(Surface*)out = {.wl_display = wl_st.display, .wl_surface = wl_st.surface};
}
void  os_close_window() {
  wl_st.should_close = true; 
}

////////////////////////////////////////////////////////////////////////
// keyboard

void os_input_update() {
  MemCopyStruct(&wl_st.input.keyboard_previous, &wl_st.input.keyboard_current);
  MemCopyStruct(&wl_st.input.mouse_previous, &wl_st.input.mouse_current);
}

b32 os_is_key_down(Key key)       { return wl_st.input.keyboard_current.keys[key] == true; }
b32 os_is_key_up(Key key)         { return wl_st.input.keyboard_current.keys[key] == false; }
b32 os_was_key_down(Key key)      { return wl_st.input.keyboard_previous.keys[key] == true; }
b32 os_was_key_up(Key key)        { return wl_st.input.keyboard_previous.keys[key] == false; }
b32 os_is_key_pressed(Key key)    { return os_is_key_down(key) && os_was_key_up(key); }
b32 os_is_key_released(Key key)   { return os_is_key_up(key) && os_was_key_down(key); }

////////////////////////////////////////////////////////////////////////
// mouse

b32 os_is_button_down(MouseButtons button)          { return wl_st.input.mouse_current.buttons[button] == true; }
b32 os_is_button_up(MouseButtons button)            { return wl_st.input.mouse_current.buttons[button] == false; }
b32 os_was_button_down(MouseButtons button)         { return wl_st.input.mouse_previous.buttons[button] == true; }
b32 os_was_button_up(MouseButtons button)           { return wl_st.input.mouse_previous.buttons[button] == false; }
b32 os_is_button_pressed(MouseButtons button)       { return os_is_button_down(button) && os_was_button_up(button); }
b32 os_is_button_released(MouseButtons button)      { return os_is_button_up(button) && os_was_button_down(button); }

v2 os_get_mouse_pos() {
  return v2(wl_st.input.mouse_current.x, wl_st.input.mouse_current.y);
}

#endif
