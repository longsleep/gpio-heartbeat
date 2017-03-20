# GPIO heartbeat

This module provides a continuous heartbeat signal on a GPIO pin. It can be
used as optical signal that the Kernel is alive, by connecting an LED to the
GPIO pin.

Whenever the module is loaded, the GPIO pin periodically toggles on/off
simulating a heartbeat like pluse signal.

## Compile

This module is developed as an out of tree module and can be compiled with an
existing Kernel source tree or the corresponding headers.

```
make
```

This builds the module for the currently booted (uname -r) Kernel. To install
run `make install`.

## Device tree support

The module can be used as a platform driver with automated early loading using
a device tree entry like the following.

```
    gpio-heartbeat {
        compatible = "gpio-heartbeat";
        status = "okay";
    };
```

## License

This module uses the same license as the Linux Kernel. See `COPYING` for details.
