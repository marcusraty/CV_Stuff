# System Notifications Manager Sample App

This sample demonstrates how system notifications can be suppressed and a few examples of how developers can implement their own.
The Sample Notification Manager will only work with certain device SKUs, specifically the call to API MLSystemNotificationManagerSetNotifications will return MLResult_IncompatibleSKU on incompatible devices.

## Prerequisites

Refer to https://developer.magicleap.cloud/learn/docs/guides/native/capi-getting-started

## Gui
  - GUI provides control to suppress or enable the system notifications
  - GUI provides information about the current system status, including a stream of messages of the events occured.

## Running on device

```sh
adb install ./app/build/outputs/apk/ml2/debug/com.magicleap.capi.sample.system_notifications-debug.apk
adb shell am start -a android.intent.action.MAIN -n com.magicleap.capi.sample.system_notifications/android.app.NativeActivity
```

## Removing from device

```sh
adb uninstall com.magicleap.capi.sample.system_notifications
```

## What to Expect

 - When the checkbox to suppress system notifications is selected, there will be no system notifications, dialogs or alerts (such as head tracking lost).
 - GUI Console will continue to display system events even when system notifications are suppressed.

 The following system events will be reported in the GUI:
 - Connection to wifi.
 - Compute Pack battery charge is under 5%.
 - Controller battery charge is under 5%.
 - Available space (internal and external storage) is under 10%.
 - Volume is increased above 75%.
 - Compute Pack battery temperature is above 40 degrees Celsius.
 - Head tracking lost.
 - System memory low (see: https://developer.android.com/reference/android/content/ComponentCallbacks2#TRIM_MEMORY_RUNNING_LOW. Low memory conditions can be tested with the following command: ```adb shell am send-trim-memory com.magicleap.capi.sample.system_notifications 15```).