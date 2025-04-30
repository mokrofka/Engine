#include "loader_utils.h"

void res_unload(ResLoader* self, Res* res) {
  Assert(self && res);
  mem_free(res->data);
  res->data = 0;
  res->data = 0;
  res->loader_id = INVALID_ID;
}
