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

GUI developed in a native language (c/c++) runs fast on RasPi 3!

### Bicubic interpolation

The resolution of AMG8833 is only 8x8 pixels. I applied bicubic interpolation to the original 8x8 pixel image for higher resolution.

<img src="./doc/bicubic_interpolation.png" width=200>

```
$ bin/thermo -m 4 -i
```

## Next features to be developed

- Pattern matching to recognize hand gesture etc

