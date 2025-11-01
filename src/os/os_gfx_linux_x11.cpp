#include "lib.h"

#if OS_LINUX && GFX_X11

// #include <X11/Xlib.h>
// #include <X11/Xutil.h>
// #include <X11/Xatom.h>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "event.h"

struct X11State {
  xcb_connection_t* connection;
  xcb_screen_t* screen;
  xcb_window_t window;
  xcb_key_symbols_t* key_symbols;

  // Input
  i32 width = 1;
  i32 height = 1;
  b8 should_close = false;

  struct KeyboardState {
    b8 keys[256];
  };
  struct MouseState {
    u16 x;
    u16 y;
    b8 buttons[MouseButton_COUNT];
  };
  struct {
    struct KeyboardState keyboard_current;
    struct KeyboardState keyboard_previous;
    struct MouseState mouse_current;
    struct MouseState mouse_previous;
  } input;

  struct {
    // Display* display;
    // Window window;
  } vk_surface;
};

global X11State st;

intern u32 x11_mouse_button_translate(u32 button) {
  // switch (button) {
  //   case Button1: return MouseButton_Left;
  //   case Button3: return MouseButton_Right;
  //   default: return MouseButton_COUNT;
  // }
  return 0;
}

// void os_gfx_init() {
//   st.display = XOpenDisplay(null);
//   Display* display = st.display;
//   AssertMsg(display, "failed to open display");

//   i32 screen = DefaultScreen(display);
//   Window root = RootWindow(display, screen);
//   Window window = XCreateSimpleWindow(
//     display, root,
//     100, 100,                // x, y position
//     800, 600,                // width, height
//     1,                       // border width
//     BlackPixel(display, screen),
//     WhitePixel(display, screen));
//   XStoreName(display, window, "X11 Window Example");
//   XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);
//   XMapWindow(display, window);
// }

// void os_gfx_shutdown() {
// }

// // Process pending X11 events (non-blocking)
// void os_pump_messages() {
//   Display* display = st.display;

//   XEvent event;
//   while (XPending(display)) {
//     XNextEvent(display, &event);

//     switch (event.type) {
//       case KeyPress: {
//         KeySym key_x11 = XLookupKeysym(&event.xkey, 0);
//         Key key = lnx_keycode_translate(key_x11);
//         st.input.keyboard_current.keys[key] = true;
//       } break;
//       case KeyRelease: {
//         KeySym key_x11 = XLookupKeysym(&event.xkey, 0);
//         Key key = lnx_keycode_translate(key_x11);
//         st.input.keyboard_current.keys[key] = false;
//       } break;
//     }
//   }

// }

Key lnx_keycode_translate(u32 keysym) {
  switch (keysym) {
    // Control keys
    case XK_BackSpace:    return Key_Backspace;
    case XK_Return:       return Key_Enter;
    case XK_Tab:          return Key_Tab;
    case XK_Shift_L:      return Key_LShift;
    case XK_Shift_R:      return Key_RShift;
    case XK_Control_L:    return Key_LControl;
    case XK_Control_R:    return Key_RControl;
    case XK_Alt_L:        return Key_LAlt;
    case XK_Alt_R:        return Key_RAlt;
    case XK_Escape:       return Key_Escape;
    case XK_Caps_Lock:    return Key_Capslock;

    // Navigation
    case XK_space:        return Key_Space;
    case XK_Page_Up:      return Key_Pageup;
    case XK_Page_Down:    return Key_Pagedown;
    case XK_End:          return Key_End;
    case XK_Home:         return Key_Home;
    case XK_Left:         return Key_Left;
    case XK_Up:           return Key_Up;
    case XK_Right:        return Key_Right;
    case XK_Down:         return Key_Down;

    // Function keys
    case XK_F1:           return Key_F1;
    case XK_F2:           return Key_F2;
    case XK_F3:           return Key_F3;
    case XK_F4:           return Key_F4;
    case XK_F5:           return Key_F5;
    case XK_F6:           return Key_F6;
    case XK_F7:           return Key_F7;
    case XK_F8:           return Key_F8;
    case XK_F9:           return Key_F9;
    case XK_F10:          return Key_F10;
    case XK_F11:          return Key_F11;
    case XK_F12:          return Key_F12;

    // Letters
    case XK_a: case XK_A: return Key_A;
    case XK_b: case XK_B: return Key_B;
    case XK_c: case XK_C: return Key_C;
    case XK_d: case XK_D: return Key_D;
    case XK_e: case XK_E: return Key_E;
    case XK_f: case XK_F: return Key_F;
    case XK_g: case XK_G: return Key_G;
    case XK_h: case XK_H: return Key_H;
    case XK_i: case XK_I: return Key_I;
    case XK_j: case XK_J: return Key_J;
    case XK_k: case XK_K: return Key_K;
    case XK_l: case XK_L: return Key_L;
    case XK_m: case XK_M: return Key_M;
    case XK_n: case XK_N: return Key_N;
    case XK_o: case XK_O: return Key_O;
    case XK_p: case XK_P: return Key_P;
    case XK_q: case XK_Q: return Key_Q;
    case XK_r: case XK_R: return Key_R;
    case XK_s: case XK_S: return Key_S;
    case XK_t: case XK_T: return Key_T;
    case XK_u: case XK_U: return Key_U;
    case XK_v: case XK_V: return Key_V;
    case XK_w: case XK_W: return Key_W;
    case XK_x: case XK_X: return Key_X;
    case XK_y: case XK_Y: return Key_Y;
    case XK_z: case XK_Z: return Key_Z;

    // Numbers
    case XK_0: return Key_0;
    case XK_1: return Key_1;
    case XK_2: return Key_2;
    case XK_3: return Key_3;
    case XK_4: return Key_4;
    case XK_5: return Key_5;
    case XK_6: return Key_6;
    case XK_7: return Key_7;
    case XK_8: return Key_8;
    case XK_9: return Key_9;

    // Symbols
    case XK_semicolon:    return Key_Semicolon;
    case XK_apostrophe:   return Key_Apostrophe;
    case XK_equal:        return Key_Equal;
    case XK_comma:        return Key_Comma;
    case XK_minus:        return Key_Minus;
    case XK_period:       return Key_Dot;
    case XK_slash:        return Key_Slash;
    case XK_grave:        return Key_Grave;
    case XK_bracketleft:  return Key_LBracket;
    case XK_bracketright: return Key_RBracket;
    case XK_backslash:    return Key_Backslash;

    default: return Key_COUNT;
  }
}

void os_gfx_init() {
  /* Connect to the X server */
  int screen_number;
  st.connection = xcb_connect(NULL, &screen_number);
  if (xcb_connection_has_error(st.connection)) {
    AssertMsg(false, "Failed to connect to X server");
  }

  /* Get the screen */
  const xcb_setup_t* setup = xcb_get_setup(st.connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  for (int i = 0; i < screen_number; ++i)
    xcb_screen_next(&iter);
  st.screen = iter.data;

  /* Create the window */
  st.window = xcb_generate_id(st.connection);
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[] = {
    st.screen->white_pixel,
    XCB_EVENT_MASK_EXPOSURE |
    XCB_EVENT_MASK_KEY_PRESS |
    XCB_EVENT_MASK_KEY_RELEASE |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY
  };

  xcb_create_window(
    st.connection,
    XCB_COPY_FROM_PARENT,         // depth
    st.window,                    // window ID
    st.screen->root,              // parent
    100, 100, 800, 600,           // x, y, width, height
    1,                            // border width
    XCB_WINDOW_CLASS_INPUT_OUTPUT,// class
    st.screen->root_visual,       // visual
    mask, values                  // masks
  );

  /* Set window title */
  const char* title = "XCB Window Example";
  xcb_change_property(
    st.connection,
    XCB_PROP_MODE_REPLACE,
    st.window,
    XCB_ATOM_WM_NAME,
    XCB_ATOM_STRING,
    8,
    cstr_length(title),
    title
  );

  /* Handle window close (WM_DELETE_WINDOW) */
  xcb_intern_atom_cookie_t protocols_cookie = xcb_intern_atom(st.connection, 1, 12, "WM_PROTOCOLS");
  xcb_intern_atom_cookie_t delete_cookie = xcb_intern_atom(st.connection, 0, 16, "WM_DELETE_WINDOW");
  xcb_intern_atom_reply_t* protocols_reply = xcb_intern_atom_reply(st.connection, protocols_cookie, NULL);
  xcb_intern_atom_reply_t* delete_reply = xcb_intern_atom_reply(st.connection, delete_cookie, NULL);

  xcb_change_property(
    st.connection,
    XCB_PROP_MODE_REPLACE,
    st.window,
    protocols_reply->atom,
    4,
    32,
    1,
    &delete_reply->atom
  );

  /* Map (show) the window */
  xcb_map_window(st.connection, st.window);
  xcb_flush(st.connection);
  st.key_symbols = xcb_key_symbols_alloc(st.connection);
}

void os_gfx_shutdown() {
  xcb_disconnect(st.connection);
}

void os_pump_messages() {
  xcb_generic_event_t* event;
  while ((event = xcb_poll_for_event(st.connection))) {
    switch (event->response_type & ~0x80) {
      case XCB_KEY_PRESS: {
        xcb_key_press_event_t* kp = (xcb_key_press_event_t*)event;
        // xcb_keysym_t sym = xcb_key_symbols_get_keysym(st.key_symbols, kp->detail, 0);
        xcb_keysym_t sym = xcb_key_press_lookup_keysym(st.key_symbols, kp, 0);
        Key key = lnx_keycode_translate(sym);
        st.input.keyboard_current.keys[key] = true;
        Info("Key pressed: %c", key);

      } break;
      case XCB_KEY_RELEASE: {
        xcb_key_press_event_t* kp = (xcb_key_press_event_t*)event;
        // xcb_keysym_t sym = xcb_key_symbols_get_keysym(st.key_symbols, kp->detail, 0);
        xcb_keysym_t sym = xcb_key_press_lookup_keysym(st.key_symbols, kp, 0);
        Key key = lnx_keycode_translate(sym);
        st.input.keyboard_current.keys[key] = false;
        Info("Key released: %c", key);
      } break;
    }
  }
}


b32 os_window_should_close() { return st.should_close; }
v2i os_get_window_size() { return v2i(st.width, st.height); }
v2i os_get_mouse_pos() { return v2i(st.input.mouse_current.x, st.input.mouse_current.y); }

void* os_get_vk_surface() {
  return {};
}

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

#endif
