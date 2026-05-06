This code will not work "out of the box" because flex sensor values are unique to every user's hand and circuit configuration. You must perform a calibration step before the gesture recognition logic will function correctly.

1. Calibrating Flex Sensor Thresholds
The code relies on two sets of variables for the Thumb, Index, Pinky, and Middle fingers:

reg_values (Relaxed): The analog value read when your fingers are completely straight.

Bend_values (Curled): The analog value read when your fingers are fully bent into a fist.

To calibrate:

Open the Serial Monitor in the Arduino IDE while wearing the glove.

Note the values displayed for each finger when your hand is open vs. when it is closed.

Update the constants in the top section of the code with your specific numbers.

2. Resistor Selection & Sensitivity
3. 
10kΩ Resistors: Provide a standard range of motion.
52kΩ Resistors: Offer a much higher "voltage swing." Higher resistance makes the system significantly more sensitive, resulting in a larger delta between a straight and bent finger, but it requires much tighter thresholding in the code.
