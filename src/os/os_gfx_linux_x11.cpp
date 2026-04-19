#include "lib.h"

#if OS_LINUX && GFX_X11

#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include <stdlib.h>

struct Clipboard {
  xcb_atom_t atom;
  xcb_atom_t targets_atom;
  xcb_atom_t property_atom;
  xcb_atom_t utf8_atom;
  DString str_to_write;
  DString str_to_read;
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
  u32 width = 1;
  u32 height = 1;
  b8 should_close;
  struct KeyboardState {
    b8 keys[256];
  };
  struct MouseState {
    f32 x;
    f32 y;
  };
  struct {
    KeyboardState keyboard_current;
    KeyboardState keyboard_previous;
    MouseState mouse_current;
    MouseState mouse_previous;
    f32 mouse_scroll;
    f32 touchpad_move_x;
  } input;
  Darray<OS_InputEvent> input_events;
  Darray<xcb_generic_event_t*> xcb_events;
  OS_Modifiers modifiers = 0;
};

global X11State gfx_st;

Key lnx_keycode_translate(u32 keysym) {
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

u32 os_key_to_str(Key key, OS_Modifiers modifiers) {
  if (!FlagHas(modifiers, OS_Modifier_Shift)) {
    switch (key) {
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
      default: return 0;
    }
  }
  if (FlagHas(modifiers, OS_Modifier_Shift)) {
    switch (key) {
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
      default: return 0;
    }
  }
  return 0;
}

xcb_atom_t intern_(String name) {
  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(gfx_st.connection, 0, name.size, (const char*)name.str);
  xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(gfx_st.connection, cookie, null);
  if (!reply) return XCB_NONE;
  xcb_atom_t atom = reply->atom;
  return atom;
}

void os_gfx_init() {
  X11State& g = gfx_st;
  g.arena = arena_init();
  g.gpa.init(g.arena);
  g.input_events.init(g.gpa);

  i32 screen_number;
  g.connection = xcb_connect(null, &screen_number);
  xcb_connection_has_error(g.connection);
  const xcb_setup_t* setup = xcb_get_setup(g.connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  Loop (i, screen_number) {
    xcb_screen_next(&iter);
  }
  g.screen = iter.data;
  g.window = xcb_generate_id(g.connection);
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
  g.clipboard.atom = intern_("CLIPBOARD");
  g.clipboard.targets_atom = intern_("TARGETS");
  g.clipboard.utf8_atom = intern_("UTF8_STRING");
  g.clipboard.property_atom = intern_("XSEL_DATA");

  g.wm_delete_window = delete_reply->atom;
  xcb_change_property(g.connection, XCB_PROP_MODE_REPLACE, g.window, protocols_reply->atom, 4, 32, 1, &delete_reply->atom);
  xcb_map_window(g.connection, g.window);
  xcb_flush(g.connection);
  g.key_symbols = xcb_key_symbols_alloc(g.connection);
}

void os_gfx_shutdown() { xcb_disconnect(gfx_st.connection); }

void os_pump_messages() {
  X11State& g = gfx_st;
  g.input_events.clear();
  g.input.mouse_scroll = 0;
  g.input.touchpad_move_x = 0;

  xcb_generic_event_t* event;
  u32 i = 0;
  while (true) {
    event = xcb_poll_for_event(g.connection);
    if (event) {
    } else if (i < g.xcb_events.count) {
      event = g.xcb_events[i++];
    } else {
      break;
    }
    switch (event->response_type & ~0x80) {
      case XCB_KEY_PRESS: {
        xcb_key_press_event_t* kp = (xcb_key_press_event_t*)event;
        xcb_keysym_t sym = xcb_key_symbols_get_keysym(g.key_symbols, kp->detail, 0);
        Key key = lnx_keycode_translate(sym);
        g.input.keyboard_current.keys[key] = true;
        b32 modifier_changed = false;
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
            .type = OS_EventKind_Modifier,
            .modifier = g.modifiers,
          };
          g.input_events.add(event);
        }
        OS_InputEvent event = {
          .type = OS_EventKind_Key,
          .is_pressed = true,
          .key = key,
          .modifier = g.modifiers,
        };
        g.input_events.add(event);
      } break;
      case XCB_KEY_RELEASE: {
        xcb_key_press_event_t* kp = (xcb_key_press_event_t*)event;
        xcb_keysym_t sym = xcb_key_symbols_get_keysym(g.key_symbols, kp->detail, 0);
        Key key = lnx_keycode_translate(sym);
        g.input.keyboard_current.keys[key] = false;
        b32 modifier_changed = false;
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
            .type = OS_EventKind_Modifier,
            .modifier = g.modifiers,
          };
          g.input_events.add(event);
        }
        OS_InputEvent event = {
          .type = OS_EventKind_Key,
          .key = key,
          .is_pressed = false,
          .modifier = g.modifiers,
        };
        g.input_events.add(event);
      } break;
      case XCB_CONFIGURE_NOTIFY: {
        xcb_configure_notify_event_t* cfg = (xcb_configure_notify_event_t*)event;
        if (!cfg->width || !cfg->height) {
          return;
        }
        g.width = cfg->width;
        g.height = cfg->height;
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
        #define XK_ScrollUp 4
        #define XK_ScrollDown 5
        #define XK_ScrollLeft 6
        #define XK_ScrollRight 7
        xcb_button_press_event_t* bp = (xcb_button_press_event_t*)event;
        OS_InputEvent event = {};
        if (bp->detail >= XK_MouseLeft && bp->detail <= XK_MouseRight) {
          switch (bp->detail) {
            case XK_MouseLeft: g.input.keyboard_current.keys[MouseKey_Left] = true; event.key = MouseKey_Left; break;
            case XK_MouseMiddle: g.input.keyboard_current.keys[MouseKey_Middle] = true; event.key = MouseKey_Middle; break;
            case XK_MouseRight: g.input.keyboard_current.keys[MouseKey_Right] = true; event.key = MouseKey_Right; break;
          }
          event.type = OS_EventKind_MouseButton;
          event.is_pressed = true;
        }
        else {
          switch (bp->detail) {
            case XK_ScrollUp: ; event.scroll = 1; break;
            case XK_ScrollDown: event.scroll = -1; break;
            case XK_ScrollLeft: g.input.touchpad_move_x = -1; break;
            case XK_ScrollRight: g.input.touchpad_move_x = 1; break;
          }
          g.input.mouse_scroll = event.scroll;
          event.type = OS_EventKind_Scroll;
        }
        g.input_events.add(event);
      } break;
      case XCB_BUTTON_RELEASE: {
        xcb_button_press_event_t* bp = (xcb_button_press_event_t*)event;
        OS_InputEvent event = {};
        if (bp->detail >= XK_MouseLeft && bp->detail <= XK_MouseRight) {
          switch (bp->detail) {
            case XK_MouseLeft: g.input.keyboard_current.keys[MouseKey_Left] = false; event.key = MouseKey_Left; break;
            case XK_MouseMiddle: g.input.keyboard_current.keys[MouseKey_Middle] = false; event.key = MouseKey_Middle; break;
            case XK_MouseRight: g.input.keyboard_current.keys[MouseKey_Right] = false; event.key = MouseKey_Right; break;
          }
          event.type = OS_EventKind_MouseButton;
          event.is_pressed = false;
        }
        g.input_events.add(event);
      } break;
      case XCB_MOTION_NOTIFY: {
        xcb_motion_notify_event_t* motion = (xcb_motion_notify_event_t*)event;
        g.input.mouse_current.x = motion->event_x;
        g.input.mouse_current.y = motion->event_y;
        OS_InputEvent event = {.type = OS_EventKind_MouseMove};
        event.x = g.input.mouse_current.x;
        event.y = g.input.mouse_current.y;
        g.input_events.add(event);
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
  g.xcb_events.clear();
}

void os_clipboard_write(String str) {
  X11State& g = gfx_st;
  g.clipboard.str_to_write.clear();
  g.clipboard.str_to_write.add(str);
  xcb_set_selection_owner(g.connection, g.window, g.clipboard.atom, XCB_CURRENT_TIME);
  xcb_flush(g.connection);
}

String os_clipboard_read() {
  X11State& g = gfx_st;
  xcb_convert_selection(g.connection, g.window, g.clipboard.atom, g.clipboard.utf8_atom, g.clipboard.property_atom, XCB_CURRENT_TIME);
  xcb_flush(g.connection);

  xcb_generic_event_t* event;
  while (true) {
    event = xcb_wait_for_event(g.connection);
    u8 type = event->response_type & ~0x80;
    if (type == XCB_SELECTION_REQUEST) {
      xcb_window_t owner = xcb_get_selection_owner_reply(g.connection, xcb_get_selection_owner(g.connection, g.clipboard.atom), NULL)->owner;
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
        g.clipboard.str_to_read.clear();
        g.clipboard.str_to_read.add(String(data, len));
      }
      break;
    }
    else {
      add_event:
      g.xcb_events.add(event);
    }
  }
  return g.clipboard.str_to_read;
}

b32 os_window_should_close() { return gfx_st.should_close; }
v2u os_get_window_size() { return v2u(gfx_st.width, gfx_st.height); }
v2 os_get_mouse_pos() { return v2(gfx_st.input.mouse_current.x, gfx_st.input.mouse_current.y); }
f32 os_get_scroll() { return gfx_st.input.mouse_scroll; }
f32  os_get_touchpad() { return gfx_st.input.touchpad_move_x; }
void os_close_window() { gfx_st.should_close = true; }

void os_get_gfx_api_handlers(void* out) {
  struct Surface {
    xcb_connection_t* connection;
    xcb_window_t window;
  };
  *(Surface*)out = { gfx_st.connection, gfx_st.window };
}

Slice<OS_InputEvent> os_get_events() { return {gfx_st.input_events.data, gfx_st.input_events.count}; }

void os_input_update() {
  MemCopyStruct(&gfx_st.input.keyboard_previous, &gfx_st.input.keyboard_current);
  MemCopyStruct(&gfx_st.input.mouse_previous, &gfx_st.input.mouse_current);
}

b32 os_is_key_down(Key key)       { return gfx_st.input.keyboard_current.keys[key] == true; }
b32 os_is_key_up(Key key)         { return gfx_st.input.keyboard_current.keys[key] == false; }
b32 os_was_key_down(Key key)      { return gfx_st.input.keyboard_previous.keys[key] == true; }
b32 os_was_key_up(Key key)        { return gfx_st.input.keyboard_previous.keys[key] == false; }
b32 os_is_key_pressed(Key key)    { return os_is_key_down(key) && os_was_key_up(key); }
b32 os_is_key_released(Key key)   { return os_is_key_up(key) && os_was_key_down(key); }

#endif
