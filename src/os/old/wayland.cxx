#include "lib.h"
#include "sys/socket.h"
#include <sys/un.h>

//////////////////////////////////////////////////////////////////////
// Buffer write/read helpers

intern void buf_write_u32(u8* buf, u64* buf_size, u64 buf_cap, u32 x) {
  Assert(*buf_size + sizeof(u32) <= buf_cap);
  Assert(IsAligned((u64)Offset(buf, *buf_size), sizeof(u32)));
  *(u32*)Offset(buf, *buf_size) = x;
  *buf_size += sizeof(u32);
}

intern void buf_write_u16(u8* buf, u64* buf_size, u64 buf_cap, u16 x) {
  Assert(*buf_size + sizeof(16) <= buf_cap);
  Assert(IsAligned((u64)Offset(buf, *buf_size), sizeof(u16)));
  *(u16*)Offset(buf, *buf_size) = x;
  *buf_size += sizeof(u16);
}

intern void buf_write_string(u8* buf, u64* buf_size, u64 buf_cap, String str) {
  Assert(*buf_size + str.size <= buf_cap);
  buf_write_u32(buf, buf_size, buf_cap, str.size);
  MemCopy(Offset(buf, *buf_size), str.str, str.size);
  *buf_size += AlignUp(str.size, sizeof(u32));
}

intern u32 buf_read_u32(u8** buf, u64* buf_size) {
  Assert(*buf_size >= sizeof(u32));
  Assert(IsAligned((u64)*buf, sizeof(u32)));
  u32 res = *(u32*)(*buf);
  *buf += sizeof(u32);
  *buf_size -= sizeof(u32);
  return res;
}

intern u16 buf_read_u16(u8** buf, u64* buf_size) {
  Assert(*buf_size >= sizeof(u16));
  Assert(IsAligned((u64)*buf, sizeof(u16)));
  u16 res = *(u16*)(*buf);
  *buf += sizeof(u16);
  *buf_size -= sizeof(u16);
  return res;
}

intern void buf_read_n(u8** buf, u64* buf_size, u8* dst, u64 n) {
  Assert(*buf_size >= n);
  MemCopy(dst, *buf, n);
  *buf += n;
  *buf_size -= n;
}

////////////////////////////////////////////////////////////////////////
// Wayland

const u32 wl_display_object_id = 1;
const u16 wl_display_request_get_registry_opcode = 1;
const u16 wl_display_event_error = 0;
const u16 wl_registry_request_bind_opcode = 0;
const u16 wl_registry_event_global_opcode = 0;
const u16 wl_compositor_request_create_surface_opcode = 0;
const u16 wl_surface_request_commit_opcode = 6;

const u16 xdg_wm_base_request_get_xdg_surface_opcode = 2;
const u16 xdg_wm_base_request_pong_opcode = 3;
const u16 xdg_wm_base_event_ping = 0;
const u16 xdg_surface_request_get_toplevel_opcode = 1;
const u16 xdg_surface_request_ack_configure_opcode = 4;
const u16 xdg_surface_event_configure = 0;
const u16 xdg_toplevel_event_configure = 0;

const u32 wl_header_size = 8;

struct WlState {
  int fd;
  u32 current_id = 1;
  u32 registry;
  u32 compositor;
  u32 surface;
  u32 xdg_wm_base;
  u32 xdg_surface;
  u32 xdg_toplevel;
  u32 seat;

  u32 w = 1;
  u32 h = 1;
};

global WlState wl_st;

intern u32 wl_registry_bind(u32 name, String interface, u32 version) {
  u64 msg_size = 0;
  u8 msg[512] = {};
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.registry);
  buf_write_u16(msg, &msg_size, sizeof(msg), wl_registry_request_bind_opcode);
  u16 msg_announced_size =
      wl_header_size + sizeof(name) + sizeof(u32) +
      AlignUp(interface.size, sizeof(u32)) + sizeof(version) + sizeof(wl_st.current_id);
  Assert(AlignUp(msg_announced_size, sizeof(u32)) == msg_announced_size);
  buf_write_u16(msg, &msg_size, sizeof(msg), msg_announced_size);
  buf_write_u32(msg, &msg_size, sizeof(msg), name);
  buf_write_string(msg, &msg_size, sizeof(msg), interface);
  buf_write_u32(msg, &msg_size, sizeof(msg), version);
  ++wl_st.current_id;
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.current_id);
  Assert(msg_size == AlignUp(msg_size, sizeof(u32)));
  if ((i64)msg_size != send(wl_st.fd, msg, msg_size, 0)) {
    Assert(false);
  }
  Info("-> wl_registry@%u.bind: name=%u interface=%s version=%u "
      "wayland_current_id=%u",
      wl_st.registry, name, interface, version, wl_st.current_id);
  return wl_st.current_id;
}

intern void xdg_wm_base_pong(u32 ping) {
  Assert(wl_st.xdg_wm_base > 0);
  Assert(wl_st.surface > 0);
  u64 msg_size = 0;
  u8 msg[128] = {};
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.xdg_wm_base);
  buf_write_u16(msg, &msg_size, sizeof(msg), xdg_wm_base_request_pong_opcode);
  u16 msg_announced_size = wl_header_size + sizeof(ping);
  Assert(AlignUp(msg_announced_size, sizeof(u32)) == msg_announced_size);
  buf_write_u16(msg, &msg_size, sizeof(msg), msg_announced_size);
  buf_write_u32(msg, &msg_size, sizeof(msg), ping);
  if ((i64)msg_size != send(wl_st.fd, msg, msg_size, 0)) {
    Assert(false);
  }
  Info("-> xdg_wm_base@%u.pong: ping=%u", wl_st.xdg_wm_base, ping);
}

intern void xdg_surface_ack_configure(u32 configure) {
  Assert(wl_st.xdg_surface > 0);
  u64 msg_size = 0;
  u8 msg[128] = {};
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.xdg_surface);
  buf_write_u16(msg, &msg_size, sizeof(msg), xdg_surface_request_ack_configure_opcode);
  u16 msg_announced_size = wl_header_size + sizeof(configure);
  Assert(AlignUp(msg_announced_size, sizeof(u32)) == msg_announced_size);
  buf_write_u16(msg, &msg_size, sizeof(msg), msg_announced_size);
  buf_write_u32(msg, &msg_size, sizeof(msg), configure);
  if ((int64_t)msg_size != send(wl_st.fd, msg, msg_size, 0)) {
    Assert(false);
  }
  Info("-> xdg_surface@%u.ack_configure: configure=%u", wl_st.xdg_surface, configure);
}

intern void wl_handle_message(u8** msg, u64* msg_size) {
  Assert(*msg_size >= 8);
  u64 start_msg_size = *msg_size;
  u32 object_id = buf_read_u32(msg, msg_size);
  // Assert(object_id <= wl_st.current_id);
  u16 opcode = buf_read_u16(msg, msg_size);
  u16 announced_size = buf_read_u16(msg, msg_size);
  // Assert(AlignUp(announced_size, sizeof(u32)) <= announced_size);
  // Assert(AlignUp(announced_size, sizeof(u32)) <= wl_header_size + *msg_size);
   if (object_id == wl_st.xdg_wm_base && opcode == xdg_wm_base_event_ping) {
     u32 ping = buf_read_u32(msg, msg_size);
     Info("<- xdg_wm_base@%u.ping: ping=%u", wl_st.xdg_wm_base, ping);
     xdg_wm_base_pong(ping);
     return;
  }
  else if (object_id == wl_st.xdg_surface && opcode == xdg_surface_event_configure) {
     u32 configure = buf_read_u32(msg, msg_size);
     Info("<- xdg_surface@%u.configure: configure=%u", wl_st.xdg_surface, configure);
     xdg_surface_ack_configure(configure);
     return;
   }
  else if (object_id == wl_display_object_id && opcode == wl_display_event_error) {
    i32 a = 1;
    Assert(false);
  }
  else if (object_id == wl_st.xdg_toplevel && opcode == xdg_toplevel_event_configure) {
    u32 w = buf_read_u32(msg, msg_size);
    u32 h = buf_read_u32(msg, msg_size);
    u32 size = buf_read_u32(msg, msg_size);
    u8 buf[256] = {};
    Assert(size <= sizeof(buf));
    buf_read_n(msg, msg_size, buf, size);
    Info("<- xdg_toplevel@%u.configure: w=%u h=%u states[%u]\n", wl_st.xdg_toplevel, w, h, size);
    if (w && h && (w != wl_st.w || h != wl_st.h)) { // Resize.
      wl_st.w = w;
      wl_st.h = h;
    }
  }
  else if (object_id == wl_st.registry && opcode == wl_registry_event_global_opcode) {
    u32 name = buf_read_u32(msg, msg_size);
    u32 interface_size = buf_read_u32(msg, msg_size);
    u32 padded_interface_size = AlignUp(interface_size, sizeof(u32));
    u8 interface[512] = {};
    buf_read_n(msg, msg_size, interface, padded_interface_size);
    Assert(interface[interface_size - 1] == 0);
    u32 version = buf_read_u32(msg, msg_size);
    Info("<- wl_registry@%u.global: name=%u interface=%s version=%u",
         wl_st.registry, name, String(interface, interface_size), version);
    Assert(announced_size == sizeof(object_id) + sizeof(announced_size) +
                             sizeof(opcode) + sizeof(name) +
                             sizeof(interface_size) + padded_interface_size +
                             sizeof(version));
    if (str_match("wl_compositor", interface)) {
      Assert(wl_st.compositor == 0);
      wl_st.compositor = wl_registry_bind(name, String(interface, interface_size), version);
    }
    else if (str_match("wl_seat", interface)) {
      Assert(wl_st.seat == 0);
      wl_st.seat = wl_registry_bind(name, String(interface, interface_size), version);
    }
    else if (str_match("xdg_wm_base", interface)) {
      Assert(wl_st.xdg_wm_base == 0);
      wl_st.xdg_wm_base = wl_registry_bind(name, String(interface, interface_size), version);
    }
  }
}

intern u32 wl_compositor_create_surface() {
  Assert(wl_st.compositor > 0);
  u64 msg_size = 0;
  u8 msg[128] = {};
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.compositor);
  buf_write_u16(msg, &msg_size, sizeof(msg), wl_compositor_request_create_surface_opcode);
  u16 msg_announced_size = wl_header_size + sizeof(wl_st.current_id);
  Assert(AlignUp(msg_announced_size, sizeof(u32)) == msg_announced_size);
  buf_write_u16(msg, &msg_size, sizeof(msg), msg_announced_size);
  ++wl_st.current_id;
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.current_id);
  if ((i64)msg_size != send(wl_st.fd, msg, msg_size, 0)) {
    Assert(false);
  }
  Info("-> wl_compositor@%u.create_surface: wl_surface=%u\n",
       wl_st.compositor, wl_st.current_id);
  return wl_st.current_id;
}

intern u32 xdg_wm_base_get_xdg_surface() {
  Assert(wl_st.xdg_wm_base > 0);
  Assert(wl_st.surface > 0);
  u64 msg_size = 0;
  u8 msg[128] = {};
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.xdg_wm_base);
  buf_write_u16(msg, &msg_size, sizeof(msg), xdg_wm_base_request_get_xdg_surface_opcode);
  uint16_t msg_announced_size = wl_header_size +
                                sizeof(wl_st.current_id) +
                                sizeof(wl_st.surface);
  Assert(AlignUp(msg_announced_size, sizeof(u32)) == msg_announced_size);
  buf_write_u16(msg, &msg_size, sizeof(msg), msg_announced_size);
  ++wl_st.current_id;
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.current_id);
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.surface);
  if ((int64_t)msg_size != send(wl_st.fd, msg, msg_size, 0)) {
    Assert(false);
  }
  Info("-> xdg_wm_base@%u.get_xdg_surface: xdg_surface=%u wl_surface=%u",
       wl_st.xdg_wm_base, wl_st.current_id, wl_st.surface);
  return wl_st.current_id;
}

intern u32 xdg_surface_get_toplevel() {
  Assert(wl_st.xdg_surface > 0);
  u64 msg_size = 0;
  u8 msg[128] = {};
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.xdg_surface);
  buf_write_u16(msg, &msg_size, sizeof(msg), xdg_surface_request_get_toplevel_opcode);
  u16 msg_announced_size = wl_header_size + sizeof(wl_st.current_id);
  Assert(AlignUp(msg_announced_size, sizeof(u32)) == msg_announced_size);
  buf_write_u16(msg, &msg_size, sizeof(msg), msg_announced_size);
  ++wl_st.current_id;
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.current_id);
  if ((i64)msg_size != send(wl_st.fd, msg, msg_size, 0)) {
    Assert(false);
  }
  Info("-> xdg_surface@%u.get_toplevel: xdg_toplevel=%u", wl_st.xdg_surface,
       wl_st.current_id);
  return wl_st.current_id;
}

static void wl_surface_commit() {
  Assert(wl_st.surface > 0);
  u64 msg_size = 0;
  u8 msg[128] = {};
  buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.surface);
  buf_write_u16(msg, &msg_size, sizeof(msg), wl_surface_request_commit_opcode);
  u16 msg_announced_size = wl_header_size;
  Assert(AlignUp(msg_announced_size, sizeof(u32)) == msg_announced_size);
  buf_write_u16(msg, &msg_size, sizeof(msg), msg_announced_size);
  if ((i64)msg_size != send(wl_st.fd, msg, msg_size, 0)) {
    Assert(false);
  }
  Info("-> wl_surface@%u.commit:", wl_st.surface);
}

void wl_init() {
  // Connect to wayland
  {
    Scratch scratch;
    String xdg_runtime_dir = os_get_environment("XDG_RUNTIME_DIR");
    if (!xdg_runtime_dir.str) {
      Assert(false);
    }
    String wayland_display = os_get_environment("WAYLAND_DISPLAY");
    if (!wayland_display.str) {
      Assert(false);
    }
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    String wayland_socket_path = push_strf(scratch, "%s/%s", xdg_runtime_dir, wayland_display);
    MemCopy(addr.sun_path, wayland_socket_path.str, wayland_socket_path.size);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    Assert(fd != -1);
    int res = connect(fd, (sockaddr*)&addr, sizeof(addr));
    Assert(res != -1);
    wl_st.fd = fd;
  }

  // Get registry
  {
    ++wl_st.current_id;
    u64 msg_size = 0;
    u8 msg[128] = {};
    buf_write_u32(msg, &msg_size, sizeof(msg), wl_display_object_id);
    buf_write_u16(msg, &msg_size, sizeof(msg), wl_display_request_get_registry_opcode);
    buf_write_u16(msg, &msg_size, sizeof(msg), wl_header_size + sizeof(u32));
    buf_write_u32(msg, &msg_size, sizeof(msg), wl_st.current_id);
    if ((i64)msg_size != send(wl_st.fd, msg, msg_size, MSG_DONTWAIT)) {
      Assert(false);
    }
    Info("-> wl_display@%u.get_registry: wl_registry=%u", wl_display_object_id, wl_st.current_id);
    wl_st.registry = wl_st.current_id;
  }

  // Handling all objects from registry
  {
    u8 read_buf[KB(4)] = {};
    u64 read_bytes = recv(wl_st.fd, read_buf, sizeof(read_buf), 0);
    if (read_bytes == -1) {
      Assert(false);
    }
    u8* msg = read_buf;
    u64 msg_size = read_bytes;
    while (msg_size > 0) {
      wl_handle_message(&msg, &msg_size);
    }
  }

  wl_st.surface = wl_compositor_create_surface();
  wl_st.xdg_surface = xdg_wm_base_get_xdg_surface();
  wl_st.xdg_toplevel = xdg_surface_get_toplevel();
  wl_surface_commit();

}


