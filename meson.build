project('qrb5-multimedia-sdk', 'c',
  version: '2.4.0-dev',
  license: 'proprietary',
  default_options: ['c_std=gnu11', 'warning_level=2'],
)

gst_dep = dependency('gstreamer-1.0', version: '>= 1.20')
gstvideo_dep = dependency('gstreamer-video-1.0', version: '>= 1.20')
v4l2_dep = dependency('libv4l2', required: false)

plugins_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

subdir('src/gst/v4l2')
