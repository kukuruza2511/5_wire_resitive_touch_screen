# 5-Wire Resistive Touchscreen Library for STM32

This is a driver library for interfacing a **5-wire resistive touchscreen** with STM32 microcontrollers.  
The 5-wire touchscreen technology uses five connection lines to detect the location of a touch input. This library supports raw ADC reading, pin switching between idle and sensing mode, and position mapping after calibration.

![5 wire resitive touch screen pinout](https://europe1.discourse-cdn.com/arduino/original/4X/e/a/1/ea1a0486fc4d8ee15b80b2ed888aeded4aeb940f.gif)

A 5-wire resistive touchscreen consists of two conductive layers:
- The **bottom layer** is fixed and connected to 4 corners (UL, UR, LL, LR)
- The **top layer** acts as a flexible probe (common line)
When touched, the top layer contacts the bottom layer at a point, forming a voltage divider. By alternately powering the X and Y axes and sensing the voltage, the position of the touch can be calculated.
To detect touch positions, the library dynamically switches the GPIO pin configuration between two modes:
- **Idle Mode**: All touch pins are in high-impedance or grounded state to avoid false readings.
- **Read Mode**: Two opposing corners are powered to create a voltage gradient along the X or Y axis. The center sense pin is read via ADC to determine the touch position.

To use library
## 1. Add the library to your project

Copy all touch_5wire.c and touch_5wire.h files into your STM32 project and include the header in main.c.

<pre>#include "touch_5wire.h"</pre>

## 2. Declare variables and screen dimensions
Initialize a variable of type TOUCH_5WIRE_t for your screen and declare the length and width boundaries of your screen. For example:
<pre> TOUCH_5WIRE_t Ts;
uint16_t length = 248;
uint16_t width = 186;
</pre>
## 3. Initialize the touchscreen with your GPIO and ADC setup
<pre> touch_5wire_init(&Ts,
		  &hadc1, ADC_CHANNEL_8,
		  GPIOB, GPIO_PIN_0,
		  GPIOE, LR_Pin,
		  GPIOE, LL_Pin,
		  GPIOE, UR_Pin,
		  GPIOE, UL_Pin);
</pre>
## 4. Calibrate the panel
Calibrate by running the get_raw_value_x() and get_raw_value_y() functions in the touch_5wire.h file and measure the maximum and minimum ADC values (at the 4 corners of the screen) for each x and y axis and record them to any variable (since the screens have their own internal resistors, each screen these values will be different so you need to determine these values to map the touch position (Or you can do it another way if you think of a better method :))) ).
For example, here are the boundary values I measured for my display:
<pre>uint16_t min_adc_x = 1168;
uint16_t max_adc_x = 2933;
uint16_t min_adc_y = 1208;
uint16_t max_adc_y = 2985;
</pre>
## 5. Detect touch and read position
Check for touch events using the touch_detect() function, if it returns 1 then configure the sense pin to ADC and read the touch position. For example:
<pre>while (1)
{
    if (touch_detect(&Ts)) {
        set_to_adc(&Ts);  // switch pin config
        HAL_Delay(10);    // small delay for stability
        position_x = get_position_x(&Ts, min_adc_x, max_adc_x, length);
        position_y = get_position_y(&Ts, min_adc_y, max_adc_y, width);
        // Use (position_x, position_y)
    }
}</pre>
