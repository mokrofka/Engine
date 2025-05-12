#pragma once
#include "lib.h"

enum ResType {
  ResType_Text,
  ResType_Binary,
  ResType_Image,
  ResType_Material,
  ResType_StaticMesh,
  ResType_Custom,
};

struct Res {
  u32 loader_id;
  String64 name64;
  String64 file_path64;
  u64 data_size;
  u8* data;
};

struct Binary {
  String64 file_path64;
  u64 size;
  u8* data;
};

// struct TextureRes {
//   String64 file_path64;
//   u8 channel_count;
//   u32 width;
//   u32 height;
//   u8* pixels;
// };


struct Texture {
  u32 id;
  u32 width;
  u32 height;
  u8 channel_count;
  b8 has_transparency;
  u32 generation;
  String64 file_path64;
  u8* data;
  void* internal_data;
};

enum TextureUse {
  TextureUse_Unknown = 0x00,
  TextureUse_MapDiffuse  = 0x01,
};

struct TextureMap {
  Texture* texture;
  TextureUse use;
};

enum MaterialType {
  MaterialType_World,
  MaterialType_UI,
};

struct MaterialRes {
  String64 name64;
  String64 diffuse_map_name64;
  MaterialType type;
  b8 auto_release;
  v4 diffuse_color;
};

struct MaterialConfig {
  String64 name64;
  MaterialType type;
  b8 auto_release;
  v4 diffuse_color;
  String64 diffuse_map_name64;
};

struct Material {
  u32 id;
  u32 generation;
  u32 internal_id;
  MaterialType type;
  String64 name64;
  v4 diffuse_color;
  TextureMap diffuse_map;
};

struct Geometry {
  u32 id;
  u32 internal_id;
  u32 generation;
  String64 name64;
  Material* material;
  
  u32 vertex_size;
  u32 vertex_count;
  void* vertices;
  u32 index_size;
  u32 index_count;
  void* indices;
};

enum ShaderStage {
  ShaderStage_Vertex = 0x00000001,
  ShaderStage_Geometry = 0x00000002,
  ShaderStage_Fragment = 0x00000004,
  ShaderStage_Compute = 0x0000008
};

enum ShaderAttributeType {
  ShaderAttribType_f32,
  ShaderAttribType_v2,
  ShaderAttribType_v3,
  ShaderAttribType_v4,
  ShaderAttribType_mat4,
  ShaderAttribType_i8,
  ShaderAttribType_u8,
  ShaderAttribType_i16,
  ShaderAttribType_u16,
  ShaderAttribType_i32,
  ShaderAttribType_u32,
};

enum ShaderUniformType {
  ShaderUniformType_f32,
  ShaderUniformType_v2,
  ShaderUniformType_v3,
  ShaderUniformType_v4,
  ShaderUniformType_i8,
  ShaderUniformType_u8,
  ShaderUniformType_i16,
  ShaderUniformType_u16,
  ShaderUniformType_i32,
  ShaderUniformType_u32,
  ShaderUniformType_mat4,
  ShaderUniformType_Sampler,
  ShaderUniformType_Custom = 255u
};

enum ShaderScope {
  ShaderScope_Global,
  ShaderScope_Instance,
  ShaderScope_Local
};

struct ShaderAttributeConfig {
  u8 name_length;
  char* name;
  u8 size;
  ShaderAttributeType type;
};

struct ShaderUniformConfig {
  u8 name_length;
  char* name;
  u8 size;
  u32 location;
  ShaderUniformType type;
  ShaderScope scope;
};

// struct ShaderConfig {
//   String64 name64;

//   b8 use_instances;
//   b8 use_local;

//   u8 attribute_count;
//   ShaderAttributeConfig* attributes;

//   u8 uniform_count;
//   ShaderUniformConfig* uniforms;

//   String64 renderpass_name64;

//   u8 stage_count;
//   ShaderStage* stages;
//   String* stage_names;
//   String* stage_filenames;
// };

struct ShaderConfig {
  String name;
  b8 has_position;
};
