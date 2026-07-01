/* GStreamer V4L2 Object - Helper for device I/O
 * Copyright (c) 2024-2026 Qualcomm Innovation Center, Inc.
 */

#ifndef __GST_V4L2_OBJECT_H__
#define __GST_V4L2_OBJECT_H__

#include <gst/gst.h>
#include <linux/videodev2.h>

G_BEGIN_DECLS

typedef struct _GstV4l2Object GstV4l2Object;

struct _GstV4l2Object {
  gint fd;
  gchar *device;
  enum v4l2_buf_type type;
  guint32 pixelformat;
  guint width;
  guint height;
  guint fps_n;
  guint fps_d;
  gboolean streaming;
};

GstV4l2Object * gst_v4l2_object_new (GstElement *element);
void gst_v4l2_object_free (GstV4l2Object *obj);
gboolean gst_v4l2_object_open (GstV4l2Object *obj);
gboolean gst_v4l2_object_close (GstV4l2Object *obj);
gboolean gst_v4l2_object_set_format (GstV4l2Object *obj);
gboolean gst_v4l2_object_start_streaming (GstV4l2Object *obj);
gboolean gst_v4l2_object_stop_streaming (GstV4l2Object *obj);

G_END_DECLS

#endif /* __GST_V4L2_OBJECT_H__ */
