#include "imgui.h"
#include "imgui_impl_x11.h"
#include "common.h"

void imgui_impl_x11_init() {
  ImGuiIO& io = ImGui::GetIO();
  io.BackendPlatformName = "imgui_impl_x11";
}

ImGuiKey imgui_keycode_translate(Key key) {
  switch (key) {
    case Key_Tab:         return ImGuiKey_Tab;
    case Key_Left:        return ImGuiKey_LeftArrow;
    case Key_Right:       return ImGuiKey_RightArrow;
    case Key_Up:          return ImGuiKey_UpArrow;
    case Key_Down:        return ImGuiKey_DownArrow;

    case Key_Pageup:      return ImGuiKey_PageUp;
    case Key_Pagedown:    return ImGuiKey_PageDown;
    case Key_Home:        return ImGuiKey_Home;
    case Key_End:         return ImGuiKey_End;
    case Key_Delete:      return ImGuiKey_Delete;

    case Key_Backspace:   return ImGuiKey_Backspace;
    case Key_Space:       return ImGuiKey_Space;
    case Key_Enter:       return ImGuiKey_Enter;
    case Key_Escape:      return ImGuiKey_Escape;

    case Key_Capslock:    return ImGuiKey_CapsLock;
    case Key_Numlock:     return ImGuiKey_NumLock;
    case Key_Printscreen: return ImGuiKey_PrintScreen;
    case Key_Pause:       return ImGuiKey_Pause;

    case Key_LShift:      return ImGuiKey_LeftShift;
    case Key_RShift:      return ImGuiKey_RightShift;
    case Key_LControl:    return ImGuiKey_LeftCtrl;
    case Key_RControl:    return ImGuiKey_RightCtrl;
    case Key_LAlt:        return ImGuiKey_LeftAlt;
    case Key_RAlt:        return ImGuiKey_RightAlt;
    case Key_Lsuper:      return ImGuiKey_LeftSuper;
    case Key_Rsuper:      return ImGuiKey_RightSuper;

    // Digits
    case Key_0: return ImGuiKey_0;
    case Key_1: return ImGuiKey_1;
    case Key_2: return ImGuiKey_2;
    case Key_3: return ImGuiKey_3;
    case Key_4: return ImGuiKey_4;
    case Key_5: return ImGuiKey_5;
    case Key_6: return ImGuiKey_6;
    case Key_7: return ImGuiKey_7;
    case Key_8: return ImGuiKey_8;
    case Key_9: return ImGuiKey_9;

    // Letters
    case Key_A: return ImGuiKey_A;
    case Key_B: return ImGuiKey_B;
    case Key_C: return ImGuiKey_C;
    case Key_D: return ImGuiKey_D;
    case Key_E: return ImGuiKey_E;
    case Key_F: return ImGuiKey_F;
    case Key_G: return ImGuiKey_G;
    case Key_H: return ImGuiKey_H;
    case Key_I: return ImGuiKey_I;
    case Key_J: return ImGuiKey_J;
    case Key_K: return ImGuiKey_K;
    case Key_L: return ImGuiKey_L;
    case Key_M: return ImGuiKey_M;
    case Key_N: return ImGuiKey_N;
    case Key_O: return ImGuiKey_O;
    case Key_P: return ImGuiKey_P;
    case Key_Q: return ImGuiKey_Q;
    case Key_R: return ImGuiKey_R;
    case Key_S: return ImGuiKey_S;
    case Key_T: return ImGuiKey_T;
    case Key_U: return ImGuiKey_U;
    case Key_V: return ImGuiKey_V;
    case Key_W: return ImGuiKey_W;
    case Key_X: return ImGuiKey_X;
    case Key_Y: return ImGuiKey_Y;
    case Key_Z: return ImGuiKey_Z;

    // Function keys
    case Key_F1:  return ImGuiKey_F1;
    case Key_F2:  return ImGuiKey_F2;
    case Key_F3:  return ImGuiKey_F3;
    case Key_F4:  return ImGuiKey_F4;
    case Key_F5:  return ImGuiKey_F5;
    case Key_F6:  return ImGuiKey_F6;
    case Key_F7:  return ImGuiKey_F7;
    case Key_F8:  return ImGuiKey_F8;
    case Key_F9:  return ImGuiKey_F9;
    case Key_F10: return ImGuiKey_F10;
    case Key_F11: return ImGuiKey_F11;
    case Key_F12: return ImGuiKey_F12;

    // Symbols
    case Key_Semicolon:   return ImGuiKey_Semicolon;
    case Key_Equal:       return ImGuiKey_Equal;
    case Key_Comma:       return ImGuiKey_Comma;
    case Key_Minus:       return ImGuiKey_Minus;
    case Key_Dot:         return ImGuiKey_Period;
    case Key_Slash:       return ImGuiKey_Slash;
    case Key_Grave:       return ImGuiKey_GraveAccent;
    case Key_LBracket:    return ImGuiKey_LeftBracket;
    case Key_Backslash:   return ImGuiKey_Backslash;
    case Key_RBracket:    return ImGuiKey_RightBracket;
    case Key_Apostrophe:  return ImGuiKey_Apostrophe;

    // Mouse
    case MouseKey_Left:   return (ImGuiKey)ImGuiMouseButton_Left;
    case MouseKey_Right:  return (ImGuiKey)ImGuiMouseButton_Right;
    case MouseKey_Middle: return (ImGuiKey)ImGuiMouseButton_Middle;

    default:
    return ImGuiKey_None;
  }
}

void imgui_impl_x11_new_frame() {
  ImGuiIO& io = ImGui::GetIO();
  io.DeltaTime = g_dt;
  v2u win_size = os_get_window_size();
  io.DisplaySize = ImVec2(win_size.x, win_size.y);
  v2 mouse_pos = os_get_mouse_pos();
  Slice<OS_InputEvent> events = os_get_events();
  for (OS_InputEvent event : events) {
    switch (event.type) {
      case OS_EventKind_Key: {
        ImGuiKey key = imgui_keycode_translate(event.key);
        if (FlagHas(event.modifier, OS_Modifier_Shift)) {
          io.AddKeyEvent(ImGuiMod_Shift, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Shift, false);
        }
        if (FlagHas(event.modifier, OS_Modifier_Alt)) {
          io.AddKeyEvent(ImGuiMod_Alt, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Alt, false);
        }
        if (FlagHas(event.modifier, OS_Modifier_Ctrl)) {
          io.AddKeyEvent(ImGuiMod_Ctrl, true);
        } else {
          io.AddKeyEvent(ImGuiMod_Ctrl, false);
        }
        io.AddKeyEvent(key, event.is_pressed);
        if (event.is_pressed) {
          io.AddInputCharacter(os_key_to_str(event.key, event.modifier));
        }
      } break;
      case OS_EventKind_MouseButton: {
        ImGuiKey key = imgui_keycode_translate(event.key);
        io.AddMouseButtonEvent(key, event.is_pressed);
      } break;
      case OS_EventKind_MouseMove: {
        io.AddMousePosEvent(event.x, event.y);
      } break;
      case OS_EventKind_Scroll: {
        io.AddMouseWheelEvent(event.scroll_x, event.scroll_y);
      } break;
    }
  }
}


