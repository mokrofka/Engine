#include "lib.slang"

static v3 triangle[] = {
  v3(-0.5, -0.5, 0),
  v3(0.5, -0.5, 0),
  v3(0.0, 0.5, 0),
};

VS_Entry VSOutput vs_main(VSInput in) {
  VSOutput o;
  v4 pos = v4(triangle[in.vert_id], 1);
  o.pos = st().projection_view * st().mat * pos;
  return o;
}

FS_Entry v4 fs_main(VSOutput in) {
  return v4(1.0, 0.0, 0.0, 1.0);
}

