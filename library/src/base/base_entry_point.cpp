#include "render/r_frontend.h"

#include "sys/texture.h"
#include "sys/geometry.h"
#include "sys/res.h"
#include "sys/shader.h"

#include "asset_watch.h"
#include "ui.h"

void base_main_thread_entry(void (*entry)()) {
  entry();
}
