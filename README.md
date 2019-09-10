# oscbox
Encoders for the ETC EOS family of lighting consoles using OSC and an Orange Pi Zero

## Overview
The purpose of this project is to create encoder wheels for the ETC EOS family of lighting software.
Specifically, the target is a PC or Mac running the EOS software.
It's possible this would work with a physical lighting console but it hasn't been tested.

The project was inspired by ETC's "lighthack" project.  
Their "box_1" implements two encoder wheels, one for PAN and one for TILT.
I wanted the same, but I wanted four wheels to match the wheels on an Ion console, 
and I wanted the wheels to be multi-function like the console wheels are.

## Hardware
I decided to go with the "Pi" family of single board computers rather than the Arduino family.
I find the Pis to be easy to powerful, easy to program, and not much more expensive that Arduinos.
I used an [Orange Pi Zero from Aliexpress](https://www.aliexpress.com/item/32761500374.html).
I liked the fact that it had Ethernet as well as WiFi built in.
You could probably also build this project with a Raspberry Pi Zero W, or a Raspberry Pi Zero, or even a full Raspberry Pi 3.
Really what you need are:
* A card that can run some flavor of Linux
* GPIO pins in the standard 2x13 format of the Raspberry Pi
* A USB port

There are two main pieces that the processor has to talk to - the display and the encoders.
The display is a 3.5 inch touchscreen that plugs straight onto the GPIO pins of the Pi.
The encoders use a GPIO expander board from Adafruit which connects to a custom PC board with the encoders mounted to it.

**TODO:  Separate page with parts list and instructions**
 
## Code
**TODO:  Create a separate page that describes the code**

## Enclosure
**TODO:  Create a separate page that describes the enclosure**
