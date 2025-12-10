Software for multi-channel-pipette

Load "Main.ino" file onto arduino, it will automatically include the "MotorDrivers.ino" script if it is in the same folder.  

NOTE: The rotary encoder uses digital pins D0 and D1, which are typically reserved for serial. This may interfere with subsequent uploads. 
If so, either (1) scroll the rotary encoder 1-3 ticks and try again OR (2) Disconnect encoder during upload OR (3) adjust code to use A0 and A2 insteald of D0 and D1
