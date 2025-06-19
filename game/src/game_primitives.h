#pragma once
#include "lib.h"

f32 cube_vertices[] = {
  // Front face (0, 0, 1)
  -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
   0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,

  // Back face (0, 0, -1)
   0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,

  // Left face (-1, 0, 0)
  -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

  // Right face (1, 0, 0)
   0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
   0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
   0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

  // Bottom face (0, -1, 0)
  -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,
   0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
   0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,

  // Top face (0, 1, 0)
  -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
};

f32 triangle_vertices[] = {
  // position          // color
  0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // Vertex3D 1: red
 -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // Vertex3D 2: green
  0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // Vertex3D 3: blue
};

f32 axis_vertices[] = {
  // Position        // Color
   0, 0, 0,          1, 0, 0,  // Line from (0,0,0) to (1,0,0) - X axis (red)
   1, 0, 0,          1, 0, 0,

   0, 0, 0,          0, 1, 0,  // Line from (0,0,0) to (0,1,0) - Y axis (green)
   0, 1, 0,          0, 1, 0,

   0, 0, 0,          0, 0, 1,  // Line from (0,0,0) to (0,0,1) - Z axis (blue)
   0, 0, 1,          0, 0, 1,
};

inline void* grid_create(Arena* arena, i32 grid_size, f32 grid_step) {
  v3* grid = push_array(arena, v3, grid_size*4);

  i32 i = 0;
  f32 half = (grid_size / 2.0f) * grid_step;
  // horizontal from left to right
  {
    f32 z = 0;
    f32 x = grid_size * grid_step;
    Loop (j, grid_size) {
      grid[i++] = v3(0 - half, 0, z + half);
      grid[i++] = v3(x - half, 0, z + half);
      z -= grid_step;
    }
  }

  // vertical from top to down
  {
    f32 z = -grid_size * grid_step;
    f32 x = 0;
    Loop (j, grid_size) {
      grid[i++] = v3(x - half, 0, 0 + half);
      grid[i++] = v3(x - half, 0, z + half);
      x += grid_step;
    }
  }
  return grid;
}
