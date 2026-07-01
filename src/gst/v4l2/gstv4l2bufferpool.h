/* GStreamer V4L2 Buffer Pool - DMA buffer management
 * Copyright (c) 2024-2026 Qualcomm Innovation Center, Inc.
 */

#ifndef __GST_V4L2_BUFFER_POOL_H__
#define __GST_V4L2_BUFFER_POOL_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include "gstv4l2object.h"

G_BEGIN_DECLS

typedef struct _GstV4l2BufferPool GstV4l2BufferPool;

GstV4l2BufferPool * gst_v4l2_buffer_pool_new (GstV4l2Object *obj, GstCaps *caps);
void gst_v4l2_buffer_pool_free (GstV4l2BufferPool *pool);
GstFlowReturn gst_v4l2_buffer_pool_acquire (GstV4l2BufferPool *pool, GstBuffer **buf);
GstFlowReturn gst_v4l2_buffer_pool_release (GstV4l2BufferPool *pool, GstBuffer *buf);

G_END_DECLS

#endif /* __GST_V4L2_BUFFER_POOL_H__ */
