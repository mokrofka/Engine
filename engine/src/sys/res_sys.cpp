#include "res_sys.h"

#include "res/loaders/binary_loader.h"
#include "res/loaders/image_loader.h"
#include "res/loaders/material_loader.h"

struct ResSysState {
  ResSysConfig config;
  ResLoader* registered_loaders;
};

global ResSysState* state;

b32 load(String name, ResLoader* loader, Res* out_res);

void res_sys_init(Arena* arena, ResSysConfig config) {
  Assert(config.max_loader_count);
  
  state = push_struct(arena, ResSysState);
  state->config = config;
  state->registered_loaders = push_array(arena, ResLoader, config.max_loader_count);
  
  Loop (i, config.max_loader_count) {
    state->registered_loaders[i].id = INVALID_ID;
  }
  
  // NOTE Auto-register known loader types here
  res_sys_register_loader(binary_res_loader_create());
  res_sys_register_loader(image_resource_loader_create());
  res_sys_register_loader(material_res_loader_create());
  
  Info("Resource system initialized with base path '%s'", config.asset_base_path);
}

void res_sys_shutdown() {

}

b32 res_sys_register_loader(ResLoader loader) {
  u32 count = state->config.max_loader_count;
  Loop (i, count) {
    ResLoader* l = &state->registered_loaders[i];
    if (l->id != INVALID_ID) {
      if (l->type == loader.type) {
        Error("res_sys_register_laoder - Loader of type %i already exists and will not be registered", loader.type);
        return false;
      } else if (loader.custom_type64 && loader.custom_type64.size > 0 && str_matchi(l->custom_type64, loader.custom_type64)) {
        Error("res_sys_register_laoder - Loader of type %i already exists and will not be registered", loader.type);
        return false;
      }
    }
  }
  Loop (i, count) {
    if (state->registered_loaders[i].id == INVALID_ID) {
      state->registered_loaders[i] = loader;
      state->registered_loaders[i].id = i;
      Trace("Loader registered"_);
      return true;
    }
  }
  
  
  return false;
}

b32 res_sys_load(String name, ResType type, Res* out_res) {
  if (type != ResType_Custom) {
    // Select loader
    u32 count = state->config.max_loader_count;
    Loop (i, count) {
      ResLoader* l = &state->registered_loaders[i];
      if (l->id != INVALID_ID && l->type == type) {
        return load(name, l, out_res);
      }
    }
  }
  
  out_res->loader_id = INVALID_ID;
  Error("resoruce_system_load - No loader for type %i was found", type);
  return false;
}

b32 res_sys_load_custom(String name, String custom_type, Res* out_res) {
  if (custom_type) {
    // Select loader
    u32 count = state->config.max_loader_count;
    Loop (i, count) {
      ResLoader* l = &state->registered_loaders[i];
      if (l->id != INVALID_ID && l->type == ResType_Custom && str_matchi(l->custom_type64, custom_type)) {
        return load(name, l, out_res);
      }
    }
  }
  
  out_res->loader_id = INVALID_ID;
  Error("resoruce_system_load_custom - No loader for type %i was found", custom_type);
  return false;
}

void res_sys_unload(Res* res) {
  Assert(res);
  if (res->loader_id != INVALID_ID) {
    ResLoader* l = &state->registered_loaders[res->loader_id];
    if (l->id != INVALID_ID && l->unload) {
      l->unload(l, res);
    }
  }
}

String res_sys_base_path() {
  return state->config.asset_base_path;
}

b32 load(String name, ResLoader* loader, Res* out_res) {
  Assert(name && loader && loader->load && out_res);

  out_res->loader_id = loader->id;
  return loader->load(loader, name, out_res);
}
