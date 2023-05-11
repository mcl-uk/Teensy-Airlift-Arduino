# Teensy-Airlift-Arduino
IMHO the ESP32-Airlift WiFi/BLE module with its WiFiNINA library and NINA-W102 firmware (on Arduino) is not fit for purpose, it is incapable of reliably serving large chunks of data (eg a 2MB jpg) at remotely useable data-rates.
I now use the ATWINC1500 solution which far out-performs Airlift in terms of link reliability, data-rate, power consumption and size.
In my application, the ATWINC reliably trasmits data at over 800KB/s whereas the Airlift can manage only 20-30KB/s, while this can be increased by carefully tweaking the library source code it is always at the cost of link reliability.
