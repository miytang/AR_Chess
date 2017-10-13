///*
// *  opticalFlow.cpp
// *  OpenCVTries1
// *
// *  Created by Roy Shilkrot on 11/3/10.
// *  Copyright 2010. All rights reserved.
// *
// */
//
#include <vector>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv/cxcore.hpp>
#include <opencv2/core.hpp>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>

#define MARK_SIZE 0.18

using namespace std;
using namespace cv;
using namespace aruco;
using namespace glm;

//String camera = "/dev/v4l/by-id/usb-Etron_Technology__Inc._USB2.0_Camera-video-index0";
String camera = "/dev/v4l/by-id/usb-046d_C922_Pro_Stream_Webcam_808791DF-video-index0";
String param_file = "logicool_params_1400x1050_3.yml";
String out_name = "full_chess_board.avi";
// Black pawns' IDs and indices if found
int black_pawns[8] = {8, 9, 10, 11, 12, 13, 14 ,15};
int black_pawns_i[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
// Black queen and king's IDs and indices if found
int black_queen = 3;
int black_queen_i = -1;
int black_king = 4;
int black_king_i = -1;
// Black rooks' IDs and indices if found
int black_rooks[2] = {0, 7};
int black_rooks_i[2] = {-1, -1};
// Black knights' IDs and indices if found
int black_knights[2] = {1, 6};
int black_knights_i[2] = {-1, -1};
// Black bishops' IDs and indices if found
int black_bishops[2] = {2, 5};
int black_bishops_i[2] = {-1, -1};

// White pawns' IDs and indices if found
int white_pawns[8] = {16, 17, 18, 19, 20, 21, 22, 23};
int white_pawns_i[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
// White queen and king's IDs and indices if found
int white_queen = 27;
int white_queen_i = -1;
int white_king = 28;
int white_king_i = -1;
// White rooks' IDs and indices if found
int white_rooks[2] = {24, 31};
int white_rooks_i[2] = {-1, -1};
// White knights' IDs and indices if found
int white_knights[2] = {25, 30};
int white_knights_i[2] = {-1, -1};
// White bishops' IDs and indices if found
int white_bishops[2] = {26, 29};
int white_bishops_i[2] = {-1, -1};

// For displaying functions
GLuint textID;
float width;
float height;

// For chess pieces
// Each pieces' extrinsic matrix
GLfloat black_pawns_glMatrix[8][16];
GLfloat black_rooks_glMatrix[8][16];
GLfloat black_knights_glMatrix[8][16];
GLfloat black_bishops_glMatrix[8][16];
GLfloat black_queen_glMatrix[16];
GLfloat black_king_glMatrix[16];

GLfloat white_pawns_glMatrix[8][16];
GLfloat white_rooks_glMatrix[8][16];
GLfloat white_knights_glMatrix[8][16];
GLfloat white_bishops_glMatrix[8][16];
GLfloat white_queen_glMatrix[16];
GLfloat white_king_glMatrix[16];

// Each pieces' vertex values
vector<vec3> pawn;
vector<vec3> pawn_norms;
vector<vec3> knight;
vector<vec3> knight_norms;
vector<vec3> rook;
vector<vec3> rook_norms;
vector<vec3> bishop;
vector<vec3> bishop_norms;
vector<vec3> king;
vector<vec3> king_norms;
vector<vec3> queen;
vector<vec3> queen_norms;

static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
    FileStorage fs(filename, FileStorage::READ);
    if(!fs.isOpened())
        return false;
    fs["camera_matrix"] >> camMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    return true;
}

void loadGLArrayOfMatrix (int size, int id_i[], GLfloat (&glMatrix)[8][16], vector<Vec3d> r_vecs, vector<Vec3d> t_vecs){
	for (int i = 0; i < size; i ++){
		if (id_i[i] >= 0){
			Mat rot_3d(3,3,CV_32FC1);
			Vec3d temp_rot = r_vecs[id_i[i]];

			// Flip x and y rotation... I think
			//temp_rot[0] = -temp_rot[0];
			//temp_rot[1] = -temp_rot[1];

			// Apply Rodrigues to convert from 2d rotational vector to 3d matrix
			Rodrigues(temp_rot, rot_3d);

			// Place rotation matrix into the correct transformation matrix positions
			for (int j = 0; j < 3; j ++) {
				for (int k = 0; k < 3; k ++){
					glMatrix[i][j*4+k] = -rot_3d.at<double>(j,k);
				}
			}

			// Place translation vector into correct transformation matrix positions
			Vec3d temp_vec = t_vecs[id_i[i]];

			double x = temp_vec[0];
			double y = -temp_vec[1]; // -y since OpenCV and OpenGL have opposite y directions
			double z = -temp_vec[2]; // Same with z

			glMatrix[i][12] = x;
			glMatrix[i][13] = y;
			glMatrix[i][14] = z;
		}
	}
}

void loadGLMatrix(int id_i, GLfloat(& glMatrix)[16], vector<Vec3d> r_vecs, vector<Vec3d> t_vecs){
	if (id_i >= 0){
		Mat rot_3d(3,3,CV_32FC1);
		Vec3d temp_rot = r_vecs[id_i];

		//temp_rot[0] = -temp_rot[0];
		//temp_rot[1] = -temp_rot[1];

		Rodrigues(temp_rot, rot_3d);

		// Place into glMatrix
		for (int i = 0; i < 3; i ++) {
			for (int j = 0; j < 3; j ++){
				glMatrix[i*4+j] = -rot_3d.at<double>(i,j);
			}
		}
		// Translating
		Vec3d test = t_vecs[id_i];
		double x = test[0];
		double y = -test[1];
		double z = -test[2];

		glMatrix[12] = x;
		glMatrix[13] = y;
		glMatrix[14] = z;
	}
}

// Reset array indices for each piece
// This is so that pieces do not stay
// on image when its respective marker
// ID cannot be found
void resetIndices(){
	for (int i = 0; i < 8; i ++){
		black_pawns_i[i] = -1;
		white_pawns_i[i] = -1;
	}
	for (int i = 0; i < 2; i ++){
		black_rooks_i[i] = -1;
		black_bishops_i[i] = -1;
		black_knights_i[i] = -1;
		white_rooks_i[i] = -1;
		white_bishops_i[i] = -1;
		white_knights_i[i] = -1;
	}
	black_king_i = -1;
	white_king_i = -1;
	black_queen_i = -1;
	white_queen_i = -1;
}

// Sets up the extrinsic matrices
void initMatrices(){
	for (int i = 0; i < 16; i ++){
		if (i == 0 || i == 5 || i == 10 || i == 15){
			black_queen_glMatrix[i] = 1;
			black_king_glMatrix[i] = 1;
			white_queen_glMatrix[i] = 1;
			white_king_glMatrix[i] = 1;
		}
		else{
			black_queen_glMatrix[i] = 0;
			black_king_glMatrix[i] = 0;
			white_queen_glMatrix[i] = 0;
			white_king_glMatrix[i] = 0;
		}
	}

	for (int i = 0; i < 8; i ++){
		for(int j = 0; j < 16; j ++){
			if (j == 0 || j == 5 || j == 10 || j == 15){
				black_pawns_glMatrix[i][j] = 1;
				black_rooks_glMatrix[i][j] = 1;
				black_knights_glMatrix[i][j] = 1;
				black_bishops_glMatrix[i][j] = 1;

				white_pawns_glMatrix[i][j] = 1;
				white_rooks_glMatrix[i][j] = 1;
				white_knights_glMatrix[i][j] = 1;
				white_bishops_glMatrix[i][j] = 1;
			}
			else{
				black_pawns_glMatrix[i][j] = 0;
				black_rooks_glMatrix[i][j] = 0;
				black_knights_glMatrix[i][j] = 0;
				black_bishops_glMatrix[i][j] = 0;

				white_pawns_glMatrix[i][j] = 0;
				white_rooks_glMatrix[i][j] = 0;
				white_knights_glMatrix[i][j] = 0;
				white_bishops_glMatrix[i][j] = 0;
			}
		}
	}
}

// Draw the background image
// In our case, it is the camera
// feed from OpenCV
void drawBackground(){
	// Start Ortho
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw 2D background
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textID);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// End Ortho
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat) width / (GLfloat) height, 0.1f, 100.0f);
}

// Draws one chess piece
// used for drawing the Kings and Queens
void drawPiece(int id_i, GLfloat glMatrix[16], vector<vec3> vecs, vec3 color) {
	glColor3f(color.x, color.y, color.z);

	if(id_i >= 0){
		glLoadIdentity();
		glLoadMatrixf(glMatrix);
		glBegin(GL_TRIANGLES);
		for(int i = 0; i < vecs.size(); i ++){
			glVertex3f(vecs[i].x, vecs[i].y, vecs[i].z);
		}
		glEnd();

		glLineWidth(1.5f);
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i = 0; i < vecs.size(); i ++){
			glVertex3f(vecs[i].x, vecs[i].y, vecs[i].z);
		}
		glEnd();
	}
}

// Draws multiple pieces at once
// used for drawing the pawns, knights, rooks, and bishops
// ele = number of elements to draw
// id_i[] = array of each piece's index storing whether the piece has been spotted or not
// glMatrix[][16] = the array of matrices that contain the extrinsic matrix values for transformation
// vecs = vector of triangle points to draw actual piece
// color = color used to glColor3f
void drawPieces(int ele, int id_i[], GLfloat glMatrix[][16], vector<vec3> vecs, vec3 color) {
	for (int i = 0; i < ele; i ++){
		if(id_i[i] >= 0){
			/*for (int j = 0; j < 16; j ++){
				if (j % 4 == 0)
					cout << endl;
				cout << glMatrix[i][j] << ", ";
			}
			cout << "=============================================" << endl;*/
			glColor3f(color.x, color.y, color.z);
			glLoadIdentity();
			glLoadMatrixf(glMatrix[i]);
			glBegin(GL_TRIANGLES);
				for(int j = 0; j < vecs.size(); j ++)
					glVertex3f(vecs[j].x, vecs[j].y, vecs[j].z);
			glEnd();

			glLineWidth(1.5f);
			glColor3f(0.0f, 0.0f, 0.0f);
			glBegin(GL_LINES);
			for (int j = 0; j < vecs.size(); j ++){
				glVertex3f(vecs[j].x, vecs[j].y, vecs[j].z);
			}
			glEnd();
		}
	}
}

// Load vertex values of object
bool loadOBJ(const char * path, vector<vec3> & out_vertices, vector<vec3> & out_normals) {
	printf("Loading OBJ file %s...\n", path);

	vector<unsigned int> vertexIndices, normalIndices;
	vector<vec3> temp_vertices;
	vector<vec3> temp_normals;

	FILE * file = fopen(path, "r");
	if( file == NULL ){
		cout << "Cannot Open:" << path << endl;
		return false;
	}

	while(1){
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if ( strcmp( lineHeader, "v" ) == 0 ){
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			unsigned int vertexIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0],  &normalIndex[0], &vertexIndex[1],  &normalIndex[1], &vertexIndex[2],  &normalIndex[2]);

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){
		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the indexblack_pawns
		vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_normals.push_back(normal);
	}

	return true;
}

// Display callback function
// Draws background and pieces
void display (void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
	glColor3f(1.0f, 1.0f, 1.0f);

	drawBackground();
	// Draw all the pieces
	glMatrixMode(GL_MODELVIEW);
	// Black pieces
	drawPiece(black_king_i, black_king_glMatrix, king, vec3(0.2f, 0.2f, 0.2f));
	drawPiece(black_queen_i, black_queen_glMatrix, queen, vec3(0.2f, 0.2f, 0.2f));
	drawPieces(8, black_pawns_i, black_pawns_glMatrix, pawn, vec3(0.2f, 0.2f, 0.2f));
	drawPieces(2, black_knights_i, black_knights_glMatrix, knight, vec3(0.2f, 0.2f, 0.2f));
	drawPieces(2, black_bishops_i, black_bishops_glMatrix, bishop, vec3(0.2f, 0.2f, 0.2f));
	drawPieces(2, black_rooks_i, black_rooks_glMatrix, rook, vec3(0.2f, 0.2f, 0.2f));
	// White pieces
	drawPiece(white_king_i, white_king_glMatrix, king, vec3(0.8f, 0.8f, 0.8f));
	drawPiece(white_queen_i, white_queen_glMatrix, queen, vec3(0.8f, 0.8f, 0.8f));
	drawPieces(8, white_pawns_i, white_pawns_glMatrix, pawn, vec3(0.8f, 0.8f, 0.8f));
	drawPieces(2, white_knights_i, white_knights_glMatrix, knight, vec3(0.8f, 0.8f, 0.8f));
	drawPieces(2, white_bishops_i, white_bishops_glMatrix, bishop, vec3(0.8f, 0.8f, 0.8f));
	drawPieces(2, white_rooks_i, white_rooks_glMatrix, rook, vec3(0.8f, 0.8f, 0.8f));

	glutSwapBuffers();  // Swap the front and back frame buffers (double buffering)
}

// Reshapes window when scaled and sets perspective
void reshape(GLsizei width, GLsizei height) {
	// Compute aspect ratio of the new window
	if (height == 0) // To prevent divide by 0
		height = 1;

	GLfloat aspect = (GLfloat)width / (GLfloat)height;

	// Set the viewport to cover the new window
	glViewport(0, 0, width, height);

	// Set the aspect ratio of the clipping volume to match the viewport
	glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
	glLoadIdentity();             // Reset

	// Enable perspective projection with fovy, aspect, zNear and zFar
	gluPerspective(45.0f, aspect, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

// Binds an image to a texture to use as background
void bindTexture(Mat img){
	glGenTextures(1, &textID);
	glBindTexture(GL_TEXTURE_2D, textID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
	0,                 				// Pyramid level (for mip-mapping) - 0 is the top level
	GL_RGB,            				// Internal colour format to convert to
	img.cols,          				// Image width
	img.rows,          				// Image height
	0,                 				// Border width in pixels (can either be 1 or 0)
	GL_BGR, 						// Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
	GL_UNSIGNED_BYTE,  				// Image data type
	img.data);        				// The actual image data itself
}

// Init Glut things
void initGlut(){
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(320,240);
	glutCreateWindow("Glut Window");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
}

// Initialize OpenGL Graphics
void initGL() {
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
   glClearDepth(1.0f);                   // Set background depth to farthest
   glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
   glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
   glShadeModel(GL_SMOOTH);   // Enable smooth shading
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections
}

int main(int argc, char** argv) {
	VideoCapture cap;
	//VideoWriter video(out_name, CV_FOURCC('M','J','P','G'), 10, Size(width, height), true);
	Mat frame, img, gl_img;
	float rvecs_z[32][10];
	double rvecs_y[32][10];
	double rvecs_x[32][10];

	// Open camera
	//cap.open(camera);
	cap.open(camera);
	if (!cap.isOpened()) {
		cout << "CAMERA?!" << endl;
		return -1;
	}
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1400);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1050);
	width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

	cout << "WIDTH: " << width << " HEIGHT: " << height << endl;
	// Dictionary of markers
	Ptr<Dictionary> dictionary = getPredefinedDictionary(DICT_4X4_50);

	// Initialize functions for glut, glew, GL
	glutInit(&argc, argv);
	initGlut();
	glewInit();
	initGL();

	// Opening Chess Object Files
	loadOBJ("Chess_Pieces/Pawn/Pawn.obj", pawn, pawn_norms);
	loadOBJ("Chess_Pieces/Knight/Knight.obj", knight, knight_norms);
	loadOBJ("Chess_Pieces/Rook/Rook.obj", rook, rook_norms);
	loadOBJ("Chess_Pieces/Bishop/Bishop.obj", bishop, bishop_norms);
	loadOBJ("Chess_Pieces/King/King.obj", king, king_norms);
	loadOBJ("Chess_Pieces/Queen/Queen.obj", queen, queen_norms);

	// Camera Parameters for drawing axis
	Mat camMatrix, distCoeffs;
	bool read = readCameraParameters(param_file, camMatrix, distCoeffs);
	if (!read){
		cerr << "Invalid camera file" << endl;
		return 0;
	}

	// Init extrinsic matrices of each chess piece to be diagonal
	initMatrices();

	while(1){
		// Main processing function of glut
		glutMainLoopEvent();

		// Grab frame
		cap >> frame;
		frame.copyTo(img);

		// Detect and draw Markers
		vector<int> markerIds;
		vector<vector<Point2f>> markerCorners;
		Ptr<DetectorParameters> params = DetectorParameters::create();
		params->cornerRefinementMethod = CORNER_REFINE_CONTOUR;

		detectMarkers(img, dictionary, markerCorners, markerIds, params);
		if (markerIds.size() > 0){
			drawDetectedMarkers(img, markerCorners, markerIds);

			for (int i = 0; i < markerIds.size(); i ++){
				// Find ID of 8 black pawns (8 - 16)
				for (int j = 0; j < 8; j ++){
					if (black_pawns[j] == markerIds[i]){
						black_pawns_i[j] = i;
					}
				}
				// Find ID of 2 black rooks
				for (int j = 0; j < 2; j ++){
					if (black_rooks[j] == markerIds[i]){
						black_rooks_i[j] = i;
					}
				}
				// Find ID of 2 black knights
				for (int j = 0; j < 2; j ++){
					if (black_knights[j] == markerIds[i]){
						black_knights_i[j] = i;
					}
				}
				// Find ID of 2 black bishops
				for (int j = 0; j < 2; j ++){
					if (black_bishops[j] == markerIds[i]){
						black_bishops_i[j] = i;
					}
				}
				// Find ID of black King
				if (markerIds[i] == black_king){
					black_king_i = i;
				}
				// Find ID of black Queen
				if (markerIds[i] == black_queen){
					black_queen_i = i;
				}

				// Find ID of 8 white pawns
				for (int j = 0; j < 8; j ++){
					if (white_pawns[j] == markerIds[i]){
						white_pawns_i[j] = i;
					}
				}
				// Find ID of 2 white rooks
				for (int j = 0; j < 2; j ++){
					if (white_rooks[j] == markerIds[i]){
						white_rooks_i[j] = i;
					}
				}
				// Find ID of 2 white knights
				for (int j = 0; j < 2; j ++){
					if (white_knights[j] == markerIds[i]){
						white_knights_i[j] = i;
					}
				}
				// Find ID of 2 white bishops
				for (int j = 0; j < 2; j ++){
					if (white_bishops[j] == markerIds[i]){
						white_bishops_i[j] = i;
					}
				}
				// Find ID of white King
				if (markerIds[i] == white_king){
					white_king_i = i;
				}
				// Find ID of white Queen
				if (markerIds[i] == white_queen){
					white_queen_i = i;
				}
			}

			// Estimate and draw all poses
			vector<Vec3d> rvecs, tvecs;
			estimatePoseSingleMarkers(markerCorners, MARK_SIZE, camMatrix, distCoeffs, rvecs, tvecs);

			// Average rvec's x rotation to get rid of the z axix flipping randomly
			/*for (int i = 0; i < markerIds.size(); i ++){
				double total = 0;
				int markerID = markerIds[i];
				for (int j = 0; j < 9; j ++){
					rvecs_z[markerID][j] = rvecs_z[markerID][j+1];
					total += rvecs_z[markerID][j];
				}
				rvecs_z[markerID][10] = rvecs[i][1];
				total += rvecs_z[markerID][10];
				double avg = total / 10.0;
				cout << "ID: " << markerID << "\tRAW: " << rvecs[i][2] << "\tAVG: " << avg << endl;
				rvecs[i][1] = avg;
			}*/

			for (int i = 0; i < markerIds.size(); i ++)
				drawAxis(img, camMatrix, distCoeffs, rvecs[i], tvecs[i], MARK_SIZE);

			// Black Pawns
			loadGLArrayOfMatrix(8, black_pawns_i, black_pawns_glMatrix, rvecs, tvecs);
			// Black Rooks
			loadGLArrayOfMatrix(2, black_rooks_i, black_rooks_glMatrix, rvecs, tvecs);
			// Black Knights
			loadGLArrayOfMatrix(2, black_knights_i, black_knights_glMatrix, rvecs, tvecs);
			// Black Bishops
			loadGLArrayOfMatrix(2, black_bishops_i, black_bishops_glMatrix, rvecs, tvecs);
			// Black Queen
			loadGLMatrix(black_queen_i, black_queen_glMatrix, rvecs, tvecs);
			// Black King
			loadGLMatrix(black_king_i, black_king_glMatrix, rvecs, tvecs);

			// White Pawns
			loadGLArrayOfMatrix(8, white_pawns_i, white_pawns_glMatrix, rvecs, tvecs);
			// White Rooks
			loadGLArrayOfMatrix(2, white_rooks_i, white_rooks_glMatrix, rvecs, tvecs);
			// White Knights
			loadGLArrayOfMatrix(2, white_knights_i, white_knights_glMatrix, rvecs, tvecs);
			// White Bishops
			loadGLArrayOfMatrix(2, white_bishops_i, white_bishops_glMatrix, rvecs, tvecs);
			// White Queen
			loadGLMatrix(white_queen_i, white_queen_glMatrix, rvecs, tvecs);
			// White King
			loadGLMatrix(white_king_i, white_king_glMatrix, rvecs, tvecs);
		}

		// Bind camera frame to texture image for GL to display as background
		// flip image since GL displays a flipped image
		flip(img, gl_img, 0);
		bindTexture(gl_img);

		display();
		imshow("camera", img);

		int key = waitKey(10);

		switch(key){
			case 'q':
				exit(1);
			case 27:
				exit(1);
			default:
				break;
		}

		// Reset arrays that hold the indices of pieces
		resetIndices();
		// Reset transformation matrices
		initMatrices();
		cout << "==================================================================" << endl;
	}
}
