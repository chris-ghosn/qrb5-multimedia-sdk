/* GStreamer V4L2 Video Encoder Element - Header
 * Copyright (c) 2024-2026 Qualcomm Innovation Center, Inc.
 */

#ifndef __GST_V4L2_VIDEO_ENC_H__
#define __GST_V4L2_VIDEO_ENC_H__

#include <gst/gst.h>
#include <gst/video/gstvideoencoder.h>
#include "gstv4l2object.h"

G_BEGIN_DECLS

#define GST_TYPE_V4L2_VIDEO_ENC (gst_v4l2_video_enc_get_type())
#define GST_V4L2_VIDEO_ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_V4L2_VIDEO_ENC, GstV4l2VideoEnc))
#define GST_V4L2_VIDEO_ENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_V4L2_VIDEO_ENC, GstV4l2VideoEncClass))

typedef struct _GstV4l2VideoEnc GstV4l2VideoEnc;
typedef struct _GstV4l2VideoEncClass GstV4l2VideoEncClass;

struct _GstV4l2VideoEnc {
  GstVideoEncoder parent;
};

struct _GstV4l2VideoEncClass {
  GstVideoEncoderClass parent_class;
};

GType gst_v4l2_video_enc_get_type (void);

G_END_DECLS

#endif /* __GST_V4L2_VIDEO_ENC_H__ */
