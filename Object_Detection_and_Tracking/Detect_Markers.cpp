/*
 * Detect_Markers.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: t-michael
 */
#include <opencv2/highgui.hpp>
#include <opencv2/aruco/charuco.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv/cxcore.hpp"
#include "opencv2/core.hpp"

#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define BAUDRATE B115200 // Serial Speed
#define MARK_SIZE 0.017

using namespace std;
using namespace cv;
using namespace aruco;

void promptID (int &id, int &row, int &col){
	int move;

	cout << "Which ID do you want to find? ";
	cin >> id;
	cout << "Where do you want to move this piece? ";
	cin >> move;
	row = move/10;
	col = move%10;
}

String writeBraccio (int fd, string input){
	if(write(fd, input.c_str(), strlen(input.c_str())) < 1){
		cout << "Error writing to Braccio!\n";
		exit(0);
	}
	input.clear();
	return input;
}

void reset (int &base, int &shoulder, int &elbow, int &wrist, int &grip){
	    //base = 90;
		elbow = 150;
		wrist = 180;
		shoulder = 90;
		grip = 0;
}

static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
    FileStorage fs(filename, FileStorage::READ);
    if(!fs.isOpened())
        return false;
    fs["camera_matrix"] >> camMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    return true;
}

// Build string to send
string buildString(int base, int shoulder, int elbow, int wrist, int wrist_rot, int grip){
	string input;
	string shoulder_str;
	string elbow_str;
	string base_str;
	string wrist_str;
	string wrist_rot_str;
	string grip_str;

	shoulder_str = to_string(shoulder);
	base_str = to_string(base);
	elbow_str = to_string(elbow);
	wrist_str = to_string(wrist);
	wrist_rot_str = to_string(wrist_rot);
	grip_str = to_string(grip);

	if (shoulder != -1)
	{
		input.append(shoulder_str);
		input.append("s");
	}
	if (base != -1){
		input.append(base_str);
		input.append("b");
	}
	if (wrist != -1){
		input.append(wrist_str);
		input.append("w");
	}
	if (wrist_rot != -1){
		input.append(wrist_rot_str);
		input.append("v");
	}
	if (elbow != -1){
		input.append(elbow_str);
		input.append("e");
	}
	if (grip != -1){
		input.append(grip_str);
		input.append("g");
	}
	input.append("z");

	return input;
}

int main(int argc, char *argv[]){
	bool grabbed = false;
	String input = "r";

	int base = 90;
	int elbow = 150;
	int wrist = 180;
	int shoulder = 90;
	int grip = 0;

	int count = 0;
	int count2 = 0;

	int id;
	int id_i;
	int row, col;

	int board[8][8] = {0};
	int piece = 0;
	for (int i = 0; i < 8; i ++){
		for (int j = 0; j < 8; j ++){
			if (i < 2 || i > 5)
				board[i][j] = piece++;
			cout << board[i][j] << ", ";
		}
		cout << "\n";
	}

	double x = 0;
	double y = 0;
	double z = 0;

	bool up = false;

	struct termios serial; // Structure to contain UART parameters
	char dev_id[] = "/dev/serial/by-id/usb-Arduino_Srl_Arduino_Uno_556353038383517160A1-if00"; // UART device identifier

	printf("Opening %s\n", dev_id);
	int fd = open(dev_id, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1){ // Open failed
		perror(dev_id);
		return -1;
	}

	// Get UART configuration
	if (tcgetattr(fd, &serial) < 0){
		perror("Getting configuration");
		return -1;
	}

	// Set UART parameters in the termios structure
	serial.c_iflag = 0;
	serial.c_oflag = 0;
	serial.c_lflag = 0;
	serial.c_cflag = BAUDRATE | CS8 | CREAD;
	// Speed setting + 8-bit data + Enable RX

	serial.c_cc[VMIN] = 0; // 0 for Nonblocking mode
	serial.c_cc[VTIME] = 0; // 0 for Nonblocking mode

	// Set the parameters by writing the configuration
	tcsetattr(fd, TCSANOW, &serial);

	sleep(2);
	cout << "Setting up Braccio...\n";
	input = writeBraccio(fd, input);
	sleep(2);
	cout << "Braccio Setup.\n";

	// Input video
	VideoCapture cam("/dev/v4l/by-id/usb-Etron_Technology__Inc._USB2.0_Camera-video-index0");
	if (!cam.isOpened()) return -1;
	int width = (int)cam.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = (int)cam.get(CV_CAP_PROP_FRAME_HEIGHT);
	printf("W: %I32d, H: %I32d\n", width, height);

	// Video 2
	VideoCapture cam2("/dev/v4l/by-id/usb-046d_C922_Pro_Stream_Webcam_808791DF-video-index0");
	if (!cam2.isOpened()) return -1;

	// Dictionary of markers
	Ptr<Dictionary> dictionary = getPredefinedDictionary(DICT_4X4_100);

	// Camera Parameters for drawing axis
	Mat camMatrix, distCoeffs;
	bool read = readCameraParameters("cam_cal.yml", camMatrix, distCoeffs);
	if (!read){
		cerr << "Invalid camera file" << endl;
		return 0;
	}

	input = buildString(base, shoulder, elbow, wrist, -1, grip = 60);
	input = writeBraccio(fd, input);

	// Testing how to find specific markers
	promptID(id, row, col);

	while(1){
		Mat image, imageCopy;
		Mat image2, image2Copy;

		cam >> image;
		image.copyTo(imageCopy);

		cam2 >> image2;
		image2.copyTo(image2Copy);

		vector<int> markerIds, markerIds2;
		vector<vector<Point2f>> markerCorners, markerCorners2;

		// Camera two displays all markers and draws lines between them
		detectMarkers(image2, dictionary, markerCorners2, markerIds2);
		if (markerIds2.size() > 0){
			drawDetectedMarkers(image2Copy, markerCorners2, markerIds2);

			// Draw line between two markers
			for (int i = 0; i < markerIds2.size() - 1; i ++){
				Point2f point = markerCorners2[i][0];
				Point2f point2 = markerCorners2[i+1][0];
				line(image2Copy, point, point2, Scalar(rand() % 256, rand() % 256, rand() % 256), 2, 8);
			}

		}

		// Camera one searches for needed marker and points2moves it
		detectMarkers(image, dictionary, markerCorners, markerIds);
		if (markerIds.size() > 0){
			// Stores rotational and translation vectors of camera pos
			vector<Vec3d> rvecs, tvecs;

			// Find ID of user inputed ID
			for (int i = 0; i < markerIds.size(); i ++){
				if (markerIds[i] == id){
					id_i = i;
					break;
				}
				else id_i = -1;
			}

			// Draw detected Markers
			drawDetectedMarkers(imageCopy, markerCorners, markerIds);

			if (id_i >= 0){
				// Draw axis for Markers
				estimatePoseSingleMarkers(markerCorners, MARK_SIZE, camMatrix, distCoeffs, rvecs, tvecs);

				// Draw pose
				drawAxis(imageCopy, camMatrix, distCoeffs, rvecs[id_i], tvecs[id_i], MARK_SIZE);

				Vec3d test = tvecs[id_i];
				x = test[0];
				y = test[1];
				z = test[2];
			}
			else {
				x = 0;
				y = -0.01;
				z = -1;
				count2 ++;

				if (count2 >= 15){
					cout << "SEARCHING FOR CORRECT MARKER\n";
					count2 = 0;
					if (up)
						base += 2;
					else
						base -= 2;
					if (base <= 50){
						base = 50;
						up = true;
					}
					else if (base >= 135){
						base = 180;
						up = false;
					}

					if (shoulder >= 100){
						shoulder = 100;
						elbow = 150;
					}
					input = buildString(base, shoulder, elbow, -1, -1, -1);
					cout << input << "\n";
					input = writeBraccio(fd, input);
				}
			}

			// Move arm every other loop to artificially slow down the transmit rate
			if (count >= 3){
				cout << "X: " << x << "\nY: " << y <<  "\nZ: " << z << "\n";

				count = 0;

				if (grabbed == false){
					// Move x first before moving y
					// Like a crane
					if (x < -MARK_SIZE/6)
					{
						cout << "LEFT\n";
						up = false;
						base --;
					}
					else if (x > MARK_SIZE/6)
					{
						cout << "RIGHT\n";
						up = true;
						base ++;
					}

					if (y < -0.01 - MARK_SIZE/6)
					{
						cout << "UP\n";
						elbow --;
					}
					else if (y > -0.01 + MARK_SIZE/6)
					{
						cout << "DOWN\n";
						elbow ++;
					}

					if (z > 0.1)
					{
						shoulder ++;
					}

					if (z < 0.1 && z > 0)
					{
						grip = 75;
						grabbed = true;
					}
				}

				// Non blocking pick up....
				if (grabbed == true)
				{
					cout << "PICKING UP BOX!!!!\n";
					if (shoulder-- <= 90){
						shoulder = 90;
						if (elbow == 130){
							if (grip-- <= 50){
								grabbed = false;
					}
						}
						else if (elbow < 130)
							elbow ++;
						else if (elbow > 130)
							elbow --;
					}
				}
				cout << "GRABBED?: " << grabbed << "\n";
				// Floor and Ceiling values for base, elbow, and wrist
				if (base <= 0){
					base = 0;
					up = true;
				}
				else if (base >= 180){
					base = 180;
					up = false;
				}

				if (shoulder >= 180)
					shoulder = 180;
				if (shoulder <= 90)
					shoulder = 90;

				if (elbow <= 0)
					elbow = 0;
				else if (elbow >= 180)
					elbow = 180;

				if (wrist <= 0)
					wrist = 0;
				else if (wrist >= 180)
					wrist = 180;

				input = buildString(base, shoulder, elbow, wrist, -1, grip);
				cout << input << "\n";
				// Write to Braccio
				input = writeBraccio(fd, input);
			}
			count ++;
			cout << "\n";
		}
		else{
			cout << "SEARCHING FOR MARKERS\n";
			if (up)
				base ++;
			else
				base --;
			if (base <= 50){
				base = 50;
				up = true;
			}
			else if (base >= 135){
				base = 135;
				up = false;
			}
			input = buildString(base, -1, -1, -1, -1, -1);
			cout << input << "\n";
			input = writeBraccio(fd, input);
		}

		// Draw crosshair in the middle of window
		line (imageCopy, Point(width/2 - 10, height/2), Point(width/2 + 10, height/2), Scalar(rand() % 256, rand() % 256, rand() % 256), 2, 8);
		line (imageCopy, Point(width/2, height/2 - 10), Point(width/2, height/2 + 10), Scalar(rand() % 256, rand() % 256, rand() % 256), 2, 8);
		imshow("out", imageCopy);

		imshow("out2", image2Copy);

		int key = waitKey(10);
		switch(key){
			case 'q':
				exit(1);
			case 27:
				exit(1);
			case 'a':
				promptID(id, row, col);
				break;
			default:
				break;
		}
	}
}


