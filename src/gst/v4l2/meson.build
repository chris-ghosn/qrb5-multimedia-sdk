v4l2_sources = [
  'gstv4l2videoenc.c',
  'gstv4l2videodec.c',
  'gstv4l2object.c',
  'gstv4l2bufferpool.c',
]

v4l2_headers = [
  'gstv4l2videoenc.h',
  'gstv4l2videodec.h',
  'gstv4l2object.h',
  'gstv4l2bufferpool.h',
]

gstv4l2 = shared_library('gstv4l2qrb5',
  v4l2_sources,
  dependencies: [gst_dep, gstvideo_dep, v4l2_dep],
  install: true,
  install_dir: plugins_dir,
)
