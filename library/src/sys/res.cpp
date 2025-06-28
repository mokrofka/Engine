#include "res.h"

struct ResSysState {
  Arena* arena;
  String asset_path;
};

global ResSysState st;

void res_sys_init(ResSysConfig config) {
  st = {
    st.arena = mem_arena_alloc(KB(1)),
    st.asset_path = push_strf(st.arena, "%s/%s", os_get_current_directory(), config.asset_base_path)
  };
}

String res_sys_base_path() {
  return st.asset_path;
}
