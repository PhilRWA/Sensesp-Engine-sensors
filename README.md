Updated version from the excellent original files;

Mostly this just fixes a couple of small things that were stopping my code compiling successfully, the include moving_average.h was the biggest problem,
along with a missing hyphen further down in the code. 

I've also changed the data in the Temp and fuel curve to match the data sheets I could find for my Bukh DV20s temp sender and theoretical fuel consumption, 
but I can't speak for the accuracy of these just yet. I'll update the decription once I've done some actual testing on the boat.

All this has been tested running on an ESP32 devkit C v4 and works great. 

Original description below:

# ESP32-code
Here is the code I'm running on my ESP32 as of Feb 2023

The setup is using examples found on the excellent SensESP pages here - https://github.com/SignalK/SensESP 

I now have engine temperature, exhaust temp, engine RPM, engine room environmental, engine runtime using the ESP uptime and (estimated) fuel consumption which is using the engine RPM and fuel burn info from the Volvo Penta documents. I have recently added a bilge monitor to this project.

Please make sure you add the following additional dependencies to you platformio.ini file, located under 'lib_deps', also check the latest version of these dependencies (Thanks Techstyle). I've uploaded my file as reference.

SensESP/OneWire@^2.0.0 

adafruit/Adafruit BMP280 Library @ ^2.5.0

Videos of my setup can be found on our YouTube channel - https://www.youtube.com/c/BoatingwiththeBaileys

Hope this is helpful and my thanks to all the talented people writing the code and continuing to develop these projects!
