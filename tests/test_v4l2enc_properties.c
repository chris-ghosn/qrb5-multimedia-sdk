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

/* -------------------------------------------------------------------------
 * New tests: slice-intra-refresh and low-latency-mode (QRB5-1042)
 * ------------------------------------------------------------------------- */

GST_START_TEST (test_slice_intra_refresh_default)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  guint sir;
  g_object_get (enc, "slice-intra-refresh", &sir, NULL);
  /* Default must be 0 (disabled) */
  ck_assert_uint_eq (sir, 0);

  gst_object_unref (enc);
}
GST_END_TEST;

GST_START_TEST (test_slice_intra_refresh_roundtrip)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  /* Recommended value for Skydio X10 5G streaming */
  g_object_set (enc, "slice-intra-refresh", 30, NULL);
  guint sir;
  g_object_get (enc, "slice-intra-refresh", &sir, NULL);
  ck_assert_uint_eq (sir, 30);

  /* Boundary: maximum value */
  g_object_set (enc, "slice-intra-refresh", 255, NULL);
  g_object_get (enc, "slice-intra-refresh", &sir, NULL);
  ck_assert_uint_eq (sir, 255);

  /* Disable by setting back to 0 */
  g_object_set (enc, "slice-intra-refresh", 0, NULL);
  g_object_get (enc, "slice-intra-refresh", &sir, NULL);
  ck_assert_uint_eq (sir, 0);

  gst_object_unref (enc);
}
GST_END_TEST;

GST_START_TEST (test_low_latency_mode_default)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  gboolean llm;
  g_object_get (enc, "low-latency-mode", &llm, NULL);
  /* Default must be FALSE */
  ck_assert_int_eq (llm, FALSE);

  gst_object_unref (enc);
}
GST_END_TEST;

GST_START_TEST (test_low_latency_mode_roundtrip)
{
  GstElement *enc = gst_element_factory_make ("v4l2h265enc", NULL);
  g_assert_nonnull (enc);

  g_object_set (enc, "low-latency-mode", TRUE, NULL);
  gboolean llm;
  g_object_get (enc, "low-latency-mode", &llm, NULL);
  ck_assert_int_eq (llm, TRUE);

  g_object_set (enc, "low-latency-mode", FALSE, NULL);
  g_object_get (enc, "low-latency-mode", &llm, NULL);
  ck_assert_int_eq (llm, FALSE);

  gst_object_unref (enc);
}
GST_END_TEST;

Suite *
v4l2enc_suite (void)
{
  Suite *s = suite_create ("v4l2h265enc");
  TCase *tc = tcase_create ("properties");
  tcase_add_test (tc, test_bitrate_property);
  tcase_add_test (tc, test_gop_size_property);
  tcase_add_test (tc, test_default_values);
  tcase_add_test (tc, test_slice_intra_refresh_default);
  tcase_add_test (tc, test_slice_intra_refresh_roundtrip);
  tcase_add_test (tc, test_low_latency_mode_default);
  tcase_add_test (tc, test_low_latency_mode_roundtrip);
  suite_add_tcase (s, tc);
  return s;
}

GST_CHECK_MAIN (v4l2enc);
