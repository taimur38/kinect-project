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
		int convertPoint(int x, int y);

		int convertIndex(int x);

		IKinectSensor* kinect;
		IColorFrameSource* colorFrameSource;
		IColorFrameReader* colorFrameReader;
		IFrameDescription* fd;

		IDepthFrameSource* depthFrameSource;
		IDepthFrameReader* depthFrameReader;
		IFrameDescription* depthFd;

		ICoordinateMapper* cMapper;

		int px_width;
		int px_height;
		unsigned int bpp;

		int depth_width;
		int depth_height;
		unsigned int depth_bpp;

		int newWidth;
		int newHeight;

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

		vector<BYTE> copyBuffer;
		unsigned int copyBufferSize;

		vector<unsigned short> depthBuffer;

		vector<ColorSpacePoint> depth2xyz;


		ofFloatPixels pixelz;
		ofTexture texture;
		ofImage img;
};
