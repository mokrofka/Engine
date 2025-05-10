#include "shader_sys.h"

#include "render/r_frontend.h"

struct ShaderSysState {
  Arena* arena;
  ShaderSysConfig config;
  HashMap lookup;
  u32 current_shader_id;
  Shader* shaders;
};

global ShaderSysState* state;

internal void add_attribute(Shader* Shader, const ShaderAttributeConfig* config);
internal void add_sampler(Shader* Shader, ShaderUniformConfig* config);
internal void add_uniform(Shader* Shader, ShaderUniformConfig* config);
internal u32 get_shader_id(const char* shader_name);
internal u32 new_shader_id();
internal void uniform_add(Shader* Shader, const char* uniform_name, u32 size, ShaderUniformType type, ShaderScope scope, u32 set_location, b8 is_sampler);
internal b8 uniform_name_valid(Shader* Shader, const char* uniform_name);
internal b8 shader_uniform_add_state_valid(Shader* Shader);
internal void shader_destroy(Shader* s);

void shader_sys_init(Arena* arena, ShaderSysConfig config) {
  state = push_struct(arena, ShaderSysState);
  state->arena = arena_alloc(arena, config.mem_reserve);
  state->shaders = push_array(arena, Shader, config.shader_count_max);
  state->config = config;
  state->current_shader_id = INVALID_ID;

  state->lookup = hashmap_create(arena, sizeof(u32), config.shader_count_max, false);

  u32 invalid_fill_id = INVALID_ID;
  hashmap_fill(&state->lookup, &invalid_fill_id);

  Loop (i, state->config.shader_count_max) {
    state->shaders[i].id = INVALID_ID;
  }
}

void shader_sys_shutdown() {
  Loop(i, state->config.shader_count_max) {
    Shader* s = &state->shaders[i];
    if (s->id != INVALID_ID) {
      // shader_destroy(s);
    }
  }
  hashmap_destroy(&state->lookup);
}

void shader_sys_create(ShaderConfig* config) {
  u32 id = new_shader_id();
  Shader* out_shader = &state->shaders[id];
  out_shader->id = id;

  Assert(out_shader->id != INVALID_ID && "Unable to find free slot to create new shader. Aborting")

  out_shader->state = ShaderState_NotCreated;
  str_copy(out_shader->name64, config->name64);
  out_shader->use_instances = config->use_instances;
  out_shader->use_locals = config->use_local;
  out_shader->push_constant_range_count = 0;
  out_shader->bound_instance_id = INVALID_ID;
  out_shader->attribute_stride = 0;

  // Setup arrays
  out_shader->global_textures = darray_create(Texture*);
  out_shader->uniforms = darray_create(ShaderUniform);
  out_shader->attributes = darray_create(ShaderAttribute);

  // Create a hashtable to store uniform array indexes. This provides a direct index into the
  // 'uniforms' array stored in the shader for quick lookups by name.
  u32 element_size = sizeof(u16); // Indexes are stored as u16s.
  u32 element_count = 1024;       // This is more uniforms than we will ever need, but a bigger table reduces collision chance.
  out_shader->uniform_lookup = hashmap_create(state->arena, element_size, element_count, false);

  // Invalidate all spots in the hashtable.
  u32 invalid = INVALID_ID;
  hashmap_fill(&out_shader->uniform_lookup, &invalid);

  out_shader->global_ubo_size = 0;
  out_shader->ubo_size = 0;

  // This is hard-coded because the Vulkan spec only guarantees that a _minimum_ 128 bytes of space are available,
  // and it's up to the driver to determine how much is available. Therefore, to avoid complexity, only the
  // lowest common denominator of 128B will be used.
  out_shader->push_constant_stride = 128;
  out_shader->push_constant_size = 0;

  u32 renderpass_id = r_renderpass_id(config->renderpass_name64);
  r_shader_create(out_shader, renderpass_id, config->stage_count, config->stage_filenames, config->stages);

  // Ready to be initialized.
  out_shader->state = ShaderState_Uninitialized;

  // Process attributes
  Loop (i, config->attribute_count) {
    add_attribute(out_shader, &config->attributes[i]);
  }

  // Process uniforms
  // for (u32 i = 0; i < config->uniform_count; ++i) {
  //   if (config->uniforms[i].type == SHADER_UNIFORM_TYPE_SAMPLER) {
  //     add_sampler(out_shader, &config->uniforms[i]);
  //   } else {
  //     add_uniform(out_shader, &config->uniforms[i]);
  //   }
  // }

  // // Initialize the shader.
  // if (!renderer_shader_initialize(out_shader)) {
  //   KERROR("shader_sys_create: initialization failed for shader '%s'.", config->name);
  //   // NOTE: initialize automatically destroys the shader if it fails.
  //   return false;
  // }

  // // At this point, creation is successful, so store the shader id in the hashtable
  // // so this can be looked up by name later.
  // if (!hashtable_set(&state->lookup, config->name, &out_shader->id)) {
  //   // Dangit, we got so far... welp, nuke the shader and boot.
  //   renderer_shader_destroy(out_shader);
  //   return false;
  // }

  // return true;
}

u32 shader_sys_get_id(const char* shader_name) {
  return get_shader_id(shader_name);
}

Shader* shader_sys_get_by_id(u32 shader_id) {
  if (shader_id >= state->config.shader_count_max || state->shaders[shader_id].id == INVALID_ID) {
    return 0;
  }
  return &state->shaders[shader_id];
}

Shader* shader_sys_get(const char* shader_name) {
  u32 shader_id = get_shader_id(shader_name);
  if (shader_id != INVALID_ID) {
    return shader_sys_get_by_id(shader_id);
  }
  return 0;
}

void shader_destroy(Shader* s) {
  r_shader_destroy(s);

  // Set it to be unusable right away.
  s->state = ShaderState_NotCreated;

  s->name64.size = 0;
}

void shader_sys_destroy(const char* shader_name) {
  u32 shader_id = get_shader_id(shader_name);
  if (shader_id == INVALID_ID) {
    return;
  }

  Shader* s = &state->shaders[shader_id];

  shader_destroy(s);
}

void shader_sys_use(const char* shader_name) {
  u32 next_shader_id = get_shader_id(shader_name);
  if (next_shader_id == INVALID_ID) {
  }

  return shader_sys_use_by_id(next_shader_id);
}

void shader_sys_use_by_id(u32 shader_id) {
  // Only perform the use if the shader id is different.
  if (state->current_shader_id != shader_id) {
    Shader* next_shader = shader_sys_get_by_id(shader_id);
    state->current_shader_id = shader_id;
    r_shader_use(next_shader);
    r_shader_bind_globals(next_shader);
  }
}

u32 shader_sys_uniform_index(Shader* s, String uniform_name) {
  if (!s || s->id == INVALID_ID) {
    Error("shader_sys_uniform_location called with invalid shader.");
    return INVALID_ID_U16;
  }

  u16 index = INVALID_ID_U16;
  // if (!hashtable_get(&s->uniform_lookup, uniform_name, &index) || index == INVALID_ID_U16) {
  //   KERROR("Shader '%s' does not have a registered uniform named '%s'", s->name, uniform_name);
  //   return INVALID_ID_U16;
  // }
  return s->uniforms[index].index;
}

void shader_sys_uniform_set(const char* uniform_name, const void* value) {
  // if (state->current_shader_id == INVALID_ID) {
  //   KERROR("shader_sys_uniform_set called without a shader in use.");
  //   return false;
  // }
  // shader* s = &state->shaders[state->current_shader_id];
  // u16 index = shader_sys_uniform_index(s, uniform_name);
  // return shader_sys_uniform_set_by_index(index, value);
}

void shader_sys_sampler_set(const char* sampler_name, const texture* t) {
  return shader_sys_uniform_set(sampler_name, t);
}

void shader_sys_uniform_set_by_index(u16 index, const void* value) {
  // shader* shader = &state->shaders[state->current_shader_id];
  // shader_uniform* uniform = &shader->uniforms[index];
  // if (shader->bound_scope != uniform->scope) {
  //   if (uniform->scope == SHADER_SCOPE_GLOBAL) {
  //     renderer_shader_bind_globals(shader);
  //   } else if (uniform->scope == SHADER_SCOPE_INSTANCE) {
  //     renderer_shader_bind_instance(shader, shader->bound_instance_id);
  //   } else {
  //     // NOTE: Nothing to do here for locals, just set the uniform.
  //   }
  //   shader->bound_scope = uniform->scope;
  // }
  // return renderer_set_uniform(shader, uniform, value);
}
void shader_sys_sampler_set_by_index(u16 index, const texture* t) {
  return shader_sys_uniform_set_by_index(index, t);
}

void shader_sys_apply_global() {
  // return renderer_shader_apply_globals(&state->shaders[state->current_shader_id]);
}
void shader_sys_apply_instance() {
  // return renderer_shader_apply_instance(&state->shaders[state->current_shader_id]);
}

void shader_sys_bind_instance(u32 instance_id) {
  // shader* s = &state->shaders[state->current_shader_id];
  // s->bound_instance_id = instance_id;
  // return renderer_shader_bind_instance(s, instance_id);
}

void add_attribute(Shader* shader, const ShaderAttributeConfig* config) {
  u32 size = 0;
  switch (config->type) {
  case ShaderAttribType_i8:
  case ShaderAttribType_u8:
    size = 1;
    break;
  case ShaderAttribType_i16:
  case ShaderAttribType_u16:
    size = 2;
    break;
  case ShaderAttribType_f32:
  case ShaderAttribType_i32:
  case ShaderAttribType_u32:
    size = 4;
    break;
  case ShaderAttribType_v2:
    size = 8;
    break;
  case ShaderAttribType_v3:
    size = 12;
    break;
  case ShaderAttribType_v4:
    size = 16;
    break;
  default:
    Error("Unrecognized type %i, defaulting to size of 4. This probably is not what is desired.", config->type);
    size = 4;
    break;
  }

  shader->attribute_stride += size;

  // Create/push the attribute.
  ShaderAttribute attrib = {};
  // attrib.name = string_duplicate(config->name);
  attrib.size = size;
  attrib.type = config->type;
  darray_push(shader->attributes, attrib);

  // return true;
}

void add_sampler(Shader* shader, ShaderUniformConfig* config) {
  // if (config->scope == SHADER_SCOPE_INSTANCE && !shader->use_instances) {
  //   KERROR("add_sampler cannot add an instance sampler for a shader that does not use instances.");
  //   return false;
  // }

  // // Samples can't be used for push constants.
  // if (config->scope == SHADER_SCOPE_LOCAL) {
  //   KERROR("add_sampler cannot add a sampler at local scope.");
  //   return false;
  // }

  // // Verify the name is valid and unique.
  // if (!uniform_name_valid(shader, config->name) || !shader_uniform_add_state_valid(shader)) {
  //   return false;
  // }

  // // If global, push into the global list.
  // u32 location = 0;
  // if (config->scope == SHADER_SCOPE_GLOBAL) {
  //   u32 global_texture_count = darray_length(shader->global_textures);
  //   if (global_texture_count + 1 > state->config.max_global_textures) {
  //     KERROR("Shader global texture count %i exceeds max of %i", global_texture_count, state->config.max_global_textures);
  //     return false;
  //   }
  //   location = global_texture_count;
  //   darray_push(shader->global_textures, texture_sys_get_default_texture());
  // } else {
  //   // Otherwise, it's instance-level, so keep count of how many need to be added during the resource acquisition.
  //   if (shader->instance_texture_count + 1 > state->config.max_instance_textures) {
  //     KERROR("Shader instance texture count %i exceeds max of %i", shader->instance_texture_count, state->config.max_instance_textures);
  //     return false;
  //   }
  //   location = shader->instance_texture_count;
  //   shader->instance_texture_count++;
  // }

  // Treat it like a uniform. NOTE: In the case of samplers, out_location is used to determine the
  // hashtable entry's 'location' field value directly, and is then set to the index of the uniform array.
  // This allows location lookups for samplers as if they were uniforms as well (since technically they are).
  // TODO: might need to store this elsewhere
  // if (!uniform_add(shader, config->name, 0, config->type, config->scope, location, true)) {
  //   KERROR("Unable to add sampler uniform.");
  //   return false;
  // }

  // return true;
}

void add_uniform(Shader* shader, ShaderUniformConfig* config) {
//   if (!shader_uniform_add_state_valid(shader) || !uniform_name_valid(shader, config->name)) {
//     return false;
//   }
//   return uniform_add(shader, config->name, config->size, config->type, config->scope, 0, false);
}

u32 get_shader_id(const char* shader_name) {
//   u32 shader_id = INVALID_ID;
//   if (!hashtable_get(&state->lookup, shader_name, &shader_id)) {
//     KERROR("There is no shader registered named '%s'.", shader_name);
//     return INVALID_ID;
//   }
//   return shader_id;
}

u32 new_shader_id() {
//   for (u32 i = 0; i < state->config.shader_count_max; ++i) {
//     if (state->shaders[i].id == INVALID_ID) {
//       return i;
//     }
//   }
//   return INVALID_ID;
}

void uniform_add(Shader* shader, const char* uniform_name, u32 size, ShaderUniformType type, ShaderScope scope, u32 set_location, b8 is_sampler) {
//   u32 uniform_count = darray_length(shader->uniforms);
//   if (uniform_count + 1 > state->config.max_uniform_count) {
//     KERROR("A shader can only accept a combined maximum of %d uniforms and samplers at global, instance and local scopes.", state->config.max_uniform_count);
//     return false;
//   }
//   shader_uniform entry;
//   entry.index = uniform_count; // Index is saved to the hashtable for lookups.
//   entry.scope = scope;
//   entry.type = type;
//   b8 is_global = (scope == SHADER_SCOPE_GLOBAL);
//   if (is_sampler) {
//     // Just use the passed in location
//     entry.location = set_location;
//   } else {
//     entry.location = entry.index;
//   }

//   if (scope != SHADER_SCOPE_LOCAL) {
//     entry.set_index = (u32)scope;
//     entry.offset = is_sampler ? 0 : is_global ? shader->global_ubo_size
//                                               : shader->ubo_size;
//     entry.size = is_sampler ? 0 : size;
//   } else {
//     if (entry.scope == SHADER_SCOPE_LOCAL && !shader->use_locals) {
//       KERROR("Cannot add a locally-scoped uniform for a shader that does not support locals.");
//       return false;
//     }
//     // Push a new aligned range (align to 4, as required by Vulkan spec)
//     entry.set_index = INVALID_ID_U8;
//     range r = get_aligned_range(shader->push_constant_size, size, 4);
//     // utilize the aligned offset/range
//     entry.offset = r.offset;
//     entry.size = r.size;

//     // Track in configuration for use in initialization.
//     shader->push_constant_ranges[shader->push_constant_range_count] = r;
//     shader->push_constant_range_count++;

//     // Increase the push constant's size by the total value.
//     shader->push_constant_size += r.size;
//   }

//   if (!hashtable_set(&shader->uniform_lookup, uniform_name, &entry.index)) {
//     KERROR("Failed to add uniform.");
//     return false;
//   }
//   darray_push(shader->uniforms, entry);

//   if (!is_sampler) {
//     if (entry.scope == SHADER_SCOPE_GLOBAL) {
//       shader->global_ubo_size += entry.size;
//     } else if (entry.scope == SHADER_SCOPE_INSTANCE) {
//       shader->ubo_size += entry.size;
//     }
//   }

//   return true;
}

b8 uniform_name_valid(Shader* shader, const char* uniform_name) {
//   if (!uniform_name || !string_length(uniform_name)) {
//     KERROR("Uniform name must exist.");
//     return false;
//   }
//   u16 location;
//   if (hashtable_get(&shader->uniform_lookup, uniform_name, &location) && location != INVALID_ID_U16) {
//     KERROR("A uniform by the name '%s' already exists on shader '%s'.", uniform_name, shader->name);
//     return false;
//   }
  return true;
}

b8 shader_uniform_add_state_valid(Shader* shader) {
//   if (shader->state != SHADER_STATE_UNINITIALIZED) {
//     KERROR("Uniforms may only be added to shaders before initialization.");
//     return false;
//   }
  return true;
}
