#include "common.h"
#include "vulkan/vulkan_core.h"
#include "stdio.h"

const u32 MaxMaterials = KB(1);
const u32 MaxLights    = KB(1);
const u32 MaxLines     = KB(1);
const u32 MaxMeshes    = KB(1);
const u32 MaxShaders   = KB(1);
const u32 MaxTextures  = KB(1);
const u32 MaxDrawCalls = KB(1);

enum RenderpassType {
  RenderpassType_World,
  RenderpassType_UI,
  RenderpassType_Screen,
};

///////////////////////////////////
// Gpu memory layout

struct GpuEntity {
  alignas(16) mat4 model;
  alignas(16) v4 color;
  Handle<GpuMaterial> material;
};

struct GpuMaterial {
  alignas(16) v3 ambient;
  alignas(16) v3 diffuse;
  alignas(16) v3 specular;
  f32 shininess;
  Handle<GpuTexture> texture;
};

struct GpuPointLight {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  f32 intensity;
  f32 rad;
};

struct GpuDirLight {
  alignas(16) v3 color;
  alignas(16) v3 dir;
  f32 intensity;
};

struct GpuSpotLight {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  alignas(16) v3 dir;
  f32 intensity;
  f32 inner_cutoff;
  f32 outer_cutoff;
};

struct GpuGlobalState {
  alignas(16) mat4 projection_view;
  alignas(16) mat4 projection;
  alignas(16) mat4 view;
  alignas(16) v4 ambient_color;
  f32 time;
  u32 point_light_count;
  u32 dir_light_count;
  u32 spot_light_count;
  u32 entity_indices[MaxEntities+MaxStaticEntities];
  // GpuEntity entities[MaxEntities+MaxStaticEntities];
  // GpuMaterial materials[MaxMaterials];
  // GpuPointLight point_lights[MaxLights];
  // GpuDirLight dir_lights[MaxLights];
  // GpuSpotLight spot_lights[MaxLights];
  // u32 drawinfo[MaxEntities+MaxStaticEntities];
};

///////////////////////////////////
// Generic stuff

struct PushConstant {
  u32 drawcall_offset;
};

struct MeshBatch {
  Handle<GpuMesh> mesh_handle;
  Darray<Handle<Entity>> entities;
};

struct ShaderBatch {
  // Handle<GpuShader> shader;
  Darray<MeshBatch> mesh_batches;
  Map<u32, u32> mesh_to_batch;
};

struct RenderEntity {
  Handle<Handle<Entity>> shader_handle;
  Handle<Handle<Entity>> entity_idx;
};

///////////////////////////////////
// Vulkan

#if BUILD_DEBUG
  #define VK_CHECK(expr)          \
    {                             \
      Assert(expr == VK_SUCCESS); \
    }
  #else
    #define VK_CHECK(expr) expr
#endif

// #if BUILD_DEBUG
//   #define VK_CHECK(expr)        \
//     {                           \
//       if (expr != VK_SUCCESS)   \
//       {                         \
//         Error("%s", vk_result_string(expr)); \
//         Assert(false);          \
//       }                         \
//     }
// #else
//   #define VK_CHECK(expr) expr
// #endif

// #if BUILD_DEBUG
//   #define VK_CHECK(expr)        \
//     {                           \
//       if (expr != VK_SUCCESS)   \
//       {                         \
//       }                         \
//     }
// #else
//   #define VK_CHECK(expr) expr
// #endif

#define vkdevice vk.device.logical_device

struct VK_Buffer {
  VkBuffer handle;
  VkDeviceMemory memory;
  u8* mapped_memory;
  u64 size;
  u64 cap;
};

struct VK_ImageInfo {
  VkImageType image_type;
  u32 width;
  u32 height;
  u32 miplevels_count;
  VkImageCreateFlags flags;
  VkFormat format;
  u32 array_layers_count;
  VkSampleCountFlagBits samples;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkMemoryPropertyFlags memory_flags;
  VkImageAspectFlags aspect;
  VkImageViewType view_type;
};

struct VK_Image {
  VkImage handle;
  VkImageView view;
  VK_ImageInfo info;
  VkDeviceMemory memory;
};

struct VK_Mesh {
  u64 vert_count;
  VkDeviceSize vert_offset;
  GpuMemHandler vert_memory;
  u64 index_count;
  VkDeviceSize index_offset;
  GpuMemHandler index_memory;
};

struct VK_Swapchain {
  VkSwapchainKHR handle;
  VkSurfaceFormatKHR image_format;  
  VkPresentModeKHR present_mode;
  VK_Image images[4];
  VK_Image depth_attachment;
};

struct VK_SwapchainSupportInfo {
  VkSurfaceCapabilitiesKHR capabilities;
  u32 format_count;
  VkSurfaceFormatKHR* formats;
  u32 present_mode_count;
  VkPresentModeKHR* present_modes;
};

struct VK_Device {
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  VK_SwapchainSupportInfo swapchain_support;
  u32 graphics_queue_index;
  u32 present_queue_index;
  u32 transfer_queue_index;
  u32 compute_queue_index;
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue transfer_queue;
  VkQueue compute_queue;
  VkCommandPool cmd_pool;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  VkFormat depth_format;
};

struct VK_ShaderStages {
  VkPipelineShaderStageCreateInfo stages[2];
};

struct VK_Shader {
  VkPipeline pipeline;
  DarrayHandler<Handle<Entity>> entities;
  DarrayHandler<Handle<Entity>> entities_indexed;
  DarrayHandler<Handle<StaticEntity>> static_entities;
  DarrayHandler<Handle<StaticEntity>> static_entities_indexed;

  ShaderBatch batch;
  ShaderBatch batch_indexed;

  u32 static_entities_count;
  u32 static_entities_indexed_count;
  u32 static_entities_count_old;
  u32 static_entities_indexed_count_old;
  ShaderBatch static_batch;
  ShaderBatch static_batch_indexed;

  VkShaderModule module[2];
};

struct VK_SyncObj {
  VkSemaphore* image_available_semaphores;
  VkSemaphore* render_complete_semaphores;
  VkFence* in_flight_fences;
};

struct VK_DrawCallInfo {
  union {
    VkDrawIndirectCommand draw_command;
    VkDrawIndexedIndirectCommand index_draw_command;
  };
  union {
    u32 entity_id;
    u32 entities_offset_id;
  };
};

////////////////////////////////////////////////////////////////////////
// Darray<ShaderBatch> shader_batches;
// HashMap<ShaderID, u32> shader_lookup;
// struct EntityRenderLocation {
//     u32 shader_batch;
//     u32 mesh_batch;
//     u32 entity_index;
// };
// EntityRenderLocation entity_locations[MAX_ENTITIES];

// void add_entity(EntityID e, ShaderID shader, MeshID mesh)
// {
//     u32 shader_idx;

//     if (!shader_lookup.get(shader, &shader_idx))
//     {
//         shader_idx = shader_batches.count;
//         shader_lookup[shader] = shader_idx;

//         ShaderBatch sb = {};
//         sb.shader = shader;

//         shader_batches.add(sb);
//     }

//     ShaderBatch& sb = shader_batches[shader_idx];

//     u32 mesh_idx;

//     if (!sb.mesh_to_batch.get(mesh, &mesh_idx))
//     {
//         mesh_idx = sb.meshes.count;
//         sb.mesh_to_batch[mesh] = mesh_idx;

//         MeshBatch mb = {};
//         mb.mesh = mesh;

//         sb.meshes.add(mb);
//     }

//     MeshBatch& mb = sb.meshes[mesh_idx];

//     u32 entity_index = mb.entities.count;
//     mb.entities.add(e);

//     entity_locations[e] = {
//         shader_idx,
//         mesh_idx,
//         entity_index
//     };
// }
// void remove_entity(EntityID e)
// {
//     auto loc = entity_locations[e];

//     ShaderBatch& sb = shader_batches[loc.shader_batch];
//     MeshBatch& mb = sb.meshes[loc.mesh_batch];

//     u32 idx = loc.entity_index;
//     u32 last = mb.entities.count - 1;

//     EntityID swapped = mb.entities[last];

//     mb.entities[idx] = swapped;
//     mb.entities.pop();

//     entity_locations[swapped].entity_index = idx;
// }
// for (ShaderBatch& sb : shader_batches)
// {
//     bind_pipeline(sb.shader);

//     for (MeshBatch& mb : sb.meshes)
//     {
//         bind_mesh(mb.mesh);

//         u32 instanceCount = mb.entities.count;

//         vkCmdDrawIndexed(cmd,
//             mesh.index_count,
//             instanceCount,
//             mesh.first_index,
//             mesh.vertex_offset,
//             0);
//     }
// }

////////////////////////////////////////////////////////////////////////

struct VK_State {
  Arena arena;
  
  VkAllocationCallbacks allocator_;
  VkAllocationCallbacks* allocator;
  VkInstance instance;
  VkSurfaceKHR surface;
  VkDebugUtilsMessengerEXT debug_messenger;
  
  VK_Device device;
  VK_SyncObj sync;
  VK_Swapchain swapchain;

  u32 images_in_flight;
  u32 frames_in_flight;
  u32 current_image_idx;
  u32 current_image_idx1;
  u32 current_image_idx_old;
  u32 current_frame_idx;
  u32 width;
  u32 height;
  f32 scale;
  VkFence* fence_images_in_flight;
  
  VK_Buffer vert_buffer;
  VK_Buffer index_buffer;
  VK_Buffer stage_buffer;
  VK_Buffer indirect_draw_buffer;
  VK_Buffer storage_buffer;
  
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet descriptor_sets;

  VkCommandBuffer* cmds;
  VkSampler sampler;
  VkPipelineLayout pipeline_layout;

  GpuGlobalState* gpu_global_shader_st;
  GpuEntity* gpu_entities;
  GpuMaterial* gpu_materials;
  VK_DrawCallInfo* gpu_draw_call_infos;
  u32* gpu_entities_indexes;

  u32 gpu_materials_count;

  u32 static_draw_indexed_offset;
  u32 static_draw_offset;
  u32 static_indexed_draw_count;
  u32 static_draw_count;

  Array<RenderEntity, MaxEntities+MaxStaticEntities> entities;
  Array<Handle<GpuShader>, MaxEntities+MaxStaticEntities> entities_to_shader;
  Array<Handle<GpuMesh>, MaxEntities+MaxStaticEntities> entities_to_mesh;

  // Array<PushConstant, MaxEntities> push_constants;
  Array<VK_Shader, MaxShaders> shaders;
  Array<VK_Mesh, MaxMeshes> meshes;
  Array<VK_Image, MaxTextures> textures;

  Array<Darray<Handle<Entity>>, MaxShaders> meshes_;
  Array<Darray<Handle<GpuMesh>>, MaxShaders> shaders_;

  // SparseSet<PointLight> point_light_data;
  // SparseSet<DirLight> dir_light_data;
  // SparseSet<SpotLight> spot_light_data;

  VK_Shader screen_shader;
  VK_Shader cubemap_shader;
  VK_Shader debug_line_shader;

  // offscreen rendering
  VK_Image msaa_texture_target;
  VK_Image offscreen_depth_buffer;
  VK_Image* texture_targets;

  mat4 view;
  mat4 projection;

  struct DebugDrawLine {
    Vertex vert[2];
  };
  Darray<f32> debug_draw_line_times;
  Darray<DebugDrawLine> debug_draw_lines;

  VkCommandBuffer cmd;


  ///////////////////////////////////
  // Vulkan loader
  #define VK_DECL(name) Glue(PFN_, vk##name) name
  #define VK_FUNCTION(name) vk.name = (Glue(PFN_, vk##name))os_lib_get_proc(vk.lib, Stringify(vk##name))
  #define VK_INSTANCE_FUNCTION(name) vk.name = (Glue(PFN_, vk##name))vk.GetInstanceProcAddr(vk.instance, Stringify(vk##name))
  #define VK_DEVICE_FUNCTION(name) vk.name = (Glue(PFN_, vk##name))vk.GetDeviceProcAddr(vkdevice, Stringify(vk##name))

  OS_Handle lib;

  // Core functions
  VK_DECL(GetInstanceProcAddr);

  // Instance functions
  VK_DECL(EnumerateInstanceExtensionProperties);
  VK_DECL(EnumerateInstanceVersion);
  VK_DECL(EnumerateInstanceLayerProperties);
  VK_DECL(CreateInstance);
  VK_DECL(DestroyInstance);
  VK_DECL(EnumeratePhysicalDevices);
  VK_DECL(GetDeviceProcAddr);
  VK_DECL(GetPhysicalDeviceProperties);
  VK_DECL(GetPhysicalDeviceFeatures);
  VK_DECL(GetPhysicalDeviceMemoryProperties);
  VK_DECL(GetPhysicalDeviceQueueFamilyProperties);
  VK_DECL(GetPhysicalDeviceFormatProperties);
  VK_DECL(GetPhysicalDeviceSurfaceFormatsKHR);
  VK_DECL(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  VK_DECL(GetPhysicalDeviceSurfacePresentModesKHR);
  VK_DECL(GetPhysicalDeviceSurfaceSupportKHR);
  VK_DECL(EnumerateDeviceExtensionProperties);
  VK_DECL(CreateDevice);
  VK_DECL(DestroySurfaceKHR);

  // Device functions.
  VK_DECL(GetDeviceQueue);
  VK_DECL(DeviceWaitIdle);
  VK_DECL(CreateCommandPool);
  VK_DECL(DestroyCommandPool);
  VK_DECL(DestroyDevice);
  VK_DECL(CreateSwapchainKHR);
  VK_DECL(DestroySwapchainKHR);
  VK_DECL(GetSwapchainImagesKHR);
  VK_DECL(CreateImage);
  VK_DECL(CreateImageView);
  VK_DECL(DestroyImage);
  VK_DECL(DestroyImageView);
  VK_DECL(GetImageMemoryRequirements);
  VK_DECL(AllocateMemory);
  VK_DECL(FreeMemory);
  VK_DECL(AllocateCommandBuffers);
  VK_DECL(FreeCommandBuffers);
  VK_DECL(BeginCommandBuffer);
  VK_DECL(EndCommandBuffer);
  VK_DECL(BindImageMemory);
  VK_DECL(CreateSemaphore);
  VK_DECL(DestroySemaphore);
  VK_DECL(CreateFence);
  VK_DECL(DestroyFence);
  VK_DECL(WaitForFences);
  VK_DECL(AcquireNextImageKHR);
  VK_DECL(ResetFences);
  VK_DECL(CreateDescriptorSetLayout);
  VK_DECL(DestroyDescriptorSetLayout);
  VK_DECL(CreateDescriptorPool);
  VK_DECL(DestroyDescriptorPool);
  VK_DECL(CreateShaderModule);
  VK_DECL(DestroyShaderModule);
  VK_DECL(CreateSampler);
  VK_DECL(DestroySampler);
  VK_DECL(CreateBuffer);
  VK_DECL(DestroyBuffer);
  VK_DECL(GetBufferMemoryRequirements);
  VK_DECL(BindBufferMemory);
  VK_DECL(MapMemory);
  VK_DECL(UnmapMemory);
  VK_DECL(FlushMappedMemoryRanges);
  VK_DECL(CreatePipelineLayout);
  VK_DECL(DestroyPipelineLayout);
  VK_DECL(CreateGraphicsPipelines);
  VK_DECL(DestroyPipeline);
  VK_DECL(AllocateDescriptorSets);
  VK_DECL(FreeDescriptorSets);
  VK_DECL(UpdateDescriptorSets);

  VK_DECL(CmdBindPipeline);
  VK_DECL(CmdPipelineBarrier);
  VK_DECL(CmdBlitImage);
  VK_DECL(CmdCopyBuffer);
  VK_DECL(CmdCopyBufferToImage);
  VK_DECL(CmdCopyImageToBuffer);
  VK_DECL(CmdExecuteCommands);
  VK_DECL(CmdSetViewport);
  VK_DECL(CmdSetScissor);
  VK_DECL(CmdSetFrontFace);
  VK_DECL(CmdSetCullMode);
  VK_DECL(CmdSetStencilTestEnable);
  VK_DECL(CmdSetDepthTestEnable);
  VK_DECL(CmdSetDepthWriteEnable);
  VK_DECL(CmdSetStencilReference);
  VK_DECL(CmdSetStencilOp);
  VK_DECL(CmdBeginRendering);
  VK_DECL(CmdEndRendering);
  VK_DECL(CmdSetStencilCompareMask);
  VK_DECL(CmdSetStencilWriteMask);
  VK_DECL(CmdClearColorImage);
  VK_DECL(CmdClearDepthStencilImage);
  VK_DECL(CmdSetPrimitiveTopology);
  VK_DECL(CmdPushConstants);
  VK_DECL(CmdBindVertexBuffers);
  VK_DECL(CmdBindIndexBuffer);
  VK_DECL(CmdDraw);
  VK_DECL(CmdDrawIndexed);
  VK_DECL(CmdDrawIndirect);
  VK_DECL(CmdDrawIndexedIndirect);
  VK_DECL(CmdBindDescriptorSets);

  VK_DECL(QueueSubmit);
  VK_DECL(QueueWaitIdle);
  VK_DECL(QueuePresentKHR);
};

VK_State vk;

intern String vk_result_string(VkResult result) {
  switch (result) {
    case VK_SUCCESS:                                            return "VK_SUCCESS";
    case VK_NOT_READY:                                          return "VK_NOT_READY";
    case VK_TIMEOUT:                                            return "VK_TIMEOUT";
    case VK_EVENT_SET:                                          return "VK_EVENT_SET";
    case VK_EVENT_RESET:                                        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:                                         return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:                           return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:                         return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:                        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:                                  return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:                            return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:                            return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:                        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:                          return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:                          return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:                             return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:                         return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:                              return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:                                      return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY:                           return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:                      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:                                return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:               return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_PIPELINE_COMPILE_REQUIRED:                          return "VK_PIPELINE_COMPILE_REQUIRED";
    case VK_ERROR_SURFACE_LOST_KHR:                             return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                     return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:                                     return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:                              return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                     return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:                        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV:                            return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:                return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:       return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:    return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:       return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:        return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:          return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT:                return "VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT";
    case VK_ERROR_NOT_PERMITTED_KHR:                            return "VK_ERROR_NOT_PERMITTED_KHR";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:          return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:                                    return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:                                    return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:                             return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:                         return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:                    return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
    case VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT:               return "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT";
    case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:             return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
    case VK_PIPELINE_BINARY_MISSING_KHR:                        return "VK_PIPELINE_BINARY_MISSING_KHR";
    case VK_ERROR_NOT_ENOUGH_SPACE_KHR:                         return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
    case VK_RESULT_MAX_ENUM:                                    return "VK_RESULT_MAX_ENUM";
  }
}

#if OS_WINDOW
  typedef void* HINSTANCE;
  typedef void* HWND;
  typedef void* HANDLE;
  typedef wchar_t* LPCWSTR;
  typedef u32 DWORD;
  typedef void* LPVOID;
  typedef b32 BOOL;
  typedef void* HMONITOR;
  struct SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
  };
  #include <vulkan/vulkan_win32.h>
  intern void vk_surface_create() {
    VkWin32SurfaceCreateInfoKHR surface_create_info = {
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = (HINSTANCE)os_get_handle_info(),
      .hwnd = (HWND)os_get_window_handle()
    };
    VK_CHECK(vkCreateWin32SurfaceKHR(vk.instance, &surface_create_info, vk.allocator, &vk.surface));
    Info("Vulkan win32 surface created");
  }
  #define VK_SURFACE_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
  
#elif OS_LINUX
  #if GFX_X11
    #include <xcb/xcb.h>
    #include <vulkan/vulkan_xcb.h>

    intern void vk_surface_create() {
      struct VK_Surface {
        xcb_connection_t* connection;
        xcb_window_t window;
      } vk_surface; 
      os_get_gfx_api_handlers(&vk_surface);
      VkXcbSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = vk_surface.connection,
        .window = vk_surface.window,
      };
      PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vk.GetInstanceProcAddr(vk.instance, "vkCreateXcbSurfaceKHR");
      VK_CHECK(vkCreateXcbSurfaceKHR(vk.instance, &surfaceInfo, vk.allocator, &vk.surface));
      Info("Vulkan XCB surface created");
    }
    #define VK_SURFACE_NAME VK_KHR_XCB_SURFACE_EXTENSION_NAME

  #else
    #include <vulkan/vulkan_wayland.h>
    intern void vk_surface_create() {
      struct VK_Surface{
        struct wl_display* wl_display;
        struct wl_surface* wl_surface;
      } vk_surface;
      os_get_gfx_api_handlers(&vk_surface);
      VkWaylandSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = vk_surface.wl_display,
        .surface = vk_surface.wl_surface,
      };
      PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)vk.GetInstanceProcAddr(vk.instance, "vkCreateWaylandSurfaceKHR");
      VK_CHECK(vkCreateWaylandSurfaceKHR (vk.instance, &surfaceInfo, vk.allocator, &vk.surface));
      Info("Vulkan wayland surface created");
    }
    #define VK_SURFACE_NAME VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
  #endif
#endif

#define VulkanUseAllocator 0

#if VulkanUseAllocator
  VkAllocationCallbacks vk_allocator_create() {
    VkAllocationCallbacks callbacks = {
      .pUserData = null,
      .pfnAllocation = [](void* user_data, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope){
        void* result = mem_alloc(vk.alloc, size);
        return result;
      },
      .pfnReallocation = [](void* user_data, void* origin, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope) {
        ...
      },
      .pfnFree = [](void* user_data, void* memory){
        if (!memory) { // NOTE: it happens
          return;
        }
        mem_free(memory);
      },
      .pfnInternalAllocation = [](void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope){},
      .pfnInternalFree = [](void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope){},
    };
    return callbacks;
  }
#endif

////////////////////////////////////////////////////////////////////////
// @Utils

// intern VkSemaphore vk_get_current_image_available_semaphore() { return vk.sync.image_available_semaphores[vk.current_image_idx]; }
intern VkSemaphore vk_get_current_image_available_semaphore() { return vk.sync.image_available_semaphores[vk.current_frame_idx]; }
intern VkSemaphore vk_get_current_render_complete_semaphore() { return vk.sync.render_complete_semaphores[vk.current_image_idx]; }
intern VkCommandBuffer vk_get_current_cmd()                   { return vk.cmds[vk.current_frame_idx]; }

intern u32 vk_find_memory_idx(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties = vk.device.memory;
  u32 idx = -1;
  Loop (i, memory_properties.memoryTypeCount) {
    if (BitHas(type_filter, i) && FlagHas(memory_properties.memoryTypes[i].propertyFlags, property_flags)) {
      idx = i;
      break;
    }
  }
  AssertMsg(idx != -1, "Unable to find suitable memory type");
  return idx;
}

////////////////////////////////////////////////////////////////////////
// @Cmd

intern VkCommandBuffer vk_cmd_alloc(VkCommandPool pool) {
  VkCommandBufferAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };
  VkCommandBuffer cmd;
  VK_CHECK(vk.AllocateCommandBuffers(vkdevice, &allocate_info, &cmd));
  return cmd;
}

intern void vk_cmd_free(VkCommandPool pool, VkCommandBuffer cmd) {
  vk.FreeCommandBuffers(vkdevice, pool, 1, &cmd);
}

intern void vk_cmd_begin(VkCommandBuffer cmd) {
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  VK_CHECK(vk.BeginCommandBuffer(cmd, &begin_info));
}

intern void vk_cmd_end(VkCommandBuffer cmd) {
  VK_CHECK(vk.EndCommandBuffer(cmd));
}

intern void vk_cmd_submit(VkCommandBuffer cmd) {
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  VK_CHECK(vk.QueueSubmit(vk.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vk.QueueWaitIdle(vk.device.graphics_queue));
}

intern void vk_cmd_end_submit(VkCommandBuffer cmd) {
  vk_cmd_end(cmd);
  vk_cmd_submit(cmd);
}

intern VkCommandBuffer vk_cmd_alloc_begin() {
  VkCommandBuffer result = vk_cmd_alloc(vk.device.cmd_pool);
  vk_cmd_begin(result);
  return result;
}

intern void vk_cmd_end_free(VkCommandBuffer cmd) {
  vk_cmd_end(cmd);
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  VK_CHECK(vk.QueueSubmit(vk.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vk.QueueWaitIdle(vk.device.graphics_queue));
  vk_cmd_free(vk.device.cmd_pool, cmd);
}

////////////////////////////////////////////////////////////////////////
// @Buffer

intern VK_Buffer vk_buffer_create(u64 size, u32 usage, u32 memory_property_flags) {
  VkBufferCreateInfo buffer_create_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  VK_Buffer buffer = {
    .cap = size,
  };
  VK_CHECK(vk.CreateBuffer(vkdevice, &buffer_create_info, vk.allocator, &buffer.handle));
  VkMemoryRequirements requirements;
  vk.GetBufferMemoryRequirements(vkdevice, buffer.handle, &requirements);
  u32 memory_index = vk_find_memory_idx(requirements.memoryTypeBits, memory_property_flags);
  VkMemoryAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = requirements.size,
    .memoryTypeIndex = memory_index,
  };
  VK_CHECK(vk.AllocateMemory(vkdevice, &allocate_info, vk.allocator, &buffer.memory));
  VK_CHECK(vk.BindBufferMemory(vkdevice, buffer.handle, buffer.memory, 0));
  return buffer;
}

intern void vk_buffer_destroy(VK_Buffer buffer) {
  vk.FreeMemory(vkdevice, buffer.memory, vk.allocator);
  vk.DestroyBuffer(vkdevice, buffer.handle, vk.allocator);
}

intern void vk_buffer_map_memory(VK_Buffer& buffer, u64 offset, u64 size) {
  VK_CHECK(vk.MapMemory(vkdevice, buffer.memory, offset, size, 0, (void**)&buffer.mapped_memory));
}

intern void vk_buffer_unmap_memory(VK_Buffer buffer) {
  vk.UnmapMemory(vkdevice, buffer.memory);
}

intern void vk_buffer_upload_to_gpu(VK_Buffer buffer, Range range, void* data) {
  MemCopy(vk.stage_buffer.mapped_memory, data, range.size);
  vk_cmd_begin(vk.cmd);
  VkBufferCopy copy_region = {
    .srcOffset = 0,
    .dstOffset = range.offset,
    .size = range.size,
  };
  vk.CmdCopyBuffer(vk.cmd, vk.stage_buffer.handle, buffer.handle, 1, &copy_region);
  vk_cmd_end_submit(vk.cmd);
}

// intern void vk_buffer_descriptor_update(VK_SubBuffer buf, u32 binding) {
//   Range range = buf.range;
//   VkDescriptorBufferInfo buffer_info = {
//     .buffer = vk.storage_buffer.handle,
//     .offset = range.offset,
//     .range = range.size,
//   };
//   VkWriteDescriptorSet ubo_descriptor = {
//     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//     .dstSet = vk.descriptor_sets,
//     .dstBinding = binding,
//     .dstArrayElement = 0,
//     .descriptorCount = 1,
//     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//     .pBufferInfo = &buffer_info,
//   };
//   VkWriteDescriptorSet descriptors[] = {ubo_descriptor};
//   vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
// }

// intern void vk_buffer_alloc(VK_SubBuffer* buf, u32 binding, u64 size, u64 align) {
//   GpuMemHandler handler = vk.gpu_alloc.alloc(size, align);
//   buf->mem = handler;
//   buf->range = {vk.gpu_alloc.get(handler), size};
// }

////////////////////////////////////////////////////////////////////////
// @Pipeline

intern void vk_shader_pipeline_init() {
  VkPushConstantRange push_constant = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .offset = 0,
    .size = sizeof(PushConstant),
  };
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &vk.descriptor_set_layout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &push_constant,
  };
  VK_CHECK(vk.CreatePipelineLayout(vkdevice, &pipeline_layout_info, vk.allocator, &vk.pipeline_layout));
}

intern VkPipeline vk_shader_pipeline_create(Shader shader) {

  ///////////////////////////////////
  // Shader modules
  VkPipelineShaderStageCreateInfo stages[2] = {};
  {
    Scratch scratch;
    Loop (i, 2) {
      String stage_type_strs[] = {"vert", "frag"};
      VkShaderStageFlagBits stage_types[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
      String filepath = push_strf(scratch, "%s/shaders/compiled/%s.%s.spv", asset_base_path(), shader.name, stage_type_strs[i]);
      Buffer binary = os_file_path_read_all(scratch, filepath);
      VkShaderModuleCreateInfo module_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = binary.size,
        .pCode = (u32*)binary.data,
      };
      VkShaderModule handle;
      VK_CHECK(vk.CreateShaderModule(vkdevice, &module_info, vk.allocator, &handle));
      stages[i] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage_types[i],
        .module = handle,
        .pName = "main",
      };
    }
  }

  // Dynamic rendering
  // VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
  VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = vk.device.depth_format,
  };
  
  // Vertex input
  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  // Attributes
  VkVertexInputAttributeDescription attribute_desriptions[] = {
    [0] = {
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, pos),
    },
    [1] = {
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, norm),
    },
    [2] = {
      .location = 2,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, uv),
    },
    [3] = {
      .location = 3,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = (u32)OffsetOf(Vertex, color),
    },
  };
  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = ArrayCount(attribute_desriptions),
    .pVertexAttributeDescriptions = attribute_desriptions,
  };

  // Input assembly
  VkPrimitiveTopology topology;
  switch (shader.primitive) {
    case ShaderTopology_Triangle: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
    case ShaderTopology_Line:     topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
    case ShaderTopology_Point:    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
  }
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = topology,
    .primitiveRestartEnable = VK_FALSE,
  };
  
  // Viewport
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)vk.height,
    .width = (f32)vk.width,
    .height = -(f32)vk.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
    .offset = { .x = 0, .y = 0 },
    .extent = {
      .width = vk.width,
      .height = vk.height,
    },
  };
  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor,
  };
  
  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    // .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp = 0.0f,
    .depthBiasSlopeFactor = 0.0f,
    .lineWidth = 1.0f,
  };
  
  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    // .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .rasterizationSamples = (VkSampleCountFlagBits)shader.samples,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f,
    .pSampleMask = 0,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE,
  };

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  if (shader.use_depth) {
    depth_stencil_state_info.depthTestEnable = VK_TRUE;
    if (shader.type == ShaderType_Cube) {
      depth_stencil_state_info.depthWriteEnable = VK_FALSE;
      depth_stencil_state_info.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    } else {
      depth_stencil_state_info.depthWriteEnable = VK_TRUE;
      depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
    }
    depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_info.stencilTestEnable = VK_FALSE;
  }
  
  // Blending
  VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
  if (shader.is_transparent) {
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;

    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  } else {
    color_blend_attachment_state.blendEnable = VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;

    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  }
  color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  VkPipelineColorBlendStateCreateInfo color_blend_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment_state,
  };
   
  // Dynamic state
  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo dynamic_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = ArrayCount(dynamic_states),
    .pDynamicStates = dynamic_states,
  };
  
  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = 2,
    .pStages = stages,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pTessellationState = null,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterizer_state,
    .pMultisampleState = &multisampling_state_info,
    .pDepthStencilState = &depth_stencil_state_info,
    .pColorBlendState = &color_blend_state_info,
    .pDynamicState = &dynamic_state_info,
    .layout = vk.pipeline_layout,
    .renderPass = null,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1,
  };
  VkPipeline result;
  VK_CHECK(vk.CreateGraphicsPipelines(vkdevice, VK_NULL_HANDLE, 1, &pipeline_info, vk.allocator, &result));
  return result;
}

intern VK_ShaderStages vk_shader_module_create(String name) {
  Scratch scratch;
  VK_ShaderStages result = {};
  Loop (i, 2) {
    String stage_type_strs[] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    String filepath = push_strf(scratch, "%s/shaders/compiled/%s.%s.spv", asset_base_path(), name, stage_type_strs[i]);
    Buffer binary = os_file_path_read_all(scratch, filepath);
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = binary.size,
      .pCode = (u32*)binary.data,
    };
    VkShaderModule handle;
    VK_CHECK(vk.CreateShaderModule(vkdevice, &module_info, vk.allocator, &handle));
    result.stages[i] = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = stage_types[i],
      .module = handle,
      .pName = "main",
    };
  }
  return result;
}

Handle<GpuShader> vk_shader_load(Shader shader) {
  Scratch scratch;
  switch (shader.type) {
    case ShaderType_Drawing: {
      // VK_ShaderStages stages = vk_shader_module_create(shader.name);
      VK_Shader vk_shader = {};
      // vk_shader.module[0] = stages.stages[0].module;
      // vk_shader.module[1] = stages.stages[1].module;
      vk_shader.pipeline = vk_shader_pipeline_create(shader);
      u32 id = vk.shaders.count;
      vk.shaders.add(vk_shader);
      Handle<GpuShader> handle = {id};
      return handle;
    }
    case ShaderType_Screen: {
      // VK_Shader &shader = vk.screen_shader;
      // shader = {
      //   .stages = vk_shader_module_create(name),
      // };
      // shader.pipeline = vk_shader_pipeline_create(type, shader.stages);
      // u32 id = vk.shaders.count + 100;
      // append(vk.shaders, shader);
      // return id;
      return {};
    }
    case ShaderType_Cube: {
      VK_Shader &vk_shader = vk.cubemap_shader;
      // VK_ShaderStages stages = vk_shader_module_create(shader.name);
      vk_shader.pipeline = vk_shader_pipeline_create(shader);
      Handle<GpuShader> handle = {200};
      return handle;
    }
    case ShaderType_Compute: {
      return {};
    } break;
  }
}

void vk_shader_reload(Shader shader, Handle<GpuShader> shader_handle) {
  switch (shader.type) {
    case ShaderType_Drawing: {
      VK_Shader& vk_shader = vk.shaders[shader_handle.handle];
      vk.DeviceWaitIdle(vkdevice);
      vk.DestroyPipeline(vkdevice, vk_shader.pipeline, vk.allocator);
      Loop (i, 2) {
        vk.DestroyShaderModule(vkdevice, vk_shader.module[i], vk.allocator);
      }
      // VK_ShaderStages stages = vk_shader_module_create(name);
      // shader.module[0] = stages.stages[0].module;
      // shader.module[1] = stages.stages[1].module;
      vk_shader.pipeline = vk_shader_pipeline_create(shader);
    }
    case ShaderType_Screen: {
      // VK_Shader& shader = vk.screen_shader;
      // vk.DeviceWaitIdle(vkdevice);
      // vk.DestroyPipeline(vkdevice, shader.pipeline, vk.allocator);
      // Loop (i, 2) {
      //   vk.DestroyShaderModule(vkdevice, shader.stages[i].module, vk.allocator);
      // }
      // shader.stages = vk_shader_module_create(name);
      // shader.pipeline = vk_shader_pipeline_create(shader.type, shader.stages);
    }
    case ShaderType_Cube: {
      // VK_Shader& shader = vk.cubemap_shader;
      // vk.DeviceWaitIdle(vkdevice);
      // vk.DestroyPipeline(vkdevice, shader.pipeline, vk.allocator);
      // Loop (i, 2) {
      //   vk.DestroyShaderModule(vkdevice, shader.stages[i].module, vk.allocator);
      // }
      // shader.stages = vk_shader_module_create(name);
      // shader.pipeline = vk_shader_pipeline_create(shader.type, shader.stages);
    }
    case ShaderType_Compute: {
    }
  }
}

intern void vk_descriptor_init() {
  // Pool
  #define MaxStorageBuffer 7
  #define MaxSets 1
  #define MaxSamplers 1
  #define MaxCubeTextures 1
  {
    VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MaxStorageBuffer},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxTextures},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxCubeTextures},
      {VK_DESCRIPTOR_TYPE_SAMPLER, MaxSamplers},
    };
    VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
      .maxSets = MaxSets,
      .poolSizeCount = ArrayCount(pool_sizes),
      .pPoolSizes = pool_sizes,
    };
    vk.CreateDescriptorPool(vkdevice, &pool_info, vk.allocator, &vk.descriptor_pool);
  }

  // Setlayout
  {
    // Bindless extension
    VkDescriptorBindingFlags flags[] = {
      0,
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
      .pNext = null,
      .bindingCount = ArrayCount(flags),
      .pBindingFlags = flags,
    };
    VkDescriptorSetLayoutBinding layout_bindings[] = {
      // Global state
      {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Textures
      {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = MaxTextures,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Samplers
      {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = MaxSamplers,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Cube textures
      {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = MaxCubeTextures,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // DrawCall info
      {
        .binding = 4,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Entities
      {
        .binding = 5,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Materials
      {
        .binding = 6,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Point lights
      {
        .binding = 7,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Dir lights
      {
        .binding = 8,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // Spot lights
      {
        .binding = 9,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      },
    };
    VkDescriptorSetLayoutCreateInfo layout_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = &binding_flags,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
      .bindingCount = ArrayCount(layout_bindings),
      .pBindings = layout_bindings,
    };
    VK_CHECK(vk.CreateDescriptorSetLayout(vkdevice, &layout_info, vk.allocator, &vk.descriptor_set_layout));
  }

  // Descriptor set
  {
    VkDescriptorSetAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = vk.descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &vk.descriptor_set_layout,
    };
    VK_CHECK(vk.AllocateDescriptorSets(vkdevice, &alloc_info, &vk.descriptor_sets));
  }

  // Descriptor
  {
    OffsetPusher pusher = {};
    offset_push_struct(pusher, GpuGlobalState);
    u64 entities_offset = (u64)offset_push_array(pusher, GpuEntity, MaxEntities+MaxStaticEntities);
    u64 materials_offset = (u64)offset_push_array(pusher, GpuMaterial, MaxMaterials);
    // Global
    VkDescriptorBufferInfo global_state_buffer_info = {
      .buffer = vk.storage_buffer.handle,
      .offset = 0,
      .range = sizeof(GpuGlobalState),
    };
    VkWriteDescriptorSet global_state_descriptor_write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &global_state_buffer_info,
    };
    // Entity
    VkDescriptorBufferInfo entity_buffer_info = {
      .buffer = vk.storage_buffer.handle,
      .offset = entities_offset,
      .range = sizeof(GpuEntity) * (MaxEntities + MaxStaticEntities),
    };
    VkWriteDescriptorSet entity_descriptor_write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 5,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &entity_buffer_info,
    };
    // Material
    VkDescriptorBufferInfo material_buffer_info = {
      .buffer = vk.storage_buffer.handle,
      .offset = materials_offset,
      .range = sizeof(GpuMaterial) * MaxMaterials,
    };
    VkWriteDescriptorSet material_descriptor_write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 6,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &material_buffer_info,
    };
    // Indirect draw
    VkDescriptorBufferInfo indirect_draw_buffer_info = {
      .buffer = vk.indirect_draw_buffer.handle,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
    };
    VkWriteDescriptorSet indirect_draw_write_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 4,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &indirect_draw_buffer_info,
    };
    VkWriteDescriptorSet descriptors[] = {global_state_descriptor_write, indirect_draw_write_descriptor, entity_descriptor_write, material_descriptor_write};
    vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  }

  // Set pointers to shader data
  {
    OffsetMemPusher pusher = {.offset = vk.storage_buffer.mapped_memory};
    vk.gpu_global_shader_st = offset_push_struct(pusher, GpuGlobalState);
    vk.gpu_entities_indexes = vk.gpu_global_shader_st->entity_indices;
    vk.gpu_entities = offset_push_array(pusher, GpuEntity, MaxEntities+MaxStaticEntities);
    vk.gpu_materials = offset_push_array(pusher, GpuMaterial, MaxMaterials);

    vk.gpu_draw_call_infos = (VK_DrawCallInfo*)vk.indirect_draw_buffer.mapped_memory;
  }
  
}

////////////////////////////////////////////////////////////////////////
// @Image

intern VK_ImageInfo vk_image_info_default(u32 width, u32 height) {
  VK_ImageInfo result = {
    .image_type = VK_IMAGE_TYPE_2D,
    .width = width,
    .height = height,
    .miplevels_count = 1,
    .flags = 0,
    .format = VK_FORMAT_R8G8B8A8_UNORM,
    .array_layers_count = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
    .view_type = VK_IMAGE_VIEW_TYPE_2D,
  };
  return result;
};

intern VkImageView vk_image_view_create(VkImage image, VK_ImageInfo info) {
  VkImageViewCreateInfo view_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = image,
    .viewType = info.view_type,
    .format = info.format,
    .subresourceRange = {
      .aspectMask = info.aspect,
      .baseMipLevel = 0,
      .levelCount = info.miplevels_count,
      .baseArrayLayer = 0,
      .layerCount = info.array_layers_count,
    },
  };
  VkImageView result;
  VK_CHECK(vk.CreateImageView(vkdevice, &view_create_info, vk.allocator, &result));
  return result;
}

intern VK_Image vk_image_create(VK_ImageInfo info) {
  VkImageCreateInfo image_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags = info.flags,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = info.format,
    .extent = {info.width, info.height, 1},
    .mipLevels = info.miplevels_count,
    .arrayLayers = info.array_layers_count,
    .samples = info.samples,
    .tiling = info.tiling,
    .usage = info.usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  VkImage handle;
  VK_CHECK(vk.CreateImage(vkdevice, &image_create_info, vk.allocator, &handle));
  VkMemoryRequirements memory_requirements;
  vk.GetImageMemoryRequirements(vkdevice, handle, &memory_requirements);
  u32 memory_type = vk_find_memory_idx(memory_requirements.memoryTypeBits, info.memory_flags);
  VkMemoryAllocateInfo memory_allocate_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memory_requirements.size,
    .memoryTypeIndex = memory_type,
  };
  VkDeviceMemory memory;
  VK_CHECK(vk.AllocateMemory(vkdevice, &memory_allocate_info, vk.allocator, &memory));
  VK_CHECK(vk.BindImageMemory(vkdevice, handle, memory, 0));
  VkImageView view = vk_image_view_create(handle, info);
  VK_Image result = {
    .handle = handle,
    .memory = memory,
    .view = view,
    .info = info,
  };
  return result;
}

intern void vk_image_destroy(VK_Image image) {
  if (image.view) {
    vk.DestroyImageView(vkdevice, image.view, vk.allocator);
  }
  if (image.memory) {
    vk.FreeMemory(vkdevice, image.memory, vk.allocator);
  }
  if (image.handle) {
    vk.DestroyImage(vkdevice, image.handle, vk.allocator);
  }
}

intern VkAccessFlags vk_image_layout_to_mem_access_flags(VkImageLayout layout) {
	switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:                return NoFlags;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:     return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:     return VK_ACCESS_TRANSFER_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:          return NoFlags;
    default: Assert(false); return 0;
	}
}

intern VkPipelineStageFlags vk_image_layout_to_pipeline_stage_flags(VkImageLayout layout) {
	switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:                return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:     return VK_PIPELINE_STAGE_TRANSFER_BIT; 
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:          return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    default: Assert(false); return 0;
	}
}

intern void vk_image_layout_transition(VkCommandBuffer cmd, VK_Image image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT) {
  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcAccessMask = vk_image_layout_to_mem_access_flags(old_layout),
    .dstAccessMask = vk_image_layout_to_mem_access_flags(new_layout),
    .oldLayout = old_layout,
    .newLayout = new_layout,
    .srcQueueFamilyIndex = vk.device.graphics_queue_index,
    .image = image.handle,
    .subresourceRange = {
      .aspectMask = aspect_mask,
      .baseMipLevel = 0,
      .levelCount = image.info.miplevels_count,
      .baseArrayLayer = 0,
      .layerCount = image.info.array_layers_count,
    },
  };
  VkPipelineStageFlags src_stage = vk_image_layout_to_pipeline_stage_flags(old_layout);
  VkPipelineStageFlags dst_stage = vk_image_layout_to_pipeline_stage_flags(new_layout);
  vk.CmdPipelineBarrier(cmd, src_stage, dst_stage, NoFlags, 0, null, 0, null, 1, &barrier);
}

intern void vk_image_upload_to_gpu(VkCommandBuffer cmd, VK_Image image) {
  VkBufferImageCopy region = {
    .bufferOffset = 0,
    .bufferRowLength = 0,
    .bufferImageHeight = 0,
    .imageSubresource = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel = 0,
      .baseArrayLayer = 0,
      .layerCount = image.info.array_layers_count,
    },
    .imageExtent = { image.info.width, image.info.height, 1 },
  };
  vk.CmdCopyBufferToImage(cmd, vk.stage_buffer.handle, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

intern void vk_texture_init() {
  // Common texture sampler
  VkSamplerCreateInfo sampler_info = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_LINEAR,
    .minFilter = VK_FILTER_LINEAR,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .mipLodBias = 0.0f,
    .anisotropyEnable = VK_TRUE,
    .maxAnisotropy = 8,
    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_ALWAYS,
    .minLod = 0.0f,
    .maxLod = VK_LOD_CLAMP_NONE,
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE,
  };
  VK_CHECK(vk.CreateSampler(vkdevice, &sampler_info, vk.allocator, &vk.sampler));
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = vk.sampler,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = vk.descriptor_sets,
    .dstBinding = 2,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
    .pImageInfo = &descriptor_image_info,
  };
  VkWriteDescriptorSet descriptors[] = {texture_descriptor};
  vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
};

intern void vk_texture_generate_mipmaps(VK_Image image) {
  VkCommandBuffer cmd = vk.cmd;
  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcQueueFamilyIndex = vk.device.graphics_queue_index,
    .image = image.handle,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .levelCount = 1,
      .layerCount = 1,
    },
  };
  
  i32 width = image.info.width;
  i32 height = image.info.height;
  for (i32 i = 1; i < image.info.miplevels_count; ++i) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vk.CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, null, 0, null, 1, &barrier);

    VkImageBlit blit = {
      .srcOffsets = { {0, 0, 0}, {width, height, 1} },
      .srcSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = (u32)i - 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
      .dstOffsets = { {0, 0, 0}, {width > 1 ? width / 2 : 1, height > 1 ? height / 2 : 1, 1} },
      .dstSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = (u32)i,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };
    vk.CmdBlitImage(cmd, image.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vk.CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, null, 0, null, 1, &barrier);

    if (width > 1) width /= 2;
    if (height > 1) height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = image.info.miplevels_count - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vk.CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, null, 0, null, 1, &barrier);
}

Handle<GpuTexture> vk_texture_load(Texture texture_info) {
  u64 size = texture_info.width * texture_info.height * 4;
  MemCopy(vk.stage_buffer.mapped_memory, texture_info.data, size);
  VK_ImageInfo image_info = vk_image_info_default(texture_info.width, texture_info.height);
  image_info.miplevels_count = Floor(Log2(Max(image_info.width, image_info.height))) + 1;
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VK_Image image = vk_image_create(image_info);
  {
    VkCommandBuffer cmd = vk.cmd;
    vk_cmd_begin(cmd);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk_image_upload_to_gpu(cmd, image);
    vk_texture_generate_mipmaps(image);
    vk_cmd_end_submit(cmd);
  }
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = vk.sampler,
    .imageView = image.view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_write_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = vk.descriptor_sets,
    .dstBinding = 1,
    .dstArrayElement = vk.textures.count,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    .pImageInfo = &descriptor_image_info,
  };
  VkWriteDescriptorSet descriptors[] = {texture_write_descriptor};
  vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  u32 id = vk.textures.count;
  vk.textures.add(image);
  Handle<GpuTexture> handle = {id};
  return handle;
}

intern void vk_texture_resize_target() {
  VK_CHECK(vk.DeviceWaitIdle(vkdevice));
  Debug("texture target resized: x = %u y = %u", vk.width, vk.height);
  u32 width = vk.width * vk.scale;
  u32 height = vk.height * vk.scale;
  if (width == 0) width = 1;
  if (height == 0) height = 1;

  // Msaa
  vk_image_destroy(vk.msaa_texture_target);
  VK_ImageInfo image_info = vk_image_info_default(width, height);
  image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
  image_info.samples = VK_SAMPLE_COUNT_4_BIT;
  vk.msaa_texture_target = vk_image_create(image_info);

  // Texture target
  Loop (i, vk.images_in_flight) {
    vk_image_destroy(vk.texture_targets[i]);
    VK_ImageInfo image_info = vk_image_info_default(width, height);
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    vk.texture_targets[i] = vk_image_create(image_info);

    VkDescriptorImageInfo descriptor_image_info = {
      .sampler = vk.sampler,
      .imageView = vk.texture_targets[i].view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet texture_write_descriptor = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk.descriptor_sets,
      .dstBinding = 1,
      .dstArrayElement = (u32)i,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
      .pImageInfo = &descriptor_image_info,
    };
    VkWriteDescriptorSet descriptors[] = {texture_write_descriptor};
    vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  }

  // Offscreen depth buffer
  vk_image_destroy(vk.offscreen_depth_buffer);
  VK_ImageInfo depth_info = vk_image_info_default(width, height);
  depth_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depth_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  depth_info.format = vk.device.depth_format;
  depth_info.samples = VK_SAMPLE_COUNT_4_BIT;
  vk.offscreen_depth_buffer = vk_image_create(depth_info);
}

Handle<GpuMaterial> vk_material_load(Material material) {
  vk.gpu_materials[vk.gpu_materials_count] = {
    .ambient = material.ambient,
    .diffuse = material.diffuse,
    .specular = material.specular,
    .shininess = material.shininess, 
    .texture = material.texture, 
  };
  u32 id = vk.gpu_materials_count++;
  Handle<GpuMaterial> handle = {id};
  return handle;
}

Handle<GpuCubemap> vk_cubemap_load(Texture* textures) {
  u32 width = textures->width;
  u32 height = textures->height;
  u64 size = width * height * 4;
  Loop (i, 6) {
    MemCopy(Offset(vk.stage_buffer.mapped_memory, size * i), textures[i].data, size);
  }
  VK_ImageInfo image_info = vk_image_info_default(width, height);
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  image_info.array_layers_count = 6;
  image_info.view_type = VK_IMAGE_VIEW_TYPE_CUBE;
  VK_Image image = vk_image_create(image_info);
  {
    VkCommandBuffer cmd = vk.cmd;
    vk_cmd_begin(cmd);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk_image_upload_to_gpu(cmd, image);
    vk_image_layout_transition(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vk_cmd_end_submit(cmd);
  }
  VkDescriptorImageInfo descriptor_image_info = {
    .sampler = vk.sampler,
    .imageView = image.view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet texture_descriptor = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = vk.descriptor_sets,
    .dstBinding = 3,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    .pImageInfo = &descriptor_image_info,
  };
  VkWriteDescriptorSet descriptors[] = {texture_descriptor};
  vk.UpdateDescriptorSets(vkdevice, ArrayCount(descriptors), descriptors, 0, null);
  return {};
}

////////////////////////////////////////////////////////////////////////
// @Device

struct VK_DevicePhysicalRequirements {
  b8 graphics; 
  b8 present;
  b8 transfer;
  b8 compute;
  b8 depth_attachment;
  StringArray device_extension_names;
};

struct VK_DevicePhysicalQueueFamilyInfo {
  u32 graphics_family_index;
  u32 present_family_index;
  u32 compute_family_index;
  u32 transfer_family_index;
};

intern VK_SwapchainSupportInfo vk_device_query_swapchain_support(VkPhysicalDevice physical_device) {
  VK_SwapchainSupportInfo support_info = {};
  // Surface capabilities
  VK_CHECK(vk.GetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vk.surface, &support_info.capabilities));
  // Surface formats
  VK_CHECK(vk.GetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk.surface, &support_info.format_count, null));
  support_info.formats = push_array(vk.arena, VkSurfaceFormatKHR, support_info.format_count);
  VK_CHECK(vk.GetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk.surface, &support_info.format_count, support_info.formats));
  // Present modes
  VK_CHECK(vk.GetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk.surface, &support_info.present_mode_count, null));
  support_info.present_modes = push_array(vk.arena, VkPresentModeKHR, support_info.present_mode_count);
  VK_CHECK(vk.GetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk.surface, &support_info.present_mode_count, support_info.present_modes));
  return support_info;
}

intern VkFormat vk_device_detect_depth_format(VkPhysicalDevice physical_device) {
  VkFormat candidates[] = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT
  };
  u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  for (VkFormat x : candidates) {
    VkFormatProperties properties;
    vk.GetPhysicalDeviceFormatProperties(physical_device, x, &properties);
    if (FlagHas(properties.optimalTilingFeatures, flags)) {
      return x;
    }
  }
  Assert(!"Failed to find a supported format!");
  return VK_FORMAT_UNDEFINED;
}

intern VK_DevicePhysicalQueueFamilyInfo vk_device_physical_meets_requirements(
  VkPhysicalDevice device,
  VkPhysicalDeviceProperties properties,
  VkPhysicalDeviceFeatures features,
  VK_DevicePhysicalRequirements requirements,
  VK_SwapchainSupportInfo* out_swapchain_support)
{
  Scratch scratch;
  VK_DevicePhysicalQueueFamilyInfo queue_info = {
    .graphics_family_index = (u32)-1,
    .present_family_index = (u32)-1,
    .compute_family_index = (u32)-1,
    .transfer_family_index = (u32)-1,
  };
  u32 queue_family_count = 0;
  vk.GetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, null);
  VkQueueFamilyProperties* queue_families = push_array(scratch, VkQueueFamilyProperties, queue_family_count);
  vk.GetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
  
  // Look at each queue and see what queues it supports
  Info("Graphics | Present | Computer | Transfer | Name");
  Loop (i, queue_family_count) {
    // Graphics queue?
    if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_info.graphics_family_index == -1)) {
      queue_info.graphics_family_index = i;
    }
    // Transfer queue?
    if ((queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && (queue_info.transfer_family_index == -1) &&
       !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
    {
      queue_info.transfer_family_index = i;
    }
    // Compute queue?
    if ((queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && (queue_info.compute_family_index == -1) &&
       !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
    {
      queue_info.compute_family_index = i;
    }
    // Present queue?
    VkBool32 supports_present;
    VK_CHECK(vk.GetPhysicalDeviceSurfaceSupportKHR(device, i, vk.surface, &supports_present));
    if (supports_present && queue_info.present_family_index == -1) {
      queue_info.present_family_index = i;
    }
    if (queue_info.graphics_family_index != -1 &&
        queue_info.transfer_family_index != -1 &&
        queue_info.compute_family_index != -1 &&
        queue_info.present_family_index != -1) 
    {
      break;
    }
  }

  if (queue_info.transfer_family_index == -1) queue_info.transfer_family_index = queue_info.graphics_family_index;
  if (queue_info.compute_family_index == -1) queue_info.compute_family_index = queue_info.graphics_family_index;

  Info("       %i |       %i |        %i |        %i | %s",
    queue_info.graphics_family_index != -1,
    queue_info.present_family_index != -1,
    queue_info.compute_family_index != -1,
    queue_info.transfer_family_index != -1,
    String(properties.deviceName));

  var missing = [](b32 required, u32 index) {
    return required && index == -1;
  };
  if (missing(requirements.graphics, queue_info.graphics_family_index) ||
      missing(requirements.present, queue_info.present_family_index) ||
      missing(requirements.compute, queue_info.compute_family_index) ||
      missing(requirements.compute, queue_info.transfer_family_index)) 
  {
    AssertMsg(false, "Device doesn't have needed queues");
  }

  Info("Device meets queue requirements");
  Trace("Grahics Family Index: %i", queue_info.graphics_family_index);
  Trace("Present Family Index: %i", queue_info.present_family_index);
  Trace("Transfer Family Index: %i", queue_info.transfer_family_index);
  Trace("Compute Family Index: %i", queue_info.compute_family_index);

  *out_swapchain_support = vk_device_query_swapchain_support(device);
  Assert((out_swapchain_support->format_count >= 1) && (out_swapchain_support->present_mode_count >= 1) && "Required swapchain support not present");

  // Device extensions.
  if (requirements.device_extension_names.count) {
    u32 available_extension_count = 0;
    VkExtensionProperties* available_extensions = 0;
    VK_CHECK(vk.EnumerateDeviceExtensionProperties(device, null, &available_extension_count, null));
    if (available_extension_count != 0) {
      available_extensions = push_array(scratch, VkExtensionProperties, available_extension_count);
      VK_CHECK(vk.EnumerateDeviceExtensionProperties(device, null, &available_extension_count, available_extensions));
    }
    Loop(i, requirements.device_extension_names.count) {
      b32 found = false;
      Loop(j, available_extension_count) {
        if (str_match(requirements.device_extension_names.v[i], available_extensions[j].extensionName)) {
          found = true;
          break;
        }
      }
      Assert(found);
    }
  }

  return queue_info;
}

intern VK_Device vk_device_select_physical() {
  Scratch scratch;
  u32 physical_device_count;
  VK_CHECK(vk.EnumeratePhysicalDevices(vk.instance, &physical_device_count, null));
  VK_Device* devices = push_array(scratch, VK_Device, physical_device_count);
  VkPhysicalDevice* physical_devices = push_array(scratch, VkPhysicalDevice, physical_device_count);
  VK_CHECK(vk.EnumeratePhysicalDevices(vk.instance, &physical_device_count, physical_devices));

  i32 discrete_gpu_idx = -1;
  i32 fallback_gpu_idx = -1;

  Loop (i, physical_device_count) {
    VkPhysicalDeviceProperties properties;
    vk.GetPhysicalDeviceProperties(physical_devices[i], &properties);
    VkPhysicalDeviceFeatures features;
    vk.GetPhysicalDeviceFeatures(physical_devices[i], &features);
    VkPhysicalDeviceMemoryProperties memory;
    vk.GetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);
    
    String extentions[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    VK_DevicePhysicalRequirements requirements = {
      .graphics = true,
      .present = true,
      .transfer = true,
      .compute = true,
      .depth_attachment = true,
      .device_extension_names = {extentions, ArrayCount(extentions)},
    };

    VK_DevicePhysicalQueueFamilyInfo queue_info = vk_device_physical_meets_requirements(
      physical_devices[i],
      properties,
      features,
      requirements,
      &devices[i].swapchain_support
    );

    Info("Available device: '%s'", String(properties.deviceName));
    // GPU type, etc.
    switch (properties.deviceType) {
      case VK_PHYSICAL_DEVICE_TYPE_OTHER: {
        Info("GPU type is Unkown");
        fallback_gpu_idx = i;
      } break;
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
        Info("GPU type is Integrated");
        fallback_gpu_idx = i;
      } break;
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
        discrete_gpu_idx = i;
      } Info("GPU type is Descrete");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
        Info("GPU type is Virtual");
        fallback_gpu_idx = i;
      } break;
      case VK_PHYSICAL_DEVICE_TYPE_CPU: {
        Info("GPU type is CPU");
        fallback_gpu_idx = i;
      } break;
      default:;
    }

    Info("GPU Driver version: %i.%i.%i",
      VK_VERSION_MAJOR(properties.driverVersion),
      VK_VERSION_MINOR(properties.driverVersion),
      VK_VERSION_PATCH(properties.driverVersion));
    Info("GPU API version: %i.%i.%i",
      VK_VERSION_MAJOR(properties.apiVersion),
      VK_VERSION_MINOR(properties.apiVersion),
      VK_VERSION_PATCH(properties.apiVersion));

    Loop (j, memory.memoryHeapCount) {
      f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / GB(1));
      if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        Info("Local GPU memory: %.2f GiB", memory_size_gib);
      else
        Info("Shared System memory: %.2f GiB", memory_size_gib);
    }

    devices[i].physical_device = physical_devices[i];
    devices[i].graphics_queue_index = queue_info.graphics_family_index;
    devices[i].present_queue_index = queue_info.present_family_index;
    devices[i].transfer_queue_index = queue_info.transfer_family_index;
    devices[i].compute_queue_index = queue_info.compute_family_index;

    devices[i].properties = properties;
    devices[i].features = features;
    devices[i].memory = memory;
    devices[i].depth_format = vk_device_detect_depth_format(physical_devices[i]);
  }
  i32 selected_index;
  if (discrete_gpu_idx != -1) {
    selected_index = discrete_gpu_idx; 
    Info("Discrete GPU was choosen");
  } else {
    selected_index = fallback_gpu_idx;
    Info("Integrated GPU was choosen");
  }
  Info("Physical device selected");
  return devices[selected_index];
}

intern void vk_device_create() {
  vk.device = vk_device_select_physical();

  // Features
  VkPhysicalDeviceShaderDrawParametersFeatures drawFeatures = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
    .shaderDrawParameters = VK_TRUE,
  };
  VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
    .pNext = &drawFeatures,
    .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,     // allows runtime indexing
    .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,  // allows update image descriptors
    .descriptorBindingPartiallyBound = VK_TRUE,               // allows not updated descriptors
    .runtimeDescriptorArray = VK_TRUE,                        // allows not specified size of descriptor array in shader
  };
  VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
    .pNext = &indexing_features,
    .dynamicRendering = true
  };
  
  const u32 queue_count = 4;
  Array<u32, queue_count> indices = {};
  indices.add(vk.device.graphics_queue_index);
  if (!indices.exists(vk.device.present_queue_index)) {
    indices.add(vk.device.present_queue_index);
  }
  if (!indices.exists(vk.device.transfer_queue_index)) {
    indices.add(vk.device.transfer_queue_index);
  }
  if (!indices.exists(vk.device.compute_queue_index)) {
    indices.add(vk.device.compute_queue_index);
  }

  Array<VkDeviceQueueCreateInfo, queue_count> queue_create_infos = {};
  for (u32 i : indices) {
    f32 queue_priority = 1.0f;
    VkDeviceQueueCreateInfo device_queue_create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = null,
      .flags = 0,
      .queueFamilyIndex = indices[i],
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
    };
    queue_create_infos.add(device_queue_create_info);
  }
  
  const char* extension_names[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
  VkPhysicalDeviceFeatures device_features = {
    .multiDrawIndirect = true,
    .fillModeNonSolid = true,  // Request anistrophy
    .samplerAnisotropy = true, // Request wireframe
  };
  VkDeviceCreateInfo device_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = &dynamic_rendering_features,
    .queueCreateInfoCount = queue_create_infos.count,
    .pQueueCreateInfos = queue_create_infos.data,
    .enabledExtensionCount = ArrayCount(extension_names),
    .ppEnabledExtensionNames = extension_names,
    .pEnabledFeatures = &device_features,
  };
  VK_CHECK(vk.CreateDevice(vk.device.physical_device, &device_create_info, vk.allocator, &vk.device.logical_device));
  Info("Logical device created");
}

////////////////////////////////////////////////////////////////////////
// @Swapchain

intern void vk_swapchain_create(b32 reuse) {
  VK_CHECK(vk.GetPhysicalDeviceSurfaceCapabilitiesKHR(vk.device.physical_device, vk.surface, &vk.device.swapchain_support.capabilities));
  VkExtent2D swapchain_extent = {vk.width, vk.height};
  if (vk.device.swapchain_support.capabilities.currentExtent.width != U32_MAX) {
    swapchain_extent = vk.device.swapchain_support.capabilities.currentExtent;
  }
  VkExtent2D min = vk.device.swapchain_support.capabilities.minImageExtent;
  VkExtent2D max = vk.device.swapchain_support.capabilities.maxImageExtent;
  swapchain_extent.width = Clamp(min.width, swapchain_extent.width, max.width);
  swapchain_extent.height = Clamp(min.height, swapchain_extent.height, max.height);
  VkSwapchainCreateInfoKHR swapchain_create_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = vk.surface,
    .minImageCount = vk.images_in_flight,
    .imageFormat = vk.swapchain.image_format.format,
    .imageColorSpace = vk.swapchain.image_format.colorSpace,
    .imageExtent = swapchain_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = vk.device.swapchain_support.capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = vk.swapchain.present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = reuse ? vk.swapchain.handle : null,
  };
  VK_CHECK(vk.CreateSwapchainKHR(vkdevice, &swapchain_create_info, vk.allocator, &vk.swapchain.handle));
  u32 image_count = vk.images_in_flight;

  VkImage images[4];
  VK_CHECK(vk.GetSwapchainImagesKHR(vkdevice, vk.swapchain.handle, &image_count, images));
  Loop (i, image_count) {
    vk.swapchain.images[i] = {
      .handle = images[i],
      .info = {
        .miplevels_count = 1,
        .array_layers_count = 1,
      }
    };
    VkImageViewCreateInfo view_create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = vk.swapchain.images[i].handle,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = vk.swapchain.image_format.format,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };
    VK_CHECK(vk.CreateImageView(vkdevice, &view_create_info, vk.allocator, &vk.swapchain.images[i].view));
  }
  VK_ImageInfo depth_info = vk_image_info_default(swapchain_extent.width, swapchain_extent.height);
  depth_info.format = vk.device.depth_format;
  depth_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depth_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  depth_info.samples = VK_SAMPLE_COUNT_4_BIT;
  vk.swapchain.depth_attachment = vk_image_create(depth_info);
}

intern void vk_swapchain_query() {
  // vk.device.swapchain_support = vk_device_query_swapchain_support(vk.device.physical_device);
  // vk.device.depth_format = vk_device_detect_depth_format();
  vk.current_frame_idx = 0;
  // Choose a swap surface format
  b32 found = false;
  Loop (i, vk.device.swapchain_support.format_count) {
    VkSurfaceFormatKHR format = vk.device.swapchain_support.formats[i];
    // Preferred formats
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM && // darker
    // if (format.format == VK_FORMAT_B8G8R8A8_SRGB && // brighter TODO: use
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
    {
      vk.swapchain.image_format = format;
      found = true;
      break;
    }
  }
  if (!found) {
    vk.swapchain.image_format = vk.device.swapchain_support.formats[0];
  }
  // Choose present mode
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  Loop (i, vk.device.swapchain_support.present_mode_count) {
    VkPresentModeKHR mode = vk.device.swapchain_support.present_modes[i];
    if (mode == VK_PRESENT_MODE_FIFO_KHR) {
      present_mode = mode;
      break;
    }
  }
  vk.swapchain.present_mode = present_mode;
  // VkExtent2D swapchain_extent = {vk.width, vk.height};
  // if (vk.device.swapchain_support.capabilities.currentExtent.width != U32_MAX) {
    // swapchain_extent = vk.device.swapchain_support.capabilities.currentExtent;
  // }
  // VkExtent2D min = vk.device.swapchain_support.capabilities.minImageExtent;
  // VkExtent2D max = vk.device.swapchain_support.capabilities.maxImageExtent;
  // swapchain_extent.width = Clamp(min.width, swapchain_extent.width, max.width);
  // swapchain_extent.height = Clamp(min.height, swapchain_extent.height, max.height);
}

intern void vk_swapchain_destroy(VK_Swapchain swapchain) {
  VK_CHECK(vk.DeviceWaitIdle(vkdevice));
  vk_image_destroy(swapchain.depth_attachment);
  Loop (i, vk.images_in_flight) {
    vk.DestroyImageView(vkdevice, swapchain.images[i].view, vk.allocator);
  }
  vk.DestroySwapchainKHR(vkdevice, swapchain.handle, vk.allocator);
}

intern void vk_swapchain_recreate() {
  VK_Swapchain old = vk.swapchain;
  vk_swapchain_create(true);
  vk_swapchain_destroy(old);
  Info("Swapchain recreated x: %i y: %i", vk.width, vk.height);
}

intern u32 vk_swapchain_acquire_next_image_index(VkSemaphore image_available_semaphore) {
  u32 image_index;
  // VkResult result = vk.AcquireNextImageKHR(vkdevice, vk.swapchain.handle, U64_MAX, image_available_semaphore, null, &image_index);

  // if (result == VK_ERROR_OUT_OF_DATE_KHR) {
  //   Assert(false);
  // }

  // if (result == VK_SUBOPTIMAL_KHR) {
  //   VK_CHECK(vk.DeviceWaitIdle(vkdevice));
  //   vk_swapchain_recreate();

  //   VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  //   vk.DestroySemaphore(vkdevice, vk.sync.image_available_semaphores[0], vk.allocator);
  //   vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.image_available_semaphores[0]);
  //   vk.DestroySemaphore(vkdevice, vk.sync.image_available_semaphores[vk.current_image_idx1], vk.allocator);
  //   vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.image_available_semaphores[vk.current_image_idx1]);

  //   result = vk.AcquireNextImageKHR(vkdevice, vk.swapchain.handle, U64_MAX, vk.sync.image_available_semaphores[0], null, &image_index);

  //   Info("current image idx: %i", image_index);
  //   Info("current old image idx: %i", vk.current_image_idx_old);
  // }
  // if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
  //   Assert(false);
  // }

  VK_CHECK(vk.AcquireNextImageKHR(vkdevice, vk.swapchain.handle, U64_MAX, image_available_semaphore, null, &image_index));
  return image_index;
}

intern void vk_swapchain_present(VkSemaphore render_complete_semaphore, u32 present_image_index) {
  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &render_complete_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &vk.swapchain.handle,
    .pImageIndices = &present_image_index,
  };
  VK_CHECK(vk.QueuePresentKHR(vk.device.graphics_queue, &present_info));
}

////////////////////////////////////////////////////////////////////////
// @Mesh

Handle<GpuMesh> vk_mesh_load(Mesh mesh) {
  VK_Buffer& vert_buff = vk.vert_buffer;
  u64 vert_size = mesh.vert_count*sizeof(Vertex);
  u64 vert_offset = vert_buff.size;
  vert_buff.size += vert_size;
  Range vert_range = { .offset = vert_offset, .size = vert_size };

  VK_Buffer& index_buff = vk.index_buffer;
  u64 index_size = mesh.index_count*sizeof(u32);
  u64 index_offset = index_buff.size;
  index_buff.size += index_size;
  Range index_range = { .offset = index_offset, .size = index_size };

  VK_Mesh vk_mesh = {
    .vert_count = mesh.vert_count,
    .vert_offset = vert_range.offset,
    .index_count = mesh.index_count,
    .index_offset = index_range.offset,
  };

  vk_buffer_upload_to_gpu(vk.vert_buffer, vert_range, mesh.vertices);
  if (mesh.indexes) {
    vk_buffer_upload_to_gpu(vk.index_buffer, index_range, mesh.indexes);
  }

  u32 id = vk.meshes.count;
  vk.meshes.add(vk_mesh);
  Handle<GpuMesh> handle = {id};
  return handle;
}

////////////////////////////////////////////////////////////////////////
// @Drawing

void vk_draw() {
  GpuGlobalState& shader_st = *vk.gpu_global_shader_st;
  shader_st.projection_view = vk.projection * vk.view;
  shader_st.projection = vk.projection;
  shader_st.view = vk.view;

  VkCommandBuffer cmd = vk_get_current_cmd();
  vk.CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.pipeline_layout, 0, 1, &vk.descriptor_sets, 0, null);

  VkDeviceSize size = 0;
  vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, &size);
  vk.CmdBindIndexBuffer(cmd, vk.index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
  u32 draw_call_count = 0;
  u32 draw_call_mem_offset = 0;
  u32 entities_draw_count = 0;
  u32 entities_draw_id_offset = 0;

  for (VK_Shader& shader : vk.shaders) {
    vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline);

    // Indexed Entities per shader
    u32 per_shader_indexed_draw_count = 0;
    for (MeshBatch mesh_batch : shader.batch_indexed.mesh_batches) {
      if (mesh_batch.entities.count == 0) continue;
      for (Handle<Entity> entity_handle : mesh_batch.entities) {
        u32 entity_idx = entity_handle.idx();
        vk.gpu_entities[entity_idx].model = mat4_transform(entities_transforms(entity_handle));
        vk.gpu_entities_indexes[entities_draw_count++] = entity_idx;
      }
      u32 mesh_idx = mesh_batch.mesh_handle.handle;
      VK_Mesh mesh = vk.meshes[mesh_idx];
      VK_DrawCallInfo info = {
        .index_draw_command = {
          .indexCount = (u32)mesh.index_count,
          .instanceCount = mesh_batch.entities.count,
          .firstIndex = (u32)(mesh.index_offset/sizeof(u32)),
          .vertexOffset = (i32)(mesh.vert_offset/sizeof(Vertex)),
          .firstInstance = 0,
        },
        .entities_offset_id = entities_draw_id_offset,
      };
      vk.gpu_draw_call_infos[draw_call_count++] = info;
      entities_draw_id_offset += mesh_batch.entities.count;
      ++per_shader_indexed_draw_count;
    }
    if (per_shader_indexed_draw_count > 0) {
      PushConstant push = {.drawcall_offset = draw_call_mem_offset / (u32)sizeof(VK_DrawCallInfo)};
      vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
      vk.CmdDrawIndexedIndirect(cmd, vk.indirect_draw_buffer.handle, draw_call_mem_offset, per_shader_indexed_draw_count, sizeof(VK_DrawCallInfo));
    }

    // Entities per shader
    u32 per_shader_draw_count = 0;
    for (MeshBatch mesh_batch : shader.batch.mesh_batches) {
      if (mesh_batch.entities.count == 0) continue;
      for (Handle<Entity> entity_handle : mesh_batch.entities) {
        u32 entity_idx = entity_handle.idx();
        vk.gpu_entities[entity_idx].model = mat4_transform(entities_transforms(entity_handle));
        vk.gpu_entities_indexes[entities_draw_count++] = entity_idx;
      }
      u32 mesh_idx = mesh_batch.mesh_handle.handle;
      VK_Mesh mesh = vk.meshes[mesh_idx];
      VK_DrawCallInfo info = {
        .draw_command = {
          .vertexCount = (u32)mesh.vert_count,
          .instanceCount = mesh_batch.entities.count,
          .firstVertex = (u32)(mesh.vert_offset/sizeof(Vertex)),
          .firstInstance = 0,
        },
        .entities_offset_id = entities_draw_id_offset,
      };
      vk.gpu_draw_call_infos[draw_call_count++] = info;
      entities_draw_id_offset += mesh_batch.entities.count;
      ++per_shader_draw_count;
    }
    if (per_shader_draw_count > 0) {
      u32 draw_call_indexed_mem_offset = draw_call_mem_offset + (u32)sizeof(VK_DrawCallInfo)*per_shader_indexed_draw_count;
      PushConstant push = {.drawcall_offset = draw_call_indexed_mem_offset / (u32)sizeof(VK_DrawCallInfo)};
      vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
      vk.CmdDrawIndirect(cmd, vk.indirect_draw_buffer.handle, draw_call_indexed_mem_offset , per_shader_draw_count, sizeof(VK_DrawCallInfo));
    }
    draw_call_mem_offset = draw_call_count * sizeof(VK_DrawCallInfo);

    /////////////////////////////////
    // Rebuilding static buffer
    if ((shader.static_entities_count_old != shader.static_entities_count) || (shader.static_entities_indexed_count_old != shader.static_entities_indexed_count)) {
      shader.static_entities_indexed_count_old = shader.static_entities_indexed_count;
      shader.static_entities_count_old = shader.static_entities_count;

      u32 static_draw_call_count = 0;
      u32 static_entities_draw_count = 0;
      u32 static_entities_draw_id_offset = 0;

      // Indexed Entities per shader
      u32 per_shader_static_indexed_draw_count = 0;
      for (MeshBatch mesh_batch : shader.static_batch_indexed.mesh_batches) {
        if (mesh_batch.entities.count == 0) continue;
        for (Handle<Entity> entity_handle : mesh_batch.entities) {
          u32 entity_idx = entity_handle.idx();
          vk.gpu_entities[MaxEntities+entity_idx].model = mat4_transform(static_entities_transforms((Handle<StaticEntity>)entity_handle.handle));
          vk.gpu_entities_indexes[MaxEntities+static_entities_draw_count++] = MaxEntities+entity_idx;
        }
        u32 mesh_idx = mesh_batch.mesh_handle.handle;
        VK_Mesh mesh = vk.meshes[mesh_idx];
        VK_DrawCallInfo info = {
          .index_draw_command = {
            .indexCount = (u32)mesh.index_count,
            .instanceCount = mesh_batch.entities.count,
            .firstIndex = (u32)(mesh.index_offset/sizeof(u32)),
            .vertexOffset = (i32)(mesh.vert_offset/sizeof(Vertex)),
            .firstInstance = 0,
          },
          .entities_offset_id = MaxEntities+static_entities_draw_id_offset,
        };
        vk.gpu_draw_call_infos[MaxDrawCalls+static_draw_call_count++] = info;
        static_entities_draw_id_offset += mesh_batch.entities.count;
        ++per_shader_static_indexed_draw_count;
      }
      vk.static_draw_indexed_offset = MaxDrawCalls*sizeof(VK_DrawCallInfo);
      vk.static_draw_offset = vk.static_draw_indexed_offset + per_shader_static_indexed_draw_count*sizeof(VK_DrawCallInfo);

      // Entities per shader
      u32 per_shader_static_draw_count = 0;
      for (MeshBatch mesh_batch : shader.static_batch.mesh_batches) {
        if (mesh_batch.entities.count == 0) continue;
        for (Handle<Entity> entity_handle : mesh_batch.entities) {
          u32 entity_idx = entity_handle.idx();
          vk.gpu_entities[MaxEntities+entity_idx].model = mat4_transform(static_entities_transforms((Handle<StaticEntity>)entity_handle.handle));
          vk.gpu_entities_indexes[MaxEntities+static_draw_call_count++] = MaxEntities+entity_idx;
        }
        u32 mesh_idx = mesh_batch.mesh_handle.handle;
        VK_Mesh mesh = vk.meshes[mesh_idx];
        VK_DrawCallInfo info = {
          .draw_command = {
            .vertexCount = (u32)mesh.vert_count,
            .instanceCount = mesh_batch.entities.count,
            .firstVertex = (u32)(mesh.vert_offset/sizeof(Vertex)),
            .firstInstance = 0,
          },
          .entities_offset_id = MaxEntities+static_entities_draw_id_offset,
        };
        vk.gpu_draw_call_infos[MaxDrawCalls+static_draw_call_count++] = info;
        static_entities_draw_id_offset += mesh_batch.entities.count;
        ++per_shader_static_draw_count;
      }

      vk.static_indexed_draw_count = per_shader_static_indexed_draw_count;
      vk.static_draw_count = per_shader_static_draw_count;
    }
    if (shader.static_entities_indexed_count) {
      PushConstant push = {MaxDrawCalls};
      vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
      vk.CmdDrawIndexedIndirect(cmd, vk.indirect_draw_buffer.handle, vk.static_draw_indexed_offset, vk.static_indexed_draw_count, sizeof(VK_DrawCallInfo));
    }
    if (shader.static_entities_count) {
      PushConstant push = {MaxDrawCalls+vk.static_indexed_draw_count};
      vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
      vk.CmdDrawIndirect(cmd, vk.indirect_draw_buffer.handle, vk.static_draw_offset, vk.static_draw_count, sizeof(VK_DrawCallInfo));
    }
  }

  // Debug drawing
  // if (vk.draw_lines.count > 0) {
  //   vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.debug_line_shader.pipeline);
  //   vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, (VkDeviceSize*)&vk.draw_lines_offset);
  //   vk.CmdDraw(cmd, vk.draw_lines.count*2, 1, 0, 0);
  // }
  // if (vk.immediate_draw_lines.count > 0) {
  //   vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.debug_line_shader.pipeline);
  //   vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, (VkDeviceSize*)&vk.immediate_draw_lines_offset);
  //   vk.CmdDraw(cmd, vk.immediate_draw_lines.count*2, 1, 0, 0);
  //   vk.immediate_draw_lines.clear();
  // }

  // Cube map
  vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.cubemap_shader.pipeline);
  Handle<GpuMesh> h = mesh_get(Mesh_Cube);
  VK_Mesh mesh = vk.meshes[h.handle];
  vk.CmdBindVertexBuffers(cmd, 0, 1, &vk.vert_buffer.handle, &mesh.vert_offset);
  if (mesh.index_count) {
    vk.CmdBindIndexBuffer(cmd, vk.index_buffer.handle, mesh.index_offset, VK_INDEX_TYPE_UINT32);
    vk.CmdDrawIndexed(cmd, mesh.index_count, 1, 0, 0, 0);
  } else {
    vk.CmdDraw(cmd, mesh.vert_count, 1, 0, 0);
  }
}

void vk_draw_screen() {
  VkCommandBuffer cmd = vk_get_current_cmd();
  PushConstant push = {vk.current_image_idx};
  vk.CmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
  vk.CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.screen_shader.pipeline);
  vk.CmdDraw(cmd, 3, 1, 0, 0);
}

// void vk_draw_compute() {
//   // vk_compute_descriptor_update();
//   VkCommandBuffer cmd = vk.compute_cmds[vk.current_frame];

//   VkCommandBufferBeginInfo beginInfo = {};
//   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//   VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
  
//   // UniformBufferObject* ubo; Assign(ubo, vk.compute_uniform_buffer.maped_memory);
//   // ubo->projection_view = *vk.projection_view;
//   // ubo->delta_time = delta_time;
  
//   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.handle);
//   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vk.compute_shader.pipeline.pipeline_layout, 0, 1, &vk.compute_descriptor_sets[vk.current_frame], 0, null);

//   vkCmdDispatch(cmd, ParticleCount / 256, 1, 1);
  
//   VK_CHECK(vkEndCommandBuffer(cmd));
// }

// void vk_compute_descriptor_update() {
//   i32 i = vk.current_frame;
//   VkWriteDescriptorSet descriptor_writes[2];

//   VkDescriptorBufferInfo storage_buffer_info_last_frame = {
//     .buffer = vk.compute_storage_buffers[i].handle,
//     .offset = 0,
//     .range = sizeof(Particle) * ParticleCount,
//   };
//   descriptor_writes[0] = {
//     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//     .dstSet = vk.compute_descriptor_sets[i],
//     .dstBinding = 1,
//     .dstArrayElement = 0,
//     .descriptorCount = 1,
//     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//     .pBufferInfo = &storage_buffer_info_last_frame,
//   };

//   VkDescriptorBufferInfo storage_buffer_info_current_frame = {
//     .buffer = vk.compute_storage_buffers[(i + 1) % FramesInFlight].handle,
//     .offset = 0,
//     .range = sizeof(Particle) * ParticleCount,
//   };
//   descriptor_writes[1] = {
//     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//     .dstSet = vk.compute_descriptor_sets[i],
//     .dstBinding = 2,
//     .dstArrayElement = 0,
//     .descriptorCount = 1,
//     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//     .pBufferInfo = &storage_buffer_info_current_frame,
//   };

//   vkUpdateDescriptorSets(vkdevice, ArrayCount(descriptor_writes), descriptor_writes, 0, null);
// }

void vk_loader_load_core() {
  vk.lib = os_lib_open("libvulkan.so");
  VK_FUNCTION(GetInstanceProcAddr);
  VK_FUNCTION(EnumerateInstanceVersion);
  VK_FUNCTION(EnumerateInstanceExtensionProperties);
  VK_FUNCTION(EnumerateInstanceLayerProperties);
  VK_FUNCTION(CreateInstance);
}

void vk_loader_load_instance() {
  VK_INSTANCE_FUNCTION(GetDeviceProcAddr);
  VK_INSTANCE_FUNCTION(DestroyInstance);
  VK_INSTANCE_FUNCTION(EnumeratePhysicalDevices);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceProperties);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceFeatures);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceMemoryProperties);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceQueueFamilyProperties);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceFormatProperties);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceSurfaceFormatsKHR);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceSurfacePresentModesKHR);
  VK_INSTANCE_FUNCTION(GetPhysicalDeviceSurfaceSupportKHR);
  VK_INSTANCE_FUNCTION(EnumerateDeviceExtensionProperties);
  VK_INSTANCE_FUNCTION(CreateDevice);
  VK_INSTANCE_FUNCTION(DestroySurfaceKHR);
}

void vk_loader_load_device() {
  VK_DEVICE_FUNCTION(GetDeviceQueue);
  VK_DEVICE_FUNCTION(DeviceWaitIdle);
  VK_DEVICE_FUNCTION(CreateCommandPool);
  VK_DEVICE_FUNCTION(DestroyCommandPool);
  VK_DEVICE_FUNCTION(DestroyDevice);
  VK_DEVICE_FUNCTION(CreateSwapchainKHR);
  VK_DEVICE_FUNCTION(DestroySwapchainKHR);
  VK_DEVICE_FUNCTION(GetSwapchainImagesKHR);
  VK_DEVICE_FUNCTION(CreateImage);
  VK_DEVICE_FUNCTION(CreateImageView);
  VK_DEVICE_FUNCTION(DestroyImage);
  VK_DEVICE_FUNCTION(DestroyImageView);
  VK_DEVICE_FUNCTION(GetImageMemoryRequirements);
  VK_DEVICE_FUNCTION(AllocateMemory);
  VK_DEVICE_FUNCTION(FreeMemory);
  VK_DEVICE_FUNCTION(AllocateCommandBuffers);
  VK_DEVICE_FUNCTION(FreeCommandBuffers);
  VK_DEVICE_FUNCTION(BeginCommandBuffer);
  VK_DEVICE_FUNCTION(EndCommandBuffer);
  VK_DEVICE_FUNCTION(BindImageMemory);
  VK_DEVICE_FUNCTION(CreateSemaphore);
  VK_DEVICE_FUNCTION(DestroySemaphore);
  VK_DEVICE_FUNCTION(CreateFence);
  VK_DEVICE_FUNCTION(DestroyFence);
  VK_DEVICE_FUNCTION(WaitForFences);
  VK_DEVICE_FUNCTION(AcquireNextImageKHR);
  VK_DEVICE_FUNCTION(ResetFences);
  VK_DEVICE_FUNCTION(CreateDescriptorSetLayout);
  VK_DEVICE_FUNCTION(DestroyDescriptorSetLayout);
  VK_DEVICE_FUNCTION(CreateDescriptorPool);
  VK_DEVICE_FUNCTION(DestroyDescriptorPool);
  VK_DEVICE_FUNCTION(CreateShaderModule);
  VK_DEVICE_FUNCTION(DestroyShaderModule);
  VK_DEVICE_FUNCTION(CreateSampler);
  VK_DEVICE_FUNCTION(DestroySampler);
  VK_DEVICE_FUNCTION(CreateBuffer);
  VK_DEVICE_FUNCTION(DestroyBuffer);
  VK_DEVICE_FUNCTION(GetBufferMemoryRequirements);
  VK_DEVICE_FUNCTION(BindBufferMemory);
  VK_DEVICE_FUNCTION(MapMemory);
  VK_DEVICE_FUNCTION(UnmapMemory);
  VK_DEVICE_FUNCTION(FlushMappedMemoryRanges);
  VK_DEVICE_FUNCTION(CreatePipelineLayout);
  VK_DEVICE_FUNCTION(DestroyPipelineLayout);
  VK_DEVICE_FUNCTION(CreateGraphicsPipelines);
  VK_DEVICE_FUNCTION(DestroyPipeline);
  VK_DEVICE_FUNCTION(CmdBindPipeline);
  VK_DEVICE_FUNCTION(AllocateDescriptorSets);
  VK_DEVICE_FUNCTION(FreeDescriptorSets);
  VK_DEVICE_FUNCTION(UpdateDescriptorSets);

  VK_DEVICE_FUNCTION(CmdPipelineBarrier);
  VK_DEVICE_FUNCTION(CmdBlitImage);
  VK_DEVICE_FUNCTION(CmdCopyBuffer);
  VK_DEVICE_FUNCTION(CmdCopyBufferToImage);
  VK_DEVICE_FUNCTION(CmdCopyImageToBuffer);
  VK_DEVICE_FUNCTION(CmdExecuteCommands);
  VK_DEVICE_FUNCTION(CmdSetViewport);
  VK_DEVICE_FUNCTION(CmdSetScissor);
  VK_DEVICE_FUNCTION(CmdSetFrontFace);
  VK_DEVICE_FUNCTION(CmdSetCullMode);
  VK_DEVICE_FUNCTION(CmdSetStencilTestEnable);
  VK_DEVICE_FUNCTION(CmdSetDepthTestEnable);
  VK_DEVICE_FUNCTION(CmdSetDepthWriteEnable);
  VK_DEVICE_FUNCTION(CmdSetStencilReference);
  VK_DEVICE_FUNCTION(CmdSetStencilOp);
  VK_DEVICE_FUNCTION(CmdBeginRendering);
  VK_DEVICE_FUNCTION(CmdEndRendering);
  VK_DEVICE_FUNCTION(CmdSetStencilCompareMask);
  VK_DEVICE_FUNCTION(CmdSetStencilWriteMask);
  VK_DEVICE_FUNCTION(CmdClearColorImage);
  VK_DEVICE_FUNCTION(CmdClearDepthStencilImage);
  VK_DEVICE_FUNCTION(CmdSetPrimitiveTopology);
  VK_DEVICE_FUNCTION(CmdPushConstants);
  VK_DEVICE_FUNCTION(CmdBindVertexBuffers);
  VK_DEVICE_FUNCTION(CmdBindIndexBuffer);
  VK_DEVICE_FUNCTION(CmdDraw);
  VK_DEVICE_FUNCTION(CmdDrawIndexed);
  VK_DEVICE_FUNCTION(CmdDrawIndirect);
  VK_DEVICE_FUNCTION(CmdDrawIndexedIndirect);
  VK_DEVICE_FUNCTION(CmdBindDescriptorSets);

  VK_DEVICE_FUNCTION(QueueSubmit);
  VK_DEVICE_FUNCTION(QueueWaitIdle);
  VK_DEVICE_FUNCTION(QueuePresentKHR);
}

intern void vk_instance_create() {
  Scratch scratch;
  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_4
  };
  
  Array<const char*, 1> required_validation_layer_names = {};
  Array<const char*, 3> required_extensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_SURFACE_NAME };

#if BUILD_DEBUG

  // Validation layer
  required_validation_layer_names.add("VK_LAYER_KHRONOS_validation");
  Debug("%Required layers:");
  for (const char* x : required_validation_layer_names) {
    Debug(x);
  }
  u32 available_layer_count = 0;
  VK_CHECK(vk.EnumerateInstanceLayerProperties(&available_layer_count, null));
  VkLayerProperties* available_layers = push_array(scratch, VkLayerProperties, available_layer_count);
  VK_CHECK(vk.EnumerateInstanceLayerProperties(&available_layer_count, available_layers));
  Loop (i, required_validation_layer_names.count) {
    b32 found = false;
    Loop (j, available_layer_count) {
      if (str_match(required_validation_layer_names[i], available_layers[j].layerName)) {
        found = true;
        Info("Validation layer %s found", String(required_validation_layer_names[i]));
        break;
      }
    }
    AssertMsg(found, "Required validation layer is missing: %s", String(required_validation_layer_names[i]));
  }

  // Extensions
  required_extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  Debug("Required extensions:");
  for (const char* x : required_extensions) {
    Debug(x);
  }
  u32 extension_count = 0;
  vk.EnumerateInstanceExtensionProperties(null, &extension_count, null);
  VkExtensionProperties* props = push_array(scratch, VkExtensionProperties, extension_count);
  vk.EnumerateInstanceExtensionProperties(null, &extension_count, props);
  Loop (i, required_extensions.count) {
    b32 found = false;
    Loop (j, extension_count) {
      if (str_match(required_extensions[i], props[j].extensionName)) {
        found = true;
        Info("extension %s found", String(required_extensions[i]));
        break;
      }
    }
    AssertMsg(found, "Required extension is missing: %s", String(required_extensions[i]));
  }
#endif

  VkInstanceCreateInfo instance_create_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledLayerCount = (u32)required_validation_layer_names.count,
    .ppEnabledLayerNames = required_validation_layer_names.data,
    .enabledExtensionCount = (u32)required_extensions.count,
    .ppEnabledExtensionNames = required_extensions.data,
  };
  
  VK_CHECK(vk.CreateInstance(&instance_create_info, vk.allocator, &vk.instance));
  Info("Vulkan insance created");

#if BUILD_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
    .pfnUserCallback = [](
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_types,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data) -> VkBool32 
    {
      switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
          // Error(String(callback_data->pMessage))
          // NOTE: For some reason Scratch arena is invalid here?
          ErrorArena({}, "%s", String(callback_data->pMessage));
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
          Warn(String(callback_data->pMessage));
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
          Info(String(callback_data->pMessage));
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
          Trace(String(callback_data->pMessage));
        } break;
        default: break;
      }
      return false;
    },
  };

  PFN_vkCreateDebugUtilsMessengerEXT func = null;
  Assign(func, vk.GetInstanceProcAddr(vk.instance, "vkCreateDebugUtilsMessengerEXT"));
  AssertMsg(func, "Failed to create debug messenger");

  VK_CHECK(func(vk.instance, &debug_create_info, vk.allocator, &vk.debug_messenger));
  Debug("Vulkan debugger created");
#endif
}

void vk_init() {
  Scratch scratch;
  vk.arena = arena_init();
  
#if VulkanUseAllocator
  vk._allocator = vk_allocator_create();
  vk.allocator = &vk._allocator;
#endif

  vk_loader_load_core();
  vk_instance_create();
  vk_loader_load_instance();
  vk_surface_create();
  vk_device_create();
  vk_loader_load_device();

  {
    vk.GetDeviceQueue(vkdevice, vk.device.graphics_queue_index, 0, &vk.device.graphics_queue);
    vk.GetDeviceQueue(vkdevice, vk.device.present_queue_index, 0, &vk.device.present_queue);
    vk.GetDeviceQueue(vkdevice, vk.device.transfer_queue_index, 0, &vk.device.transfer_queue);
    vk.GetDeviceQueue(vkdevice, vk.device.compute_queue_index, 0, &vk.device.compute_queue);
    Info("Queues obtained");
    
    VkCommandPoolCreateInfo graphics_pool_create_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = vk.device.graphics_queue_index,
    };
    VK_CHECK(vk.CreateCommandPool(vkdevice, &graphics_pool_create_info, vk.allocator, &vk.device.cmd_pool));
    Info("Graphics command pool created");

    v2i win_size = os_get_window_size();
    vk.width = win_size.x;
    vk.height = win_size.y;

    vk.images_in_flight = vk.device.swapchain_support.capabilities.minImageCount;
    vk.frames_in_flight = vk.images_in_flight - 1;
    vk_swapchain_query();
    vk_swapchain_create(false);
    Info("Swapchain created");
  
    vk.cmds = push_array(vk.arena, VkCommandBuffer, vk.frames_in_flight);
    // vk.compute_cmds = push_array(vk.arena, VkCommandBuffer, vk.frames_in_flight);
    Loop (i, vk.frames_in_flight) {
      vk.cmds[i] = vk_cmd_alloc(vk.device.cmd_pool);
      // vk.compute_cmds[i] = vk_cmd_alloc(vk.device.cmd_pool);
    }
    vk.cmd = vk_cmd_alloc(vk.device.cmd_pool);
    Info("Command buffers created");
  }

  // Sync
  {
    // NOTE: for some reasons validation layer complains about render_complete_sempahores when their number is frames_in_flight but doesn't when is images_in_flight
    vk.sync.render_complete_semaphores = push_array(vk.arena, VkSemaphore, vk.images_in_flight);
    vk.sync.image_available_semaphores = push_array(vk.arena, VkSemaphore, vk.images_in_flight);
    // vk.fence_images_in_flight = push_array_zero(vk.arena, VkFence, vk.images_in_flight);
    Loop (i, vk.images_in_flight) {
      VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.render_complete_semaphores[i]);
      vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.image_available_semaphores[i]);
    }
    vk.sync.in_flight_fences = push_array(vk.arena, VkFence, vk.frames_in_flight);
    Loop (i, vk.frames_in_flight) {
      VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
      };
      VK_CHECK(vk.CreateFence(vkdevice, &fence_create_info, vk.allocator, &vk.sync.in_flight_fences[i]));
    }

    // // NOTE: for some reasons validation layer complains about render_complete_sempahores when their number is frames_in_flight but doesn't when is images_in_flight
    // vk.sync.render_complete_semaphores = push_array(vk.arena, VkSemaphore, vk.images_in_flight);
    // // vk.sync.compute_complete_semaphores = push_array(vk.arena, VkSemaphore, vk.images_in_flight);
    // Loop (i, vk.images_in_flight) {
    //   VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    //   vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.render_complete_semaphores[i]);
    //   // vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.compute_complete_semaphores[i]);
    // }
    // vk.sync.image_available_semaphores = push_array(vk.arena, VkSemaphore, vk.images_in_flight);
    // vk.sync.in_flight_fences = push_array(vk.arena, VkFence, vk.frames_in_flight);
    // Loop (i, vk.frames_in_flight) {
    //   VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    //   vk.CreateSemaphore(vkdevice, &semaphore_create_info, vk.allocator, &vk.sync.image_available_semaphores[i]);
    //   VkFenceCreateInfo fence_create_info = {
    //     .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    //     .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    //   };
    //   VK_CHECK(vk.CreateFence(vkdevice, &fence_create_info, vk.allocator, &vk.sync.in_flight_fences[i]));
    // }
  }
  
  // Buffers
  {
    vk.vert_buffer = vk_buffer_create(MB(1), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk.index_buffer = vk_buffer_create(MB(1), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk.stage_buffer = vk_buffer_create(MB(10), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk_buffer_map_memory(vk.stage_buffer, 0, vk.stage_buffer.cap);
    vk.storage_buffer = vk_buffer_create(MB(200), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk_buffer_map_memory(vk.storage_buffer, 0, vk.storage_buffer.cap);
    vk.indirect_draw_buffer = vk_buffer_create(MB(100), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk_buffer_map_memory(vk.indirect_draw_buffer, 0, vk.indirect_draw_buffer.cap);
  }

  vk_descriptor_init();
  vk_texture_init();
  vk_shader_pipeline_init();

  // Load basic shaders
  ///////////////////////////////////
  // Debug lines
  {
    Shader shader = {
      .name = "line",
      .type = ShaderType_Drawing,
      .primitive = ShaderTopology_Line,
      .is_transparent = true,
      .use_depth = true,
    };
    vk.debug_line_shader.pipeline = vk_shader_pipeline_create(shader);
  }
  // Screen drawing
  {
    Shader shader = {
      .name = "screen",
      .type = ShaderType_Screen,
      .primitive = ShaderTopology_Triangle,
      .samples = 1,
    };
    vk.screen_shader.pipeline = vk_shader_pipeline_create(shader);
  }

  vk.scale = 1;
  // Msaa texture
  {
    VK_ImageInfo image_info = vk_image_info_default(vk.width, vk.height);
    image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_info.samples = VK_SAMPLE_COUNT_4_BIT;
    vk.msaa_texture_target = vk_image_create(image_info);
  }
  // Texture target
  {
    vk.texture_targets = push_array_zero(vk.arena, VK_Image, vk.images_in_flight);
    vk.textures.count += vk.images_in_flight;
    vk_texture_resize_target();
  }

  Info("Vulkan renderer initialized");
}

void vk_shutdown() {
  VK_CHECK(vk.DeviceWaitIdle(vkdevice));
  
  Loop (i, vk.frames_in_flight) {
    vk.DestroySemaphore(vkdevice, vk.sync.image_available_semaphores[i], vk.allocator);
    vk.DestroySemaphore(vkdevice, vk.sync.render_complete_semaphores[i], vk.allocator);
    vk.DestroyFence(vkdevice, vk.sync.in_flight_fences[i], vk.allocator);
    vk_cmd_free(vk.device.cmd_pool, vk.cmds[i]);
  }

  {
    vk.DestroyBuffer(vkdevice, vk.vert_buffer.handle, vk.allocator);
    vk.DestroyBuffer(vkdevice, vk.index_buffer.handle, vk.allocator);
    vk.DestroyBuffer(vkdevice, vk.stage_buffer.handle, vk.allocator);
    vk.DestroyBuffer(vkdevice, vk.storage_buffer.handle, vk.allocator);
  }
  
  vk_swapchain_destroy(vk.swapchain);
  
  // Device
  {
    Debug("Destroying Vulkan device...");
    vk.DestroyCommandPool(vkdevice, vk.device.cmd_pool, vk.allocator);
    vk.DestroyCommandPool(vkdevice, vk.device.cmd_pool, vk.allocator);
    vk.DestroyDevice(vkdevice, vk.allocator);
  }
  
  Info("Releasing physical device resources...");
  
  Debug("Destroying Vulkan surface...");
  vk.DestroySurfaceKHR(vk.instance, vk.surface, vk.allocator);
  
#if DUILD_DEBUG
  Debug("Destroying Vulkan debugger...");
  PFN_vkDestroyDebugUtilsMessengerEXT func; Assign(func, vkGetInstanceProcAddr(vk.instance, "vkDestroyDebugUtilsMessengerEXT"));
  func(vk.instance, vk.debug_messenger, vk.allocator);
#endif

  Debug("Destroying Vulkan instance...");
  vk.DestroyInstance(vk.instance, vk.allocator);
}

void vk_begin_frame() {
  // for (i32 i = 0; i < vk.debug_lines_remain.count; ) {
  //   vk.debug_line_times[i] -= g_dt;
  //   if (vk.debug_line_times[i] <= 0) {
  //     vk.debug_lines_remain.swap_remove(i);
  //   } else {
  //     ++i;
  //   }
  // }

  if (vk.debug_draw_lines.count > 0) {
    u32 size = vk.debug_draw_lines.count * sizeof(VK_State::DebugDrawLine);
    void* data = vk.debug_draw_lines.data;
    vk_buffer_upload_to_gpu(vk.vert_buffer, {vk.vert_buffer.size, size}, data);
  }

  VK_CHECK(vk.WaitForFences(vkdevice, 1, &vk.sync.in_flight_fences[vk.current_frame_idx], true, U64_MAX));
  VK_CHECK(vk.ResetFences(vkdevice, 1, &vk.sync.in_flight_fences[vk.current_frame_idx]));

  v2i win_size = os_get_window_size();
  if (vk.width != win_size.x || vk.height != win_size.y) {
    vk.width = win_size.x;
    vk.height = win_size.y;
    VK_CHECK(vk.DeviceWaitIdle(vkdevice));
    vk_swapchain_recreate();
    vk_texture_resize_target();
    Info("image idx: %i", vk.current_image_idx);
    vk.current_image_idx1 = 0;
  }

  vk.current_image_idx = vk_swapchain_acquire_next_image_index(vk_get_current_image_available_semaphore());
  // vk.current_image_idx = vk_swapchain_acquire_next_image_index(vk.sync.image_available_semaphores[vk.current_image_idx1]);
  // vk.current_image_idx1 = (vk.current_image_idx + 1) % vk.images_in_flight;
  // vk.current_image_idx1 = (vk.current_image_idx + 1) % vk.images_in_flight;
  // vk.current_image_idx_old = vk.current_image_idx;
  
  // if (vk.fence_images_in_flight[vk.current_image_idx] != VK_NULL_HANDLE) {
  //   vk.WaitForFences(vkdevice, 1, &vk.fence_images_in_flight[vk.current_image_idx], VK_TRUE, UINT64_MAX);
  // }
  // vk.fence_images_in_flight[vk.current_image_idx] = vk.sync.in_flight_fences[vk.current_frame_idx];

  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_begin(cmd);
  
  // VkViewport viewport = {
  //   .x = 0.0f,
  //   .y = 0,
  //   .width = (f32)vk.width,
  //   .height = (f32)vk.height,
  //   .minDepth = 0.0f,
  //   .maxDepth = 1.0f,
  // };
  // NOTE: we flip Y coordinate so Y:0 is on bottom of screen
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32)vk.height*vk.scale,
    .width = (f32)vk.width*vk.scale,
    .height = -(f32)vk.height*vk.scale,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  vk.CmdSetViewport(cmd, 0, 1, &viewport);
  VkRect2D scissor = {
    .offset = {.x = 0, .y = 0},
    .extent = {
      .width = (u32)(vk.width*vk.scale), 
      .height = (u32)(vk.height*vk.scale)
    },
  };
  vk.CmdSetScissor(cmd, 0, 1, &scissor);
}

void vk_end_frame() {
  VkCommandBuffer cmd = vk_get_current_cmd();
  vk_cmd_end(cmd);

  // graphics
  VkSemaphore semaphores_wait[] = {
    vk_get_current_image_available_semaphore(),
  };
  VkPipelineStageFlags sync_flags[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  };
  VkSemaphore semaphore_render_complete = vk_get_current_render_complete_semaphore();
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = ArrayCount(semaphores_wait),
    .pWaitSemaphores = semaphores_wait,
    .pWaitDstStageMask = sync_flags,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &semaphore_render_complete,
  };
  VK_CHECK(vk.QueueSubmit(vk.device.graphics_queue, 1, &submit_info, vk.sync.in_flight_fences[vk.current_frame_idx]));
  vk_swapchain_present(vk_get_current_render_complete_semaphore(), vk.current_image_idx);
  vk.current_frame_idx = (vk.current_frame_idx + 1) % vk.frames_in_flight;
}

intern VkRenderingAttachmentInfo vk_default_color_attachment_info(VkImageView view) {
  VkRenderingAttachmentInfo result = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = view,
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = {
      .color = {0.1f, 0.1f, 0.1f, 1.0f},
    }
  };
  return result;
}

intern VkRenderingAttachmentInfo vk_default_depth_attachment_info(VkImageView view) {
  VkRenderingAttachmentInfo result = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = view,
    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .clearValue = { 
      .depthStencil = {1, 0} 
    }
  };
  return result;
}

intern VkRenderingInfo vk_default_rendering_info(VkRenderingAttachmentInfo* color_attachment, VkRenderingAttachmentInfo* depth_attachment = null) {
  VkRenderingInfo result = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea = {
      .offset = {.x = 0, .y = 0}, 
      .extent = {.width = vk.width, .height = vk.height}
    },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = color_attachment,
    .pDepthAttachment = depth_attachment
  };
  return result;
}

void vk_begin_renderpass(RenderpassType renderpass_id) {
  VkCommandBuffer cmd = vk_get_current_cmd();
  switch (renderpass_id) {
    case RenderpassType_World: {
      vk_image_layout_transition(cmd, vk.msaa_texture_target, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      vk_image_layout_transition(cmd, vk.offscreen_depth_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      vk_image_layout_transition(cmd, vk.texture_targets[vk.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

      VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(vk.msaa_texture_target.view);
      color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
      color_attachment.resolveImageView = vk.texture_targets[vk.current_image_idx].view;
      color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(vk.offscreen_depth_buffer.view);
      
      VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      render_info.renderArea.extent = {vk.texture_targets->info.width, vk.texture_targets->info.height};
      vk.CmdBeginRendering(cmd, &render_info);

      ///////////////////////////////////
      // vk_image_layout_transition(cmd, vk.msaa_texture_target, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      // vk_image_layout_transition(cmd, vk.swapchain.depth_attachment, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      // vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

      // VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(vk.msaa_texture_target.view);
      // color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
      // color_attachment.resolveImageView = vk.swapchain.views[vk.current_image_idx];
      // color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      // VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(vk.swapchain.depth_attachment.view);
      // // depth_attachment
      
      // VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      // vk.CmdBeginRendering(cmd, &render_info);

      ///////////////////////////////////
      // // OLD
      // // Color
      // vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      // // Depth
      // vk_image_layout_transition(cmd, vk.swapchain.depth_attachment, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      // // Render
      // VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(vk.swapchain.views[vk.current_image_idx]);
      // VkRenderingAttachmentInfo depth_attachment = vk_default_depth_attachment_info(vk.swapchain.depth_attachment.view);
      // VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment, &depth_attachment);
      // vk.CmdBeginRendering(cmd, &render_info);
    } break;
    case RenderpassType_UI: {
    } break;
    case RenderpassType_Screen: {
      vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      VkRenderingAttachmentInfo color_attachment = vk_default_color_attachment_info(vk.swapchain.images[vk.current_image_idx].view);
      VkRenderingInfo render_info = vk_default_rendering_info(&color_attachment);
      vk.CmdBeginRendering(cmd, &render_info);
    } break;
  }
  return;
}

void vk_end_renderpass(RenderpassType renderpass) {
  VkCommandBuffer cmd = vk_get_current_cmd();
  switch (renderpass) {
    case RenderpassType_World: {
      vk.CmdEndRendering(cmd);
      vk_image_layout_transition(cmd, vk.texture_targets[vk.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      ///////////////////////////////////
      // vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    } break;
    case RenderpassType_UI: {
    } break;
    case RenderpassType_Screen: {
      vk.CmdEndRendering(cmd);
      vk_image_layout_transition(cmd, vk.swapchain.images[vk.current_image_idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    } break;
  }
  return;
}

void vk_begin_draw_frame() {
}

void vk_end_draw_frame() {
  vk_begin_frame();
  // World
  {
    vk_begin_renderpass(RenderpassType_World);
    vk_draw();
    vk_end_renderpass(RenderpassType_World);
  }
  // {
  //   vk_begin_renderpass(Renderpass_UI);
  //   ui_begin_frame();
  //   ui_end_frame();
  //   vk_end_renderpass(Renderpass_UI);
  // }
  // Screen
  {
    VkCommandBuffer cmd = vk_get_current_cmd();
    VkViewport viewport = {
      .x = 0.0f,
      .y = (f32)vk.height,
      .width = (f32)vk.width,
      .height = -(f32)vk.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    };
    vk.CmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor = {
      .offset = {.x = 0, .y = 0},
      .extent = {
        .width = (u32)vk.width, 
        .height = (u32)vk.height
      },
    };
    vk.CmdSetScissor(cmd, 0, 1, &scissor);
    vk_begin_renderpass(RenderpassType_Screen);
    vk_draw_screen();
    vk_end_renderpass(RenderpassType_Screen);
  }
  vk_end_frame();
}

////////////////////////////////////////////////////////////////////////
// Entity

void vk_make_renderable(Handle<Entity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuShader> shader_handle, Handle<GpuMaterial> material_handle) {
  u32 entity_idx = entity_handle.handle;
  u32 shader_idx = shader_handle.handle;
  vk.entities_to_shader[entity_idx] = shader_handle;
  u32 mesh_idx = mesh_handle.handle;
  VK_Mesh mesh = vk.meshes[mesh_idx];
  // Indexed
  if (mesh.index_count) {
    ShaderBatch& shader_batch = vk.shaders[shader_idx].batch_indexed;
    u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
    u32 mesh_idx_in_array;
    if (!p_mesh_idx_in_array) {
      mesh_idx_in_array = shader_batch.mesh_batches.count;
      p_mesh_idx_in_array = &mesh_idx_in_array;
      shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
      MeshBatch mesh_batch = {.mesh_handle = mesh_handle};
      shader_batch.mesh_batches.add(mesh_batch);
    }
    MeshBatch& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
    mesh_batch.entities.add(entity_handle);
    u32 entity_idx_in_array = mesh_batch.entities.count;
  }
  // Not Indexed
  else {
    ShaderBatch& shader_batch = vk.shaders[shader_idx].batch;
    u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
    u32 mesh_idx_in_array;
    if (!p_mesh_idx_in_array) {
      mesh_idx_in_array = shader_batch.mesh_batches.count;
      p_mesh_idx_in_array = &mesh_idx_in_array;
      shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
      MeshBatch mesh_batch = {.mesh_handle = mesh_handle};
      shader_batch.mesh_batches.add(mesh_batch);
    }
    MeshBatch& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
    mesh_batch.entities.add(entity_handle);
    u32 entity_idx_in_array = mesh_batch.entities.count;
  }
  vk.entities_to_mesh[entity_idx] = mesh_handle;
  vk.gpu_entities[entity_idx].material = material_handle;
}

// void vk_make_renderable(Handle<Entity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuShader> shader_handle, Handle<GpuMaterial> material_handle) {
//   u32 entity_idx = entity_handle.handle;
//   u32 shader_idx = shader_handle.handle;
//   vk.entities_to_shader[entity_idx] = shader_handle;
//   u32 mesh_idx = mesh_handle.handle;
//   VK_Mesh mesh = vk.meshes[mesh_idx];
//   if (mesh.index_count) {
//     vk.entities[entity_idx].shader_handle = vk.shaders[shader_idx].entities_indexed.add(entity_handle);
//     // u32 shader_idx = vk.entities_to_shader[entity_idx].handle;
//     // ShaderBatch& shader_batch = vk.shaders[shader_idx].batch_indexed;
//     // u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
//     // u32 mesh_idx_in_array;
//     // if (!p_mesh_idx_in_array) {
//     //   mesh_idx_in_array = shader_batch.mesh_batches.count;
//     //   p_mesh_idx_in_array = &mesh_idx_in_array;
//     //   shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
//     //   MeshBatch mesh_batch = {.mesh_handle = mesh_handle};
//     //   shader_batch.mesh_batches.add(mesh_batch);
//     // }
//     // MeshBatch& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
//     // mesh_batch.entities.add(entity_handle);
//     // u32 entity_idx_in_array = mesh_batch.entities.count;
//   } else {
//     vk.entities[entity_idx].shader_handle = vk.shaders[shader_idx].entities.add(entity_handle);
//     // u32 shader_idx = vk.entities_to_shader[entity_idx].handle;
//     // ShaderBatch& shader_batch = vk.shaders[shader_idx].batch;
//     // u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
//     // u32 mesh_idx_in_array;
//     // if (!p_mesh_idx_in_array) {
//     //   mesh_idx_in_array = shader_batch.mesh_batches.count;
//     //   p_mesh_idx_in_array = &mesh_idx_in_array;
//     //   shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
//     //   MeshBatch mesh_batch = {.mesh_handle = mesh_handle};
//     //   shader_batch.mesh_batches.add(mesh_batch);
//     // }
//     // MeshBatch& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
//     // mesh_batch.entities.add(entity_handle);
//     // u32 entity_idx_in_array = mesh_batch.entities.count;
//   }
//   vk.entities_to_mesh[entity_idx] = mesh_handle;
//   vk.gpu_entities[entity_idx].material = material_handle;
// }

void vk_make_renderable_static(Handle<StaticEntity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuShader> shader_handle, Handle<GpuMaterial> material_handle) {
  u32 entity_idx = entity_handle.handle + MaxEntities;
  u32 shader_idx = shader_handle.handle;
  vk.entities_to_shader[entity_idx] = shader_handle;
  u32 mesh_idx = mesh_handle.handle;
  VK_Mesh mesh = vk.meshes[mesh_idx];
  // Indexed
  if (mesh.index_count) {
    ++vk.shaders[shader_idx].static_entities_indexed_count;
    ShaderBatch& shader_batch = vk.shaders[shader_idx].static_batch_indexed;
    u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
    u32 mesh_idx_in_array;
    if (!p_mesh_idx_in_array) {
      mesh_idx_in_array = shader_batch.mesh_batches.count;
      p_mesh_idx_in_array = &mesh_idx_in_array;
      shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
      MeshBatch mesh_batch = {.mesh_handle = mesh_handle};
      shader_batch.mesh_batches.add(mesh_batch);
    }
    MeshBatch& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
    mesh_batch.entities.add((Handle<Entity>)entity_handle.handle);
    u32 entity_idx_in_array = mesh_batch.entities.count;
  }
  // Not Indexed
  else {
    ++vk.shaders[shader_idx].static_entities_count;
    ShaderBatch& shader_batch = vk.shaders[shader_idx].batch;
    u32* p_mesh_idx_in_array = shader_batch.mesh_to_batch.get(mesh_idx);
    u32 mesh_idx_in_array;
    if (!p_mesh_idx_in_array) {
      mesh_idx_in_array = shader_batch.mesh_batches.count;
      p_mesh_idx_in_array = &mesh_idx_in_array;
      shader_batch.mesh_to_batch.add(mesh_idx, mesh_idx_in_array);
      MeshBatch mesh_batch = {.mesh_handle = mesh_handle};
      shader_batch.mesh_batches.add(mesh_batch);
    }
    MeshBatch& mesh_batch = shader_batch.mesh_batches[*p_mesh_idx_in_array];
    mesh_batch.entities.add((Handle<Entity>)entity_handle.handle);
    u32 entity_idx_in_array = mesh_batch.entities.count;
  }
  vk.entities_to_mesh[entity_idx] = mesh_handle;
  // vk.entities[entity_idx].shader_handle = vk.shaders[shader_idx].entities.add(entity_handle);
  vk.gpu_entities[entity_idx].material = material_handle;
}

// void vk_make_renderable_static(Handle<StaticEntity> entity_handle, Handle<GpuMesh> mesh_handle, Handle<GpuShader> shader_handle, Handle<GpuMaterial> material_handle) {
//   u32 entity_idx = entity_handle.handle + MaxEntities;
//   u32 shader_idx = shader_handle.handle;
//   vk.entities_to_shader[entity_idx] = shader_handle;
//   u32 mesh_idx = mesh_handle.handle;
//   VK_Mesh mesh = vk.meshes[mesh_idx];
//   if (mesh.index_count) {
//     vk.entities[entity_idx].shader_handle = (Handle<Handle<Entity>>)vk.shaders[shader_idx].static_entities_indexed.add(entity_handle).handle;
//   } else {
//     vk.entities[entity_idx].shader_handle = (Handle<Handle<Entity>>)vk.shaders[shader_idx].static_entities.add(entity_handle).handle;
//   }
//   vk.entities_to_mesh[entity_idx] = mesh_handle;
//   // vk.entities[entity_idx].shader_handle = vk.shaders[shader_idx].entities.add(entity_handle);
//   vk.gpu_entities[entity_idx].material = material_handle;
// }

void vk_remove_renderable(Handle<Entity> entity_handle) {
  u32 entity_idx = entity_handle.handle;
  u32 shader_idx = vk.entities_to_shader[entity_idx].handle;
  u32 mesh_idx = vk.entities_to_mesh[entity_handle.handle].handle;
  VK_Mesh mesh = vk.meshes[mesh_idx];
  if (mesh.index_count) {
    Handle<Handle<Entity>> shader_handle = vk.entities[entity_idx].shader_handle;
    VK_Shader& shader = vk.shaders[shader_idx];
    shader.entities_indexed.remove(shader_handle);
  } else {
    Handle<Handle<Entity>> shader_handle = vk.entities[entity_idx].shader_handle;
    VK_Shader& shader = vk.shaders[shader_idx];
    shader.entities.remove(shader_handle);
  }
}

////////////////////////////////////////////////////////////////////////
// Point light
// KAPI void vk_point_light_create(u32 entity_id) {
//   // vk.point_light_data.add(entity_id);
//   // ++vk.global_shader_state->point_light_count;
// }

// KAPI void vk_point_light_destroy(u32 entity_id) {
//   // vk.point_light_data.remove_data(entity_id);;
//   // --vk.global_shader_state->point_light_count;
// }

// KAPI PointLight& vk_point_light_get(u32 entity_id) {
//   // return *(PointLight*)vk.point_light_data.get_data(entity_id);
// }

// ////////////////////////////////////////////////////////////////////////
// // Directional light
// KAPI void vk_dir_light_create(u32 entity_id) {
//   // vk.dir_light_data.add(entity_id);
//   // ++vk.global_shader_state->dir_light_count;
// }

// KAPI void vk_dir_light_destroy(u32 entity_id) {
//   // vk.dir_light_data.remove_data(entity_id);
//   // --vk.global_shader_state->dir_light_count;
// }

// KAPI DirLight& vk_dir_light_get(u32 entity_id) {
//   // return *(DirLight*)vk.dir_light_data.get_data(entity_id);
// }

// ////////////////////////////////////////////////////////////////////////
// // Spot light
// KAPI void vk_spot_light_create(u32 entity_id) {
//   // vk.spot_light_data.add(entity_id);
//   // ++vk.global_shader_state->spot_light_count;
// }

// KAPI void vk_spot_light_destroy(u32 entity_id) {
//   // vk.spot_light_data.remove_data(entity_id);
//   // --vk.global_shader_state->spot_light_count;
// }

// KAPI SpotLight& vk_spot_light_get(u32 entity_id) {
//   // return *(SpotLight*)vk.spot_light_data.get_data(entity_id);
// }

////////////////////////////////////////////////////////////////////////
// Util

GpuGlobalState* vk_get_shader_state() {
  return (GpuGlobalState*)vk.storage_buffer.mapped_memory;
}

mat4& vk_get_view() { return vk.view; }
mat4& vk_get_projection() { return vk.projection; };

void debug_draw_line(v3 a, v3 b, v3 color) {
  Vertex vert[] = {
    {.pos = a, .color = color},
    {.pos = b, .color = color},
  };
  // vk.debug_lines.add({vert[0], vert[1]});
}

void debug_draw_line_time(v3 a, v3 b, v3 color, f32 time) {
  Vertex vert[] = {
    {.pos = a, .color = color},
    {.pos = b, .color = color},
  };
  vk.debug_draw_lines.add({vert[0], vert[1]});
  vk.debug_draw_line_times.add(time);
}

void debug_draw_aabb(v3 min, v3 max, v3 color) {
  v3 p000 = {min.x, min.y, min.z};
  v3 p001 = {min.x, min.y, max.z};
  v3 p010 = {min.x, max.y, min.z};
  v3 p011 = {min.x, max.y, max.z};

  v3 p100 = {max.x, min.y, min.z};
  v3 p101 = {max.x, min.y, max.z};
  v3 p110 = {max.x, max.y, min.z};
  v3 p111 = {max.x, max.y, max.z};

  debug_draw_line(p000, p001, color);
  debug_draw_line(p000, p010, color);
  debug_draw_line(p000, p100, color);

  debug_draw_line(p111, p110, color);
  debug_draw_line(p111, p101, color);
  debug_draw_line(p111, p011, color);

  debug_draw_line(p001, p011, color);
  debug_draw_line(p001, p101, color);

  debug_draw_line(p010, p011, color);
  debug_draw_line(p010, p110, color);

  debug_draw_line(p100, p101, color);
  debug_draw_line(p100, p110, color);
}

