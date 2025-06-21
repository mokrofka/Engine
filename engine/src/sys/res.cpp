#include "res.h"

struct ResSysState {
  ResSysConfig config;
};

global ResSysState state;

void res_sys_init(Arena* arena, ResSysConfig config) {
  state.config = config;
}

String res_sys_base_path() {
  return state.config.asset_base_path;
}
