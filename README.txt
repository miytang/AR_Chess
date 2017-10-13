3D_Projection:
The chess pieces are stored in the /3D_Projection/Chess_Pieces folders
When running this program, please use OpenCV's camera callibration program to 
callibrate your own camera and include it in the source directory. 
Change the camera driver location and camera paramters location in lines 34 and 35.
Make sure your camera can support a resolution of 1400 x 1050, and if it cannot, change 
the window's resolution in lines 503 and 504.

Object_Detection_and_Tracking:
Like the 3D_Projection, make sure you have your own camera params set up before using.
Also, you must change the camera driver used in lines 193 and 186.

SerialTest.ino:
This is the program the Arduino Uno and Braccio run off of. It is the communication program between 
a computer and the Uno and follows the following format:
"180b90e45w100gz"
The numbers represent the degrees to turn a motor to while the character following each number represents the
specific motor. b = base, e = elbow, s = shoulder, w = wrist, v = wrist rotation, g = grip strength. All motors
have a range of 0 - 180 except the grip which has a range of 0 to 100.
