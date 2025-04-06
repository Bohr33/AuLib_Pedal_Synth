# AuLib Pedal Synth

### Overview

This is a MIDI synthesizer device created using the AuLib library. To use this instrument, you must have this library installed. 

This instrument features 8 voices, each of which control four oscillators with "pedal" control, allowing you to increase the frequency of the oscillators in steps of 5ths. As well as this, there is control to detune the oscillators. Finally, there is a master delay with gain controls.

### Usage
A MAKE file is provided to build the project, however the MAKE file assumes the AuLib library is installed on your machine, and you are using a Mac with arm64 architecture. If not, change the MAKE file for your needs.

Once the program is built, run the program with a MIDI device plugged in. You will be prompted to select your device from available devices.

### Controls
Each of the four oscillator pedals can be controlled with MIDI CC values, as well as the detune amount and delay. The MIDI CC values are hard coded in the .cpp file, but these can be changed easily within the main program.

The current values are...

- Pedal 1 : 		CC 22
- Pedal 2 : 		CC 23
- Pedal 3 : 		CC 24
- Pedal 4 :		CC 25
- Detune  :		CC 26
- Delay Gain : 	CC 27

