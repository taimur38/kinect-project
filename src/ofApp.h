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
		
		IKinectSensor* kinect;
		IColorFrameSource* colorFrameSource;
		IColorFrameReader* colorFrameReader;
		IFrameDescription* fd;

		int px_width;
		int px_height;
		unsigned int bpp;

		vector<BYTE> colorBuffer;
		unsigned int colorBufferSize;

		ofTexture texture;
		ofImage img;

};