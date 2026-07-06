/* v4l2h265enc: slice-intra-refresh & low-latency-mode properties
 *
 * Exposes two new GObject properties on the v4l2h265enc element:
 *
 *   slice-intra-refresh (guint, default 0)
 *     Maps to V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD.
 *     Interval (in frames) at which the encoder inserts an intra-refresh slice,
 *     enabling error recovery without a full IDR frame — critical for lossy 5G links.
 *
 *   low-latency-mode (gboolean, default FALSE)
 *     Maps to V4L2_CID_MPEG_VIDEO_HEVC_LOW_LATENCY.
 *     When TRUE, the encoder flushes each slice immediately rather than buffering
 *     a full frame, targeting sub-100ms glass-to-glass latency.
 *
 * Requested by Skydio for the X10 drone 5G live-stream pipeline (QRB5165 BSP).
 */
#ifndef __GST_V4L2_H265_ENC_PROPS_H__
#define __GST_V4L2_H265_ENC_PROPS_H__

#include <gst/gst.h>

G_BEGIN_DECLS

void gst_v4l2_h265enc_register_low_latency_props (GObjectClass * klass);
void gst_v4l2_h265enc_apply_low_latency_props   (GObject * obj, gint fd);

G_END_DECLS

#endif /* __GST_V4L2_H265_ENC_PROPS_H__ */

