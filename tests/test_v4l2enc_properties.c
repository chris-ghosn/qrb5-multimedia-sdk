/* Unit tests for V4L2 encoder property interface */

#include <gst/gst.h>
#include <gst/check/gstcheck.h>

GST_START_TEST (test_bitrate_property)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  g_object_set (enc, "bitrate", 2000000, NULL);
  guint bitrate;
  g_object_get (enc, "bitrate", &bitrate, NULL);
  ck_assert_uint_eq (bitrate, 2000000);

  gst_object_unref (enc);
}
GST_END_TEST;

GST_START_TEST (test_gop_size_property)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  g_object_set (enc, "gop-size", 15, NULL);
  guint gop;
  g_object_get (enc, "gop-size", &gop, NULL);
  ck_assert_uint_eq (gop, 15);

  gst_object_unref (enc);
}
GST_END_TEST;

GST_START_TEST (test_default_values)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  guint bitrate, gop, num_slices;
  g_object_get (enc,
      "bitrate", &bitrate,
      "gop-size", &gop,
      "num-slices", &num_slices,
      NULL);

  ck_assert_uint_eq (bitrate, 4000000);
  ck_assert_uint_eq (gop, 30);
  ck_assert_uint_eq (num_slices, 1);

  gst_object_unref (enc);
}
GST_END_TEST;

/* ------------------------------------------------------------------ *
 * Tests for slice-intra-refresh (Skydio X10 5G live-stream feature)  *
 * ------------------------------------------------------------------ */

GST_START_TEST (test_slice_intra_refresh_default)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  guint sir;
  g_object_get (enc, "slice-intra-refresh", &sir, NULL);
  /* Feature is off by default; zero means rolling-intra-refresh disabled */
  ck_assert_uint_eq (sir, 0);

  gst_object_unref (enc);
}
GST_END_TEST;

GST_START_TEST (test_slice_intra_refresh_set)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  /* Refresh 4 MB-rows per frame; at 1080p (68 MB rows) and gop-size=60
   * every region is intra-refreshed roughly every 17 frames (~570 ms at
   * 30 fps) without an IDR frame penalty. */
  g_object_set (enc, "slice-intra-refresh", 4, NULL);
  guint sir;
  g_object_get (enc, "slice-intra-refresh", &sir, NULL);
  ck_assert_uint_eq (sir, 4);

  gst_object_unref (enc);
}
GST_END_TEST;

GST_START_TEST (test_slice_intra_refresh_max_boundary)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  g_object_set (enc, "slice-intra-refresh", 255, NULL);
  guint sir;
  g_object_get (enc, "slice-intra-refresh", &sir, NULL);
  ck_assert_uint_eq (sir, 255);

  gst_object_unref (enc);
}
GST_END_TEST;

/* ------------------------------------------------------------------ *
 * Tests for low-latency-mode (Skydio X10 5G live-stream feature)     *
 * ------------------------------------------------------------------ */

GST_START_TEST (test_low_latency_mode_default)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  gboolean llm;
  g_object_get (enc, "low-latency-mode", &llm, NULL);
  /* Must be FALSE by default to preserve existing pipeline behaviour */
  ck_assert_int_eq (llm, FALSE);

  gst_object_unref (enc);
}
GST_END_TEST;

GST_START_TEST (test_low_latency_mode_enable)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  g_object_set (enc, "low-latency-mode", TRUE, NULL);
  gboolean llm;
  g_object_get (enc, "low-latency-mode", &llm, NULL);
  ck_assert_int_eq (llm, TRUE);

  gst_object_unref (enc);
}
GST_END_TEST;

GST_START_TEST (test_low_latency_mode_roundtrip)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  /* Enable then disable -- property must reflect last written value */
  g_object_set (enc, "low-latency-mode", TRUE, NULL);
  g_object_set (enc, "low-latency-mode", FALSE, NULL);
  gboolean llm;
  g_object_get (enc, "low-latency-mode", &llm, NULL);
  ck_assert_int_eq (llm, FALSE);

  gst_object_unref (enc);
}
GST_END_TEST;

/* ------------------------------------------------------------------ *
 * Integration: combined low-latency configuration                    *
 * ------------------------------------------------------------------ */

GST_START_TEST (test_low_latency_combined_config)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  /* Typical Skydio X10 enterprise 5G pipeline configuration:
   *   - low-latency-mode   TRUE  -> single-frame pipeline depth
   *   - slice-intra-refresh 4    -> rolling refresh every ~17 frames
   *   - idr-period         0     -> no periodic IDR, rely on SIR
   *   - bitrate            8 Mbps */
  g_object_set (enc,
      "low-latency-mode",    TRUE,
      "slice-intra-refresh", 4,
      "idr-period",          0,
      "bitrate",             8000000,
      NULL);

  gboolean llm;
  guint sir, idr, bps;
  g_object_get (enc,
      "low-latency-mode",    &llm,
      "slice-intra-refresh", &sir,
      "idr-period",          &idr,
      "bitrate",             &bps,
      NULL);

  ck_assert_int_eq (llm, TRUE);
  ck_assert_uint_eq (sir, 4);
  ck_assert_uint_eq (idr, 0);
  ck_assert_uint_eq (bps, 8000000);

  gst_object_unref (enc);
}
GST_END_TEST;

Suite *
v4l2enc_suite (void)
{
  Suite *s = suite_create ("v4l2h265enc");

  TCase *tc_basic = tcase_create ("properties");
  tcase_add_test (tc_basic, test_bitrate_property);
  tcase_add_test (tc_basic, test_gop_size_property);
  tcase_add_test (tc_basic, test_default_values);
  suite_add_tcase (s, tc_basic);

  TCase *tc_sir = tcase_create ("slice-intra-refresh");
  tcase_add_test (tc_sir, test_slice_intra_refresh_default);
  tcase_add_test (tc_sir, test_slice_intra_refresh_set);
  tcase_add_test (tc_sir, test_slice_intra_refresh_max_boundary);
  suite_add_tcase (s, tc_sir);

  TCase *tc_llm = tcase_create ("low-latency-mode");
  tcase_add_test (tc_llm, test_low_latency_mode_default);
  tcase_add_test (tc_llm, test_low_latency_mode_enable);
  tcase_add_test (tc_llm, test_low_latency_mode_roundtrip);
  suite_add_tcase (s, tc_llm);

  TCase *tc_integ = tcase_create ("integration");
  tcase_add_test (tc_integ, test_low_latency_combined_config);
  suite_add_tcase (s, tc_integ);

  return s;
}

GST_CHECK_MAIN (v4l2enc);
