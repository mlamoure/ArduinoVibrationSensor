This code for an Arduino Uno R3 will compile a JSON message and send the message via Zigbee Xbee to a Coordinator.  I've also contributed a set of coordinator code, which will show how to receive the message and then do something with it.

Configuration
---
Make sure you set your configuration appropriately, here are the variables and explanation:

bool publishRawResults = false;  // if set to true, will publish the x, y, z raw values in the JSON message.  Not very useful.
int thresholdVibration = 3; // amount of vibration that needs to happen on any axis for the vibration sensor to trigger.
int thresholdIterations = 10; // number of iterations (at the refreshRate) that the vibrations need to be detected (or not) before the Zigbee message is published about a status change.
int refreshRate = 3000; // in MS, the amount of time between iterations
String deviceID = "dryer"; // the name of the device, for use in the JSON object that is published in the Zigbee payload

Requirements
---
Hardware: ADXL335, XBee (I use the PRO S28), Arduino, XBee Shield from Sparkfun or otherwise.
Software: The xbee-arduino library that can be downloaded here: https://code.google.com/p/xbee-arduino/

You might want to check out my other repositories for my Zigbee Controller to receive the message.