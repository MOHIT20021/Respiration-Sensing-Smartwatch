### We are using Waveshare ST7789v 320x240 tft Display and Waveshare Mlx90640 thermal camera. you might need to change the code if you use other components.

We used teensy 4.0 and for better framerate from thermal camera we increased the I2C communication speed to 1MHz,also we increased the spi speed to 60Mhz for smooth video output on lcd display. For teensy to preform better we overclocked the teensy to 816Mhz.


The terminal which dont have any connection in wiring diagram dont connect them as code doesn't account for them.

In the code we are printing 5 as a random it has no significant.

If need to connect the Battery for powering teensy we used 3.7v 450mAh lipo battery and connect to the terminal shown in the diagram.

