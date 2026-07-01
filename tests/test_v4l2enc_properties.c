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

Suite *
v4l2enc_suite (void)
{
  Suite *s = suite_create ("v4l2h265enc");
  TCase *tc = tcase_create ("properties");
  tcase_add_test (tc, test_bitrate_property);
  tcase_add_test (tc, test_gop_size_property);
  tcase_add_test (tc, test_default_values);
  suite_add_tcase (s, tc);
  return s;
}

GST_CHECK_MAIN (v4l2enc);
