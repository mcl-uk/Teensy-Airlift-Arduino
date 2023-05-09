# Teensy-Airlift-Arduino
A hacked Arduino library for Teensy4.x and Airlift WiFi module which is better able to serve large (>1MB) files over http from a WiFi AccessPoint.
Plus an example sketch which uses the library to serve a jpg image
and an example wiring diagram , more info in the sketch.

Note there still seem to be problems serving large data blocks using this library: transfer speeds are very variable and the TCP connection will occasionaly fail, maybe someone can help improve this lib.  
