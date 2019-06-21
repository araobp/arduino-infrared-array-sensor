# arduino-infrared-array-sensor

(Work in progress)

## Development environment

- Arduino IDE on RasPi 3.
- vi and g++ on RasPi 3.
- OpenCV3 for thermography GUI development.

## Architecture

```
    [GUI/RasPi3]/dev/ttyACMX----VCP/USB----[Arduino]----I2C----[AMG8833]
```

## Code

- [Arduino](./arduino)
- [RasPi](./raspi)

