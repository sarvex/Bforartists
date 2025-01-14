/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2023 Blender Foundation */

/** \file
 * \ingroup gpu
 */

#pragma once

#include "vk_common.hh"
#include "vk_resource_tracker.hh"

#include "BLI_utility_mixins.hh"

namespace blender::gpu {
class VKBuffer;
class VKDescriptorSet;
class VKFrameBuffer;
class VKPipeline;
class VKPushConstants;
class VKTexture;

/** Command buffer to keep track of the life-time of a command buffer. */
class VKCommandBuffer : NonCopyable, NonMovable {
  /** None owning handle to the command buffer and device. Handle is owned by `GHOST_ContextVK`. */
  VkDevice vk_device_ = VK_NULL_HANDLE;
  VkCommandBuffer vk_command_buffer_ = VK_NULL_HANDLE;
  VkQueue vk_queue_ = VK_NULL_HANDLE;

  /** Owning handles */
  VkFence vk_fence_ = VK_NULL_HANDLE;
  VKSubmissionID submission_id_;

 public:
  virtual ~VKCommandBuffer();
  void init(const VkDevice vk_device, const VkQueue vk_queue, VkCommandBuffer vk_command_buffer);
  void begin_recording();
  void end_recording();
  void bind(const VKPipeline &vk_pipeline, VkPipelineBindPoint bind_point);
  void bind(const VKDescriptorSet &descriptor_set,
            const VkPipelineLayout vk_pipeline_layout,
            VkPipelineBindPoint bind_point);
  void begin_render_pass(const VKFrameBuffer &framebuffer);
  void end_render_pass(const VKFrameBuffer &framebuffer);

  /**
   * Add a push constant command to the command buffer.
   *
   * Only valid when the storage type of push_constants is StorageType::PUSH_CONSTANTS.
   */
  void push_constants(const VKPushConstants &push_constants,
                      const VkPipelineLayout vk_pipeline_layout,
                      const VkShaderStageFlags vk_shader_stages);
  void dispatch(int groups_x_len, int groups_y_len, int groups_z_len);
  /** Copy the contents of a texture MIP level to the dst buffer. */
  void copy(VKBuffer &dst_buffer, VKTexture &src_texture, Span<VkBufferImageCopy> regions);
  void copy(VKTexture &dst_texture, VKBuffer &src_buffer, Span<VkBufferImageCopy> regions);
  void pipeline_barrier(VkPipelineStageFlags source_stages,
                        VkPipelineStageFlags destination_stages);
  void pipeline_barrier(Span<VkImageMemoryBarrier> image_memory_barriers);
  /**
   * Clear color image resource.
   */
  void clear(VkImage vk_image,
             VkImageLayout vk_image_layout,
             const VkClearColorValue &vk_clear_color,
             Span<VkImageSubresourceRange> ranges);

  /**
   * Clear attachments of the active framebuffer.
   */
  void clear(Span<VkClearAttachment> attachments, Span<VkClearRect> areas);
  void fill(VKBuffer &buffer, uint32_t data);

  /**
   * Stop recording commands, encode + send the recordings to Vulkan, wait for the until the
   * commands have been executed and start the command buffer to accept recordings again.
   */
  void submit();

  const VKSubmissionID &submission_id_get() const
  {
    return submission_id_;
  }

 private:
  void encode_recorded_commands();
  void submit_encoded_commands();
};

}  // namespace blender::gpu
