# World Camera Sample App

This sample demonstrates how to preview and configure live feed from world camera(s).

## Prerequisites

Refer to https://developer.magicleap.cloud/learn/docs/guides/native/capi-getting-started

## Gui
  - A Console GUI provides control of the 3 world camera(s) (left, center and right) and two different exposure modes (low and normal)
  - The Console GUI also provides information about each frame as it is processed

## Running on device

```sh
adb install ./app/build/outputs/apk/ml2/debug/com.magicleap.capi.sample.world_camera-debug.apk
adb shell am start -a android.intent.action.MAIN -n com.magicleap.capi.sample.world_camera/android.app.NativeActivity
```

## Removing from device

```sh
adb uninstall com.magicleap.capi.sample.world_camera
```

## What to Expect

 - 3 world camera(s) feeds with low exposure, and 3 world camera(s) feeds with normal exposure should be visible
 - GUI Console displaying world camera(s) metadata and configuration settings for the application