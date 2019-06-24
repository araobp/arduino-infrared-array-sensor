# Arduino with infrared array sensor

<img src="doc/shield.jpg" width=300>

AMG8833 is an infrared array sensor product from Panasonic. It is very popular among Arduino users.

## Development environment

- Arduino IDE on RasPi 3.
- vi and g++ on RasPi 3.
- OpenCV3 for thermography GUI development.

Note: you have to install OpenCV3 on Raspi3. In my case, I built OpenCV3 on RasPi3 taking a half day.

## Architecture

```
    [GUI/RasPi3]/dev/ttyACM0----VCP/USB----[Arduino]----I2C----[AMG8833]
```
## Data frame format (raster-scan 8x8 pixel image) over VCP/USB

```
   [BEGIN(0xFE)][byte#0]...[byte#63][END(0xFF)]
```
## Arduino shield of AMG8833

==>[schematic](./kicad)

Note: the shield is powered by 3V3 pin on Arduino UNO. Although Arduino UNO is a 5V system, the circuit works.

## Code

- [Arduino](./arduino)
- [RasPi](./raspi)

## Building and running GUI

<img src="./doc/this_is_me.png" width=200>

This is me!

```
$ cd raspi
$ make
$ bin/thermo -m 64 -t -b
```

GUI developed in a native language (C/C++) runs fast on RasPi 3!

### Bicubic interpolation

The resolution of AMG8833 is only 8x8 pixels. I applied bicubic interpolation to the original 8x8 pixel image for higher resolution.

<img src="./doc/bicubic_interpolation.png" width=200>

This is my right hand.

```
$ bin/thermo -m 1 -i 3
```

With "-H" option, the GUI uses COLORMAP_HOT instead COLORMAP_JET:

<img src="./doc/hot.png" width=200>

```
$ bin/thermo -m 3 -i 2 -H -b
```

### Binalization

The GUI supports binalization:

<img src="./doc/binalization.png" width=200>

```
$ bin/thermo -m 32 -H -B
```

### Diff between frames: gradient(=velocity)

The GUI supports image diff between frames.

<img src="./doc/diff.png" width=200>

Waving my hand over the sensor.

```
$ bin/thermo -m 32 -d
```
