# Qualcomm Robotics RB5 — Multimedia SDK

GStreamer V4L2 encoder/decoder plugins for the Qualcomm Robotics RB5 (QRB5165) platform.

## Overview

This SDK provides hardware-accelerated video encoding and decoding elements that integrate with the GStreamer multimedia framework. The plugins leverage the V4L2 (Video4Linux2) kernel interface to access Qualcomm's dedicated video processing hardware.

## Supported Codecs

| Element | Codec | Direction |
|---------|-------|-----------|
| `v4l2h265enc` | H.265/HEVC | Encode |
| `v4l2h264enc` | H.264/AVC | Encode |
| `v4l2h265dec` | H.265/HEVC | Decode |
| `v4l2h264dec` | H.264/AVC | Decode |

## Platform Support

- Qualcomm Robotics RB5 (QRB5165)
- Qualcomm Robotics RB3 Gen 2
- Snapdragon Ride (select configurations)

## Building

```bash
meson setup build/ --prefix=/usr
ninja -C build/
ninja -C build/ install
```

## Branch Strategy

- `main` — stable release baseline
- `rb5-bsp-v2.4-dev` — active development for BSP v2.4
- `rb5-bsp-v2.3-release` — frozen release branch

## License

Qualcomm Proprietary. See LICENSE for details.
