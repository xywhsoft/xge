# XGE iOS Sokol Scaffold

This folder contains the first iOS/Sokol integration scaffold for XGE.

Current scope:

- Uses Sokol's iOS entry path through `sokol_main`.
- Uses OpenGL ES 3 through `SOKOL_GLES3`.
- Initializes XGE and runs a minimal clear-color frame callback.
- Avoids Metal in the first baseline, matching the XGE V2 compatibility decision.

Build the simulator MVP on macOS with Xcode command line tools:

```sh
./build_ios_sim_exe.sh
```

Optional environment variables:

```sh
IOS_SDK=iphonesimulator
IOS_ARCH=arm64
IOS_DEPLOYMENT_TARGET=12.0
```

The generated bundle is `build/ios_sim/XGE.app`.

Install and launch on a booted simulator:

```sh
./run_ios_sim_exe.sh
```

Optional environment variables:

```sh
IOS_SIMULATOR=booted
IOS_APP_DIR=build/ios_sim/XGE.app
IOS_BUNDLE_ID=dev.xge.mvp
```

This scaffold is not marked as iOS support complete until it is built and smoke-tested on an iOS simulator or device.
