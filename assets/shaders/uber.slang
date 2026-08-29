#include "lib.slang"

// printf("hello %v4f", in.pos);

global v2 positions[] = {
  {-1.0, -1.0},
  {-1.0,  3.0},
  { 3.0, -1.0},
};

global v2 tex_coords[] = {
  {0.0, 0.0},
  {0.0, 2.0},
  {2.0, 0.0}
};

VS_Entry VSOutput vs_main(Vertex in, u32 vert_id : SV_VertexID, u32 draw_id : SV_DrawIndex, u32 inst_id : SV_InstanceID) {
  // u32 drawcall_idx = drawinfo[push.idx + in.draw_id].base;
  // DrawCall draw = drawcalls[drawcall_idx];
  u32 drawcall_idx = push.idx + draw_id;
  DrawCall draw = drawcalls[drawcall_idx];
  VSOutput o;
  switch (draw.type) {
    case ShaderType::Cube: {
      m4x4 view = st.view;
      view[0][3] = 0;
      view[1][3] = 0;
      view[2][3] = 0;
      v4 pos = st.projection * view * v4(in.pos, 1);
      o.pos = v4(pos.xy, pos.w, pos.w);
      o.color = v4(in.pos, 1);
    } break;
    case ShaderType::E_Color: {
      o.pos = st.projection_view * draw.model * v4(in.pos, 1.0);
    } break;
    case ShaderType::Texture: {
      o.pos = st.projection_view * draw.model * v4(in.pos, 1.0);
      o.uv = in.uv;
    } break;
    case ShaderType::VertColor: {
      o.pos = st.projection_view * draw.model * v4(in.pos, 1.0);
      o.color = in.color;
    } break;
    case ShaderType::Line: {
      o.pos = st.projection_view * v4(in.pos, 1);
      o.color = in.color;
    } break;
    case ShaderType::Screen: {
      o.pos = v4(positions[vert_id], 0, 1);
      o.uv = tex_coords[vert_id];
    } break;
    case ShaderType::SoftwareRender: {
      o.pos = v4(positions[vert_id], 0, 1);
      o.uv = tex_coords[vert_id];
    } break;
  }
  o.id = drawcall_idx;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  v4 o = {};
  DrawCall draw = drawcalls[in.id];
  switch (draw.type) {
    case ShaderType::Cube: {
      o = cube_texture[st.cubemap].Sample(samplers[1], in.color.xyz);
    } break;
    case ShaderType::E_Color: {
      o = drawcalls[in.id].color;
    } break;
    case ShaderType::Texture: {
      u32 tex = materials[draw.mat].tex;
      o = textures[tex].Sample(samplers[1], in.uv);
    } break;
    case ShaderType::VertColor: {
      o = in.color;
    } break;
    case ShaderType::Line: {
      o = in.color;
    } break;
    case ShaderType::Screen: {
      o = textures[draw.cur_resolve_idx].Sample(samplers[1], in.uv);
    } break;
    case ShaderType::SoftwareRender: {
      f32 x = in.pos.x;
      f32 y = in.pos.y;
      o = rgba_from_u32(software_render[(u32)(y * st.res.x + x)]);
      if (o.w < 0.1) discard;
    } break;
  }
  return o;
}

