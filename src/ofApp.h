#pragma once

#include "ofMain.h"
#include "Kinect.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void filter(); // unused - see findBlueSquare

		void findBlueSquare();
		ofVec2f getCenterOfBlueSquare();
		void setCorner(); 
		void Calibrate();
		int getModifiedX(int x);
		ofVec2f convertPoint(ofVec2f cameraPoint);
		
		IKinectSensor* kinect;
		IColorFrameSource* colorFrameSource;
		IColorFrameReader* colorFrameReader;
		IFrameDescription* fd;

		int px_width;
		int px_height;
		unsigned int bpp;


		ofImage boxaroo;

		bool takePicture = false;

		bool calibrated = false;

		int state = 0;
		ofVec2f topLeft;
		ofVec2f topRight;
		ofVec2f bottomLeft;
		ofVec2f bottomRight;

		vector<BYTE> patternBuffer;
		unsigned int patternBufferSize;

		vector<BYTE> colorBuffer;
		unsigned int colorBufferSize;

		ofTexture texture;
		ofImage img;

};
