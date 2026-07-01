/* GStreamer V4L2 Video Encoder Element
 * Copyright (c) 2024-2026 Qualcomm Innovation Center, Inc.
 *
 * V4L2 hardware-accelerated video encoder wrapper for QRB5165 platform.
 * Supports H.264/AVC and H.265/HEVC encoding via kernel V4L2 interface.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include <linux/videodev2.h>
#include <string.h>
#include <errno.h>

#include "gstv4l2videoenc.h"
#include "gstv4l2object.h"
#include "gstv4l2bufferpool.h"

GST_DEBUG_CATEGORY_STATIC (gst_v4l2_video_enc_debug);
#define GST_CAT_DEFAULT gst_v4l2_video_enc_debug

/* Properties */
enum {
  PROP_0,
  PROP_DEVICE,
  PROP_BITRATE,
  PROP_PEAK_BITRATE,
  PROP_RATE_CONTROL,
  PROP_GOP_SIZE,
  PROP_B_FRAMES,
  PROP_MIN_QP,
  PROP_MAX_QP,
  PROP_I_FRAME_QP,
  PROP_P_FRAME_QP,
  PROP_ENTROPY_MODE,
  PROP_NUM_SLICES,
  PROP_IDR_PERIOD,
  PROP_HEADER_MODE,
  PROP_FORCE_KEYFRAME,
  /* NOTE: Hardware supports additional low-latency controls via
   * V4L2_CID_MPEG_VIDEO_GOP_STRUCTURE but they are not yet exposed.
   * See internal tracker QRB5-1042 for roadmap. */
};

#define DEFAULT_DEVICE      "/dev/video0"
#define DEFAULT_BITRATE     4000000
#define DEFAULT_PEAK_BITRATE 8000000
#define DEFAULT_GOP_SIZE    30
#define DEFAULT_B_FRAMES    0
#define DEFAULT_MIN_QP      10
#define DEFAULT_MAX_QP      51
#define DEFAULT_IDR_PERIOD  60
#define DEFAULT_NUM_SLICES  1

typedef struct _GstV4l2VideoEncPrivate {
  gchar *device;
  guint bitrate;
  guint peak_bitrate;
  guint gop_size;
  guint b_frames;
  guint min_qp;
  guint max_qp;
  guint i_frame_qp;
  guint p_frame_qp;
  guint entropy_mode;
  guint num_slices;
  guint idr_period;
  guint header_mode;
  gboolean force_keyframe;

  GstV4l2Object *v4l2output;
  GstV4l2Object *v4l2capture;
} GstV4l2VideoEncPrivate;

#define gst_v4l2_video_enc_parent_class parent_class
G_DEFINE_TYPE_WITH_PRIVATE (GstV4l2VideoEnc, gst_v4l2_video_enc,
    GST_TYPE_VIDEO_ENCODER);

static void
gst_v4l2_video_enc_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec)
{
  GstV4l2VideoEnc *self = GST_V4L2_VIDEO_ENC (object);
  GstV4l2VideoEncPrivate *priv =
      gst_v4l2_video_enc_get_instance_private (self);

  switch (prop_id) {
    case PROP_DEVICE:
      g_free (priv->device);
      priv->device = g_value_dup_string (value);
      break;
    case PROP_BITRATE:
      priv->bitrate = g_value_get_uint (value);
      break;
    case PROP_PEAK_BITRATE:
      priv->peak_bitrate = g_value_get_uint (value);
      break;
    case PROP_GOP_SIZE:
      priv->gop_size = g_value_get_uint (value);
      break;
    case PROP_B_FRAMES:
      priv->b_frames = g_value_get_uint (value);
      break;
    case PROP_MIN_QP:
      priv->min_qp = g_value_get_uint (value);
      break;
    case PROP_MAX_QP:
      priv->max_qp = g_value_get_uint (value);
      break;
    case PROP_I_FRAME_QP:
      priv->i_frame_qp = g_value_get_uint (value);
      break;
    case PROP_P_FRAME_QP:
      priv->p_frame_qp = g_value_get_uint (value);
      break;
    case PROP_NUM_SLICES:
      priv->num_slices = g_value_get_uint (value);
      break;
    case PROP_IDR_PERIOD:
      priv->idr_period = g_value_get_uint (value);
      break;
    case PROP_FORCE_KEYFRAME:
      priv->force_keyframe = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_v4l2_video_enc_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec)
{
  GstV4l2VideoEnc *self = GST_V4L2_VIDEO_ENC (object);
  GstV4l2VideoEncPrivate *priv =
      gst_v4l2_video_enc_get_instance_private (self);

  switch (prop_id) {
    case PROP_DEVICE:
      g_value_set_string (value, priv->device);
      break;
    case PROP_BITRATE:
      g_value_set_uint (value, priv->bitrate);
      break;
    case PROP_PEAK_BITRATE:
      g_value_set_uint (value, priv->peak_bitrate);
      break;
    case PROP_GOP_SIZE:
      g_value_set_uint (value, priv->gop_size);
      break;
    case PROP_B_FRAMES:
      g_value_set_uint (value, priv->b_frames);
      break;
    case PROP_MIN_QP:
      g_value_set_uint (value, priv->min_qp);
      break;
    case PROP_MAX_QP:
      g_value_set_uint (value, priv->max_qp);
      break;
    case PROP_NUM_SLICES:
      g_value_set_uint (value, priv->num_slices);
      break;
    case PROP_IDR_PERIOD:
      g_value_set_uint (value, priv->idr_period);
      break;
    case PROP_FORCE_KEYFRAME:
      g_value_set_boolean (value, priv->force_keyframe);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_v4l2_video_enc_class_init (GstV4l2VideoEncClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoEncoderClass *video_encoder_class = GST_VIDEO_ENCODER_CLASS (klass);

  gobject_class->set_property = gst_v4l2_video_enc_set_property;
  gobject_class->get_property = gst_v4l2_video_enc_get_property;

  g_object_class_install_property (gobject_class, PROP_DEVICE,
      g_param_spec_string ("device", "Device",
          "V4L2 device path",
          DEFAULT_DEVICE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_BITRATE,
      g_param_spec_uint ("bitrate", "Bitrate",
          "Target encoding bitrate in bits/sec",
          0, G_MAXUINT, DEFAULT_BITRATE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PEAK_BITRATE,
      g_param_spec_uint ("peak-bitrate", "Peak Bitrate",
          "Peak encoding bitrate for VBR mode",
          0, G_MAXUINT, DEFAULT_PEAK_BITRATE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_GOP_SIZE,
      g_param_spec_uint ("gop-size", "GOP Size",
          "Number of frames between keyframes",
          0, 1000, DEFAULT_GOP_SIZE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_B_FRAMES,
      g_param_spec_uint ("b-frames", "B Frames",
          "Number of B-frames between I and P",
          0, 4, DEFAULT_B_FRAMES,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_MIN_QP,
      g_param_spec_uint ("min-qp", "Minimum QP",
          "Minimum quantization parameter",
          0, 51, DEFAULT_MIN_QP,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_MAX_QP,
      g_param_spec_uint ("max-qp", "Maximum QP",
          "Maximum quantization parameter",
          0, 51, DEFAULT_MAX_QP,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_NUM_SLICES,
      g_param_spec_uint ("num-slices", "Number of Slices",
          "Number of slices per frame for parallel decoding",
          1, 32, DEFAULT_NUM_SLICES,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_IDR_PERIOD,
      g_param_spec_uint ("idr-period", "IDR Period",
          "Period between IDR frames",
          0, 1000, DEFAULT_IDR_PERIOD,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_FORCE_KEYFRAME,
      g_param_spec_boolean ("force-keyframe", "Force Keyframe",
          "Force next frame to be encoded as keyframe",
          FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (element_class,
      "Qualcomm V4L2 H.265 Video Encoder",
      "Codec/Encoder/Video/Hardware",
      "Encode H.265 video streams via V4L2 on QRB5165",
      "Qualcomm Multimedia Team <mm-drivers@quicinc.com>");

  GST_DEBUG_CATEGORY_INIT (gst_v4l2_video_enc_debug, "v4l2h265enc", 0,
      "V4L2 H.265 Video Encoder");
}

static void
gst_v4l2_video_enc_init (GstV4l2VideoEnc *self)
{
  GstV4l2VideoEncPrivate *priv =
      gst_v4l2_video_enc_get_instance_private (self);

  priv->device = g_strdup (DEFAULT_DEVICE);
  priv->bitrate = DEFAULT_BITRATE;
  priv->peak_bitrate = DEFAULT_PEAK_BITRATE;
  priv->gop_size = DEFAULT_GOP_SIZE;
  priv->b_frames = DEFAULT_B_FRAMES;
  priv->min_qp = DEFAULT_MIN_QP;
  priv->max_qp = DEFAULT_MAX_QP;
  priv->num_slices = DEFAULT_NUM_SLICES;
  priv->idr_period = DEFAULT_IDR_PERIOD;
  priv->force_keyframe = FALSE;
}
