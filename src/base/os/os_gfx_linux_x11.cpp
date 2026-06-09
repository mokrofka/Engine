#include "lib.h"

#if OS_LINUX && GFX_X11

#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <xcb/xcb_keysyms.h> // to handle keys
#include <xcb/xinput.h>      // to handle input
#define explicit xxxxx
#include <xcb/xkb.h>         // to handle autorepeat press event
#undef explicit

struct Clipboard {
  xcb_atom_t atom;
  xcb_atom_t targets_atom;
  xcb_atom_t property_atom;
  xcb_atom_t utf8_atom;
  Dstring str_to_write;
  Dstring str_to_read;
};

struct X11State {
  Arena arena;
  AllocSegList gpa;
  xcb_connection_t* connection;
  xcb_screen_t* screen;
  xcb_window_t window;
  xcb_key_symbols_t* key_symbols;
  xcb_atom_t wm_delete_window;
  Clipboard clipboard;
  u32 win_width = 1;
  u32 win_height = 1;
  u32 screen_width;
  u32 screen_height;
  b32 should_close;
  b32 cursor_is_hidden;
  v2 moused_saved_pos;
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
    f32 wheel;
    f32 wheel_horizontal;
    i32 mouse_x_delta;
    i32 mouse_y_delta;
  } input;
  Darray<OS_InputEvent> input_events;
  Darray<xcb_generic_event_t*> xcb_events;
  OS_Modifiers modifiers;
};

global X11State gfx_st;

Key lnx_x11_keycode_translate(u32 keysym) {
  switch (keysym) {
    // Control keys
    case XK_BackSpace:    return Key_Backspace;
    case XK_Return:       return Key_Enter;
    case XK_Tab:          return Key_Tab;
    case XK_Delete:       return Key_Delete;
    case XK_Shift_L:      return Key_LShift;
    case XK_Shift_R:      return Key_RShift;
    case XK_Control_L:    return Key_LControl;
    case XK_Control_R:    return Key_RControl;
    case XK_Alt_L:        return Key_LAlt;
    case XK_Alt_R:        return Key_RAlt;
    case XK_Escape:       return Key_Escape;
    case XK_Caps_Lock:    return Key_Capslock;
    case XK_Super_L:      return Key_Super;
    case XK_Super_R:      return Key_Super;

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

u32 os_key_to_character(Key key, OS_Modifiers modifiers) {
  if (!FlagHas(modifiers, OS_Modifier_Shift)) {
    switch (key) {
      default: return 0;
      case Key_Space: return ' ';
      case Key_0: return '0';
      case Key_1: return '1';
      case Key_2: return '2';
      case Key_3: return '3';
      case Key_4: return '4';  
      case Key_5: return '5'; 
      case Key_6: return '6'; 
      case Key_7: return '7'; 
      case Key_8: return '8'; 
      case Key_9: return '9'; 
      case Key_A: return 'a'; 
      case Key_B: return 'b'; 
      case Key_C: return 'c'; 
      case Key_D: return 'd'; 
      case Key_E: return 'e'; 
      case Key_F: return 'f'; 
      case Key_G: return 'g'; 
      case Key_H: return 'h'; 
      case Key_I: return 'i'; 
      case Key_J: return 'j'; 
      case Key_K: return 'k'; 
      case Key_L: return 'l'; 
      case Key_M: return 'm'; 
      case Key_N: return 'n'; 
      case Key_O: return 'o'; 
      case Key_P: return 'p'; 
      case Key_Q: return 'q'; 
      case Key_R: return 'r'; 
      case Key_S: return 's'; 
      case Key_T: return 't'; 
      case Key_U: return 'u'; 
      case Key_V: return 'v'; 
      case Key_W: return 'w'; 
      case Key_X: return 'x'; 
      case Key_Y: return 'y'; 
      case Key_Z: return 'z'; 
      case Key_Semicolon: return ';';
      case Key_Apostrophe: return '\'';
      case Key_Comma: return ',';
      case Key_Dot: return '.';
      case Key_Equal: return '=';
      case Key_Minus: return '-';
      case Key_Grave: return '`';
      case Key_LBracket: return '[';
      case Key_RBracket: return ']';
      case Key_Slash: return '/';
      case Key_Backslash: return '\\';
    }
  }
  if (FlagHas(modifiers, OS_Modifier_Shift)) {
    switch (key) {
      default: return 0;
      case Key_Space: return ' ';
      case Key_0: return ')';
      case Key_1: return '!';
      case Key_2: return '@';
      case Key_3: return '#';
      case Key_4: return '$';  
      case Key_5: return '%'; 
      case Key_6: return '^'; 
      case Key_7: return '&'; 
      case Key_8: return '*'; 
      case Key_9: return '('; 
      case Key_A: return 'A'; 
      case Key_B: return 'B'; 
      case Key_C: return 'C'; 
      case Key_D: return 'D'; 
      case Key_E: return 'E'; 
      case Key_F: return 'F'; 
      case Key_G: return 'G'; 
      case Key_H: return 'H'; 
      case Key_I: return 'I'; 
      case Key_J: return 'J'; 
      case Key_K: return 'K'; 
      case Key_L: return 'L'; 
      case Key_M: return 'M'; 
      case Key_N: return 'N'; 
      case Key_O: return 'O'; 
      case Key_P: return 'P'; 
      case Key_Q: return 'Q'; 
      case Key_R: return 'R'; 
      case Key_S: return 'S'; 
      case Key_T: return 'T'; 
      case Key_U: return 'U'; 
      case Key_V: return 'V'; 
      case Key_W: return 'W'; 
      case Key_X: return 'X'; 
      case Key_Y: return 'Y'; 
      case Key_Z: return 'Z'; 
      case Key_Semicolon: return ':';
      case Key_Apostrophe: return '\"';
      case Key_Comma: return '<';
      case Key_Dot: return '>';
      case Key_Equal: return '+';
      case Key_Minus: return '_';
      case Key_Grave: return '~';
      case Key_LBracket: return '{';
      case Key_RBracket: return '}';
      case Key_Slash: return '?';
      case Key_Backslash: return '|';
    }
  }
  return 0;
}

void os_gfx_init() {
  X11State& g = gfx_st;
  g.arena = arena_make_named("gfx arena");
  g.gpa = alloc_seglist_make(g.arena);
  g.input_events = darray_make<OS_InputEvent>(g.gpa);
  g.xcb_events = darray_make<xcb_generic_event_t*>(g.gpa);

  i32 screen_number;
  g.connection = xcb_connect(null, &screen_number);
  xcb_connection_has_error(g.connection);
  const xcb_setup_t* setup = xcb_get_setup(g.connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  g.screen = iter.data;
  g.window = xcb_generate_id(g.connection);
  g.screen_width = iter.data->width_in_pixels;
  g.screen_height = iter.data->height_in_pixels;

  ///////////////////////////////////
  // Window creation
  u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  u32 values[] = {
    g.screen->black_pixel,
    XCB_EVENT_MASK_EXPOSURE |
    XCB_EVENT_MASK_KEY_PRESS |
    XCB_EVENT_MASK_KEY_RELEASE |
    XCB_EVENT_MASK_BUTTON_PRESS |
    XCB_EVENT_MASK_BUTTON_RELEASE |
    XCB_EVENT_MASK_POINTER_MOTION |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY
  };
  xcb_create_window(g.connection, XCB_COPY_FROM_PARENT, g.window, g.screen->root,
    100, 100, 800, 600,
    1, XCB_WINDOW_CLASS_INPUT_OUTPUT, g.screen->root_visual, mask, values);
  const char* title = "XCB Window Example";
  xcb_change_property(g.connection, XCB_PROP_MODE_REPLACE, g.window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, cstr_length(title), title);
  xcb_intern_atom_cookie_t protocols_cookie = xcb_intern_atom(g.connection, 1, 12, "WM_PROTOCOLS");
  xcb_intern_atom_reply_t* protocols_reply = xcb_intern_atom_reply(g.connection, protocols_cookie, null);
  xcb_intern_atom_cookie_t delete_cookie = xcb_intern_atom(g.connection, 0, 16, "WM_DELETE_WINDOW");
  xcb_intern_atom_reply_t* delete_reply = xcb_intern_atom_reply(g.connection, delete_cookie, null);
  g.wm_delete_window = delete_reply->atom;
  xcb_change_property(g.connection, XCB_PROP_MODE_REPLACE, g.window, protocols_reply->atom, 4, 32, 1, &delete_reply->atom);
  xcb_map_window(g.connection, g.window);

  ///////////////////////////////////
  // Clipboard
  var intern_ = [](String name)->xcb_atom_t {
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(gfx_st.connection, 0, name.size, (const char*)name.str);
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(gfx_st.connection, cookie, null);
    if (!reply) return XCB_NONE;
    xcb_atom_t atom = reply->atom;
    return atom;
  };
  g.clipboard.atom = intern_("CLIPBOARD");
  g.clipboard.targets_atom = intern_("TARGETS");
  g.clipboard.utf8_atom = intern_("UTF8_STRING");
  g.clipboard.property_atom = intern_("XSEL_DATA");
  g.clipboard.str_to_write = dstr_make(g.arena);
  g.clipboard.str_to_read = dstr_make(g.arena);

  ///////////////////////////////////
  // Keysym extension
  g.key_symbols = xcb_key_symbols_alloc(g.connection);

  ///////////////////////////////////
  // detect autorepeat extension
  xcb_xkb_use_extension(g.connection, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
  xcb_xkb_per_client_flags(g.connection,
    XCB_XKB_ID_USE_CORE_KBD,
    XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
    XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
    0, 0, 0);

  ///////////////////////////////////
  // Raw mouse input
  struct {
    xcb_input_event_mask_t head;
    u32 mask;
  } evmask = {
    .head = {
      .deviceid = XCB_INPUT_DEVICE_ALL_MASTER,
      .mask_len = 1,
    },
    .mask = XCB_INPUT_XI_EVENT_MASK_RAW_MOTION,
  };
  xcb_input_xi_select_events(g.connection, g.screen->root, 1, &evmask.head);

  xcb_flush(g.connection);
}

void os_gfx_shutdown() { xcb_disconnect(gfx_st.connection); }

////////////////////////////////////////////////////////////////////////
// Window related

void os_pump_messages() {
  X11State& g = gfx_st;
  os_input_update();
  array_clear(g.input_events);
  g.input.wheel = 0;
  g.input.wheel_horizontal = 0;
  g.input.mouse_x_delta = 0;
  g.input.mouse_y_delta = 0;

  u32 i = 0;
  while (true) {
    xcb_generic_event_t* event = xcb_poll_for_event(g.connection);
    if (event) {
    } else if (i < g.xcb_events.count) {
      event = g.xcb_events[i++];
    } else {
      break;
    }
    if ((event->response_type & 0x7f) == XCB_GE_GENERIC) {
      xcb_ge_generic_event_t* ge = (xcb_ge_generic_event_t*)event;
      if (ge->event_type == XCB_INPUT_RAW_MOTION) {
        xcb_input_raw_motion_event_t* motion = (xcb_input_raw_motion_event_t*)ge;
        u32* mask = (u32*)((u8*)motion + sizeof(*motion));
        i32* values = (i32*)(mask + motion->valuators_len);
        i32 dx = values[0];
        i32 dy = values[2];
        g.input.mouse_x_delta = dx;
        g.input.mouse_y_delta = dy;
        // Info("%i %i", dx, dy);
      }
    }
    switch (event->response_type & ~0x80) {
      case XCB_KEY_PRESS: {
        xcb_key_press_event_t* kp = (xcb_key_press_event_t*)event;
        xcb_keysym_t sym = xcb_key_symbols_get_keysym(g.key_symbols, kp->detail, 0);
        Key key = lnx_x11_keycode_translate(sym);
        if (os_key_is_down(key)) {
          break;
        }
        g.input.keyboard_current.keys[key] = true;
        b32 modifier_changed = false;
        // Info("pressed %u", kp->time);
        if (key == Key_Shift) {
          if (!FlagHas(g.modifiers, OS_Modifier_Shift)) {
            modifier_changed = true;
          }
          g.modifiers |= OS_Modifier_Shift;
        }
        if (key == Key_Alt) {
          if (!FlagHas(g.modifiers, OS_Modifier_Alt)) {
            modifier_changed = true;
          }
          g.modifiers |= OS_Modifier_Alt;
        }
        if (key == Key_Ctrl) {
          if (!FlagHas(g.modifiers, OS_Modifier_Ctrl)) {
            modifier_changed = true;
          }
          g.modifiers |= OS_Modifier_Ctrl;
        }
        if (modifier_changed) {
          OS_InputEvent event = {
            .type = OS_EventType_Modifier,
            .modifier = g.modifiers,
          };
          array_push(g.input_events, event);
        }
        OS_InputEvent event = {
          .type = OS_EventType_Key,
          .key = key,
          .is_pressed = true,
          .modifier = g.modifiers,
        };
        array_push(g.input_events, event);
      } break;
      case XCB_KEY_RELEASE: {
        xcb_key_release_event_t* kp = (xcb_key_release_event_t*)event;
        xcb_keysym_t sym = xcb_key_symbols_get_keysym(g.key_symbols, kp->detail, 0);
        Key key = lnx_x11_keycode_translate(sym);
        g.input.keyboard_current.keys[key] = false;
        b32 modifier_changed = false;
        // Info("released %u", kp->time);
        if (key == Key_Shift) {
          if (FlagHas(g.modifiers, OS_Modifier_Shift)) {
            modifier_changed = true;
          }
          g.modifiers = FlagClear(g.modifiers, OS_Modifier_Shift);
        }
        if (key == Key_Alt) {
          if (FlagHas(g.modifiers, OS_Modifier_Alt)) {
            modifier_changed = true;
          }
          g.modifiers = FlagClear(g.modifiers, OS_Modifier_Alt);
        }
        if (key == Key_Ctrl) {
          if (FlagHas(g.modifiers, OS_Modifier_Ctrl)) {
            modifier_changed = true;
          }
          g.modifiers = FlagClear(g.modifiers, OS_Modifier_Ctrl);
        }
        if (modifier_changed) {
          OS_InputEvent event = {
            .type = OS_EventType_Modifier,
            .modifier = g.modifiers,
          };
          array_push(g.input_events, event);
        }
        OS_InputEvent event = {
          .type = OS_EventType_Key,
          .key = key,
          .is_pressed = false,
          .modifier = g.modifiers,
        };
        array_push(g.input_events, event);
      } break;
      case XCB_CONFIGURE_NOTIFY: {
        xcb_configure_notify_event_t* cfg = (xcb_configure_notify_event_t*)event;
        if (!cfg->width || !cfg->height) {
          return;
        }
        g.win_width = cfg->width;
        g.win_height = cfg->height;
      } break;
      case XCB_CLIENT_MESSAGE: {
        xcb_client_message_event_t* cm = (xcb_client_message_event_t*)event;
        if (cm->data.data32[0] == g.wm_delete_window) {
          g.should_close = true;
        }
      } break;
      case XCB_BUTTON_PRESS: {
        #define XK_MouseLeft 1
        #define XK_MouseMiddle 2
        #define XK_MouseRight 3
        #define XK_WheelUp 4
        #define XK_WheelDown 5
        #define XK_WheelRight 6
        #define XK_WheelLeft 7
        xcb_button_press_event_t* bp = (xcb_button_press_event_t*)event;
        OS_InputEvent event = {};

        ///////////////////////////////////
        // Button
        b32 was_button = false;
        switch (bp->detail) {
          case XK_MouseLeft: g.input.mouse_current.buttons[MouseButton_Left] = true; event.mouse_button = MouseButton_Left; was_button = true; break;
          case XK_MouseMiddle: g.input.mouse_current.buttons[MouseButton_Middle] = true; event.mouse_button = MouseButton_Middle; was_button = true; break;
          case XK_MouseRight: g.input.mouse_current.buttons[MouseButton_Right] = true; event.mouse_button = MouseButton_Right; was_button = true; break;
        }
        event.type = OS_EventType_MouseButton;
        event.is_pressed = true;

        ///////////////////////////////////
        // Scroll
        if (!was_button) {
          switch (bp->detail) {
            case XK_WheelUp: ; event.scroll = 1; break;
            case XK_WheelDown: event.scroll = -1; break;
            case XK_WheelRight: g.input.wheel_horizontal = 1; break;
            case XK_WheelLeft: g.input.wheel_horizontal = -1; break;
          }
          g.input.wheel = event.scroll;
          event.type = OS_EventType_Scroll;
        }

        array_push(g.input_events, event);
      } break;
      case XCB_BUTTON_RELEASE: {
        xcb_button_press_event_t* bp = (xcb_button_press_event_t*)event;
        OS_InputEvent event = {};
        if (bp->detail >= XK_MouseLeft && bp->detail <= XK_MouseRight) {
          switch (bp->detail) {
            case XK_MouseLeft: g.input.mouse_current.buttons[MouseButton_Left] = false; event.mouse_button = MouseButton_Left; break;
            case XK_MouseMiddle: g.input.mouse_current.buttons[MouseButton_Middle] = false; event.mouse_button = MouseButton_Middle; break;
            case XK_MouseRight: g.input.mouse_current.buttons[MouseButton_Right] = false; event.mouse_button = MouseButton_Right; break;
          }
          event.type = OS_EventType_MouseButton;
          event.is_pressed = false;
        }
        array_push(g.input_events, event);
      } break;
      case XCB_MOTION_NOTIFY: {
        xcb_motion_notify_event_t* motion = (xcb_motion_notify_event_t*)event;
        g.input.mouse_current.x = motion->event_x;
        g.input.mouse_current.y = motion->event_y;
        OS_InputEvent event = {.type = OS_EventType_MouseMove};
        event.x = g.input.mouse_current.x;
        event.y = g.input.mouse_current.y;
        array_push(g.input_events, event);
      } break;
      case XCB_SELECTION_REQUEST: {
        xcb_selection_request_event_t* req = (xcb_selection_request_event_t*)event;
        xcb_selection_notify_event_t notify = {
          .response_type = XCB_SELECTION_NOTIFY,
          .sequence = 0,
          .time = req->time,
          .requestor = req->requestor,
          .selection = req->selection,
          .target = req->target,
          .property = req->property,
        };
        if (req->target == g.clipboard.targets_atom) {
          xcb_atom_t supported[] = {
            g.clipboard.utf8_atom,
            g.clipboard.targets_atom
          };
          xcb_change_property(g.connection, XCB_PROP_MODE_REPLACE, req->requestor, req->property, XCB_ATOM_ATOM, 32, ArrayCount(supported), supported);
          notify.property = req->property;
        }
        if (req->target == g.clipboard.utf8_atom) {
          xcb_change_property(g.connection, XCB_PROP_MODE_REPLACE, req->requestor, req->property, req->target, 8, g.clipboard.str_to_write.size, g.clipboard.str_to_write.str);
        }
        xcb_send_event(g.connection, 0, req->requestor, XCB_EVENT_MASK_NO_EVENT, (char*)&notify);
        xcb_flush(g.connection);
      } break;
    }
  }
  array_clear(g.xcb_events);
}

b32 os_window_should_close() { return gfx_st.should_close; }
void os_close_window()       { gfx_st.should_close = true; }
v2u os_get_window_size()     { return v2u(gfx_st.win_width, gfx_st.win_height); }
v2u os_get_screen_size()     { return v2u(gfx_st.screen_width, gfx_st.screen_height); }
void os_get_gfx_handlers(void* out) {
  struct Surface {
    xcb_connection_t* connection;
    xcb_window_t window;
  };
  *(Surface*)out = { gfx_st.connection, gfx_st.window };
}

void os_clipboard_text_set(String str) {
  X11State& g = gfx_st;
  dstr_clear(g.clipboard.str_to_write);
  dstr_push(g.clipboard.str_to_write, str);
  xcb_set_selection_owner(g.connection, g.window, g.clipboard.atom, XCB_CURRENT_TIME);
  xcb_flush(g.connection);
}

String os_clipboard_text_get() {
  X11State& g = gfx_st;
  xcb_convert_selection(g.connection, g.window, g.clipboard.atom, g.clipboard.utf8_atom, g.clipboard.property_atom, XCB_CURRENT_TIME);
  xcb_flush(g.connection);

  xcb_generic_event_t* event;
  while (true) {
    event = xcb_wait_for_event(g.connection);
    u8 type = event->response_type & ~0x80;
    if (type == XCB_SELECTION_REQUEST) {
      xcb_window_t owner = xcb_get_selection_owner_reply(g.connection, xcb_get_selection_owner(g.connection, g.clipboard.atom), null)->owner;
      if (g.window != owner) {
        goto add_event;
      }
      xcb_selection_request_event_t* req = (xcb_selection_request_event_t*)event;
      xcb_selection_notify_event_t notify = {
        .response_type = XCB_SELECTION_NOTIFY,
        .sequence = 0,
        .time = req->time,
        .requestor = req->requestor,
        .selection = req->selection,
        .target = req->target,
        .property = req->property,
      };
      if (req->target == g.clipboard.targets_atom) {
        xcb_atom_t supported[] = {
          g.clipboard.utf8_atom,
          g.clipboard.targets_atom
        };
        xcb_change_property(g.connection, XCB_PROP_MODE_REPLACE, req->requestor, req->property, XCB_ATOM_ATOM, 32, ArrayCount(supported), supported);
        notify.property = req->property;
      }
      if (req->target == g.clipboard.utf8_atom) {
        xcb_change_property(g.connection, XCB_PROP_MODE_REPLACE, req->requestor, req->property, req->target, 8, g.clipboard.str_to_write.size, g.clipboard.str_to_write.str);
      }
      xcb_send_event(g.connection, 0, req->requestor, XCB_EVENT_MASK_NO_EVENT, (char*)&notify);
      xcb_flush(g.connection);
    }
    else if (type == XCB_SELECTION_NOTIFY) {
      xcb_selection_notify_event_t* notify = (xcb_selection_notify_event_t*)event;
      if (notify->property == XCB_NONE) {
        break;
      }
      xcb_get_property_cookie_t cookie = xcb_get_property(g.connection, 0, g.window, notify->property, XCB_GET_PROPERTY_TYPE_ANY, 0, 4096);
      xcb_get_property_reply_t* reply = xcb_get_property_reply(g.connection, cookie, null);
      if (reply) {
        u8* data = (u8*)xcb_get_property_value(reply);
        u32 len = xcb_get_property_value_length(reply);
        dstr_clear(g.clipboard.str_to_read);
        dstr_push(g.clipboard.str_to_read, str_make(data, len));
      }
      break;
    }
    else {
      add_event:
      array_push(g.xcb_events, event);
    }
  }
  return g.clipboard.str_to_read;
}

////////////////////////////////////////////////////////////////////////
// Cursor

void os_cursor_show() {
  X11State& g = gfx_st;
  uint32_t values[] = { XCB_NONE };
  xcb_change_window_attributes(
    g.connection,
    g.window,
    XCB_CW_CURSOR,
    values
  );
  xcb_flush(g.connection);
  g.cursor_is_hidden = false;
}
void os_cursor_hide() {
  X11State& g = gfx_st;
  xcb_pixmap_t pixmap = xcb_generate_id(g.connection);
  xcb_create_pixmap( g.connection, 1, pixmap, g.window, 1, 1);
  xcb_cursor_t cursor = xcb_generate_id(g.connection);
  xcb_create_cursor(g.connection, cursor, pixmap, pixmap, 0, 0, 0, 0, 0, 0, 0, 0);
  uint32_t values[] = {cursor};
  xcb_change_window_attributes(g.connection, g.window, XCB_CW_CURSOR, values);
  xcb_flush(g.connection);
  g.cursor_is_hidden = true;
}
b32 os_cursor_is_hiden() { return gfx_st.cursor_is_hidden; }
void os_cursor_confine_window() {
  X11State& g = gfx_st;
  xcb_grab_pointer(
    g.connection,
    1,
    g.window,
    XCB_EVENT_MASK_POINTER_MOTION |
    XCB_EVENT_MASK_BUTTON_PRESS |
    XCB_EVENT_MASK_BUTTON_RELEASE,
    XCB_GRAB_MODE_ASYNC,
    XCB_GRAB_MODE_ASYNC,
    g.window,
    XCB_NONE,
    XCB_CURRENT_TIME
  );
}
void os_cursor_release_window() {
  X11State& g = gfx_st;
  xcb_ungrab_pointer(g.connection, XCB_CURRENT_TIME);
  xcb_flush(g.connection);
}
void os_cursor_lock() {
  X11State& g = gfx_st;
  g.moused_saved_pos = v2(g.input.mouse_current.x, g.input.mouse_current.y);
  os_cursor_hide();
  os_mouse_set_pos(v2i(g.win_width/2, g.win_height/2));
  os_cursor_confine_window();
}
void os_cursor_unlock() {
  X11State& g = gfx_st;
  os_mouse_set_pos(v2i_of_v2(g.moused_saved_pos));
  os_cursor_show();
  os_cursor_release_window();
}

////////////////////////////////////////////////////////////////////////
// Input

void os_input_update() {
  MemCopyStruct(&gfx_st.input.keyboard_previous, &gfx_st.input.keyboard_current);
  MemCopyStruct(&gfx_st.input.mouse_previous, &gfx_st.input.mouse_current);
}
Slice<OS_InputEvent> os_get_input_events()          { return {gfx_st.input_events.data, gfx_st.input_events.count}; }

b32 os_key_is_down(Key key)                         { return gfx_st.input.keyboard_current.keys[key] == true; }
b32 os_key_is_up(Key key)                           { return gfx_st.input.keyboard_current.keys[key] == false; }
b32 os_key_was_down(Key key)                        { return gfx_st.input.keyboard_previous.keys[key] == true; }
b32 os_key_was_up(Key key)                          { return gfx_st.input.keyboard_previous.keys[key] == false; }
b32 os_key_is_pressed(Key key)                      { return os_key_is_down(key) && os_key_was_up(key); }
b32 os_key_is_released(Key key)                     { return os_key_is_up(key) && os_key_was_down(key); }

b32 os_mouse_is_button_down(MouseButton button)     { return gfx_st.input.mouse_current.buttons[button] == true; }
b32 os_mouse_is_button_up(MouseButton button)       { return gfx_st.input.mouse_current.buttons[button] == false; }
b32 os_mouse_was_button_down(MouseButton button)    { return gfx_st.input.mouse_previous.buttons[button] == true; }
b32 os_mouse_was_button_up(MouseButton button)      { return gfx_st.input.mouse_previous.buttons[button] == false; }
b32 os_mouse_is_button_pressed(MouseButton button)  { return os_mouse_is_button_down(button) && os_mouse_was_button_up(button); }
b32 os_mouse_is_button_released(MouseButton button) { return os_mouse_is_button_up(button) && os_mouse_was_button_down(button); }
v2  os_mouse_get_pos()                              { return v2(gfx_st.input.mouse_current.x, gfx_st.input.mouse_current.y); }
f32 os_mouse_get_wheel()                            { return gfx_st.input.wheel; }
f32 os_mouse_get_wheel_horizontal()                 { return gfx_st.input.wheel_horizontal; }
v2  os_mouse_get_delta()                            { return v2(gfx_st.input.mouse_x_delta, gfx_st.input.mouse_y_delta); }
void os_mouse_set_pos(v2i pos) {
  X11State& g = gfx_st;
  xcb_warp_pointer(g.connection, XCB_NONE, g.window, 0, 0, 0, 0, pos.x, pos.y);
  xcb_flush(g.connection);
}

const char* imgui_platform_get_clipboard_text(struct ImGuiContext* ctx) {
  Scratch scratch;
  String str = push_str_copy(scratch, os_clipboard_text_get());
  return (const char*)str.str;
}

void imgui_platform_set_clipboard_text(struct ImGuiContext* ctx, const char* text) {
  os_clipboard_text_set(text);
}

#endif
