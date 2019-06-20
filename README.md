# arduino-infrared-array-sensor

(Work in progress)

## Background

I developed my original AMG8833 arduino shield, and run it on a STM32 NUCLEO board in [this project](https://github.com/araobp/stm32-mcu/tree/master/NUCLEO-F401RE/Thermography).

But the following problems remain:
- The GUI based on matplotlib(Python) is so slow that it cannot playback the images at 10fps.
- The new IDE for STM32 NUCLEO boards are still unstable.

I have started learning OpenCV on RasPi lately, so I want to develop GUI based on OpenCV for the arduino shield. I am also very curious on how fast OpenCV-based GUI can run.

I also developed "rock, scissors and paper" classification based on X-CUBE-AI from STMicro in the project above. I want to compare its infarence accuracy with some other classical methods.

## Goal

- My original AMG8833 arduino transfers 8x8 pixel images to RasPi 3 via its USB-serial.
- RasPi 3 captures 8x8 pixel image frames at 10fps via its USB-serial.
- An OpenCV-based video player on RasPi 3 shows the image frames at 10fps.
- Interpolation is applied to the images to increase its resolution (32x32).
- Some methods based on OpenCV is applied to the images to classify hand gestures, like rock, scissors and paper.

## The benefits

**All-in-one and very cheap development environment: Arduino IDE, g++, make, OpenCV and all the others run on RasPi 3. This is greate!**



