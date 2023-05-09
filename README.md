# Teensy-Airlift-Arduino
A hacked Arduino library for Teensy4.x and Airlift WiFi module which is better able to serve large (>1MB) files over http from a WiFi AccessPoint.
Plus an example sketch which uses the library to serve a jpg image, also an example wiring diagram, there's more info in the sketch.

Note there still seem to be problems serving large data blocks using this library: transfer speeds are maddeningly variable from one run to the next and the TCP connection will occasionaly fail, maybe someone with better skills can help improve this code.  
