#ifndef NO_IMPORT_LIB
import com;
#endif

#define global static

#define KB(n) ((n) << 10)
#define MB(n) ((n) << 20)

#define Loop(it, c) for (u32 it = 0; it < c; ++it)

#define VS_Entry [shader("vertex")]
#define FS_Entry [shader("fragment")]
#define CS_Entry [shader("compute")]

#define VkBind(idx) [[vk::binding(idx, 0)]]

#define g st[0]
