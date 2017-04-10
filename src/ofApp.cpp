#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	HRESULT hr;

	ofSetBackgroundColor(ofColor::red);
	ofSetLogLevel(OF_LOG_VERBOSE);

	hr = GetDefaultKinectSensor(&kinect);

	ofLogVerbose("HELLO");
	if (FAILED(hr)) {
		ofLogError("couldnt get kinect");
		return;
	}

	hr = kinect->Open();
	if (FAILED(hr)) {
		ofLogError("init: error opening sensor");
		return;
	}
	ofLogVerbose("opened kinect");
	
	hr = kinect->get_ColorFrameSource(&colorFrameSource);
	if (FAILED(hr)) {
		ofLogError("init: error getting frame source");
		return;
	}
	ofLogVerbose("got frame source");

	hr = colorFrameSource->OpenReader(&colorFrameReader);

	if (FAILED(hr)) {
		ofLogError("init: error getting frame reader");
		return;
	}
	ofLogVerbose("got frame reader");

	colorFrameSource->CreateFrameDescription(ColorImageFormat::ColorImageFormat_Bgra, &fd);

	fd->get_Width(&px_width);
	fd->get_Height(&px_height);
	fd->get_BytesPerPixel(&bpp);
	colorBuffer.resize(px_width * px_height * bpp);
	patternBuffer.resize(px_width * px_height * bpp);

	//img.allocate(px_width * bpp, px_height * bpp, ofImageType::OF_IMAGE_COLOR);
	texture.allocate(px_width, px_height, GL_RGBA);

	boxaroo.loadImage("navy.png");
}

//--------------------------------------------------------------
void ofApp::update(){

	if (takePicture) {
		HRESULT hr;
		IColorFrame* colorFrame;
		hr = colorFrameReader->AcquireLatestFrame(&colorFrame);
		if (SUCCEEDED(hr)) {
			takePicture = false;
			hr = colorFrame->CopyConvertedFrameDataToArray(colorBuffer.size(), &colorBuffer[0], ColorImageFormat::ColorImageFormat_Bgra);

			if (SUCCEEDED(hr)) {

				findBlueSquare();
				texture.loadData(&patternBuffer[0], px_width, px_height, GL_BGRA);
			}
		}
		else {
			//ofLogError("didn't get latest frame");
		}

		if(colorFrame)
			colorFrame->Release();
	}


}

//--------------------------------------------------------------
void ofApp::draw(){

	texture.draw(0, 0);
	int s = 400;
	boxaroo.draw(ofGetMouseX() - s/2, ofGetMouseY() - s/2, s, s);
}

void ofApp::filter() {

	for (int i = 0; i < colorBuffer.size(); i += 4) {
		const unsigned int b = colorBuffer[i];
		const unsigned int g = colorBuffer[i + 1];
		const unsigned int r = colorBuffer[i + 2];
		const unsigned int a = colorBuffer[i + 3];

		const double ratio = (double)b / (double)(b + g + r);
		if (ratio >= .5) {
			colorBuffer[i] = (int)(ratio * 255);
			colorBuffer[i + 1] = 0;
			colorBuffer[i + 2] = 0;
			colorBuffer[i + 3] = 255;
		}
		else {
			colorBuffer[i] = 0;
			colorBuffer[i + 1] = 0;
			colorBuffer[i + 2] = 0;
			colorBuffer[i + 3] = 255;
		}
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	ofLog(OF_LOG_VERBOSE, "%d", key);

	if (key == 32) { // space
		findBlueSquare();
	}

	if (key == 120) { // x
		colorBuffer.clear();
	}
}

void ofApp::findBlueSquare() {

	// b,g,r,a values - colorbuffer[0] is the b value of first pixel. 
	// we will loop and always keep i as the blue value of center of the grid.

	patternBuffer = colorBuffer;

	for (int x = 0; x < 5; x++) {

		for (int i = (px_width + 1) * 4; i < colorBuffer.size() - (px_width + 1)*4; i += 4) {

			try {

				// 3-pixel wide row above center 
				const unsigned int a1_b = colorBuffer[i - (px_width + 1) * 4];
				const unsigned int a1_g = colorBuffer[i - (px_width + 1) * 4 + 1];
				const unsigned int a1_r = colorBuffer[i - (px_width + 1) * 4 + 2];
				const double a1_ratio = (double)(a1_b) / (double)(a1_b + a1_g + a1_r);

				const unsigned int a2_b = colorBuffer[i - px_width * 4];
				const unsigned int a2_g = colorBuffer[i - px_width * 4 + 1];
				const unsigned int a2_r = colorBuffer[i - px_width * 4 + 2];
				const double a2_ratio = (double)(a2_b) / (double)(a2_b + a2_g + a2_r);

				const unsigned int a3_b = colorBuffer[i - (px_width - 1) * 4];
				const unsigned int a3_g = colorBuffer[i - (px_width - 1) * 4 + 1];
				const unsigned int a3_r = colorBuffer[i - (px_width - 1) * 4 + 2];
				const double a3_ratio = (double)(a3_b) / (double)(a3_b + a3_g + a3_r);

				// current row of pixel
				const unsigned int a4_b = colorBuffer[i - 4];
				const unsigned int a4_g = colorBuffer[i - 3];
				const unsigned int a4_r = colorBuffer[i - 2];
				const double a4_ratio = (double)(a4_b) / (double)(a4_b + a4_g + a4_r);

				const unsigned int a5_b = colorBuffer[i]; // center
				const unsigned int a5_g = colorBuffer[i + 1]; // center
				const unsigned int a5_r = colorBuffer[i + 2]; // center
				const double a5_ratio = (double)(a5_b) / (double)(a5_b + a5_g + a5_r);

				const unsigned int a6_b = colorBuffer[i + 4];
				const unsigned int a6_g = colorBuffer[i + 5];
				const unsigned int a6_r = colorBuffer[i + 6];
				const double a6_ratio = (double)(a6_b) / (double)(a6_b + a6_g + a6_r);


				// row below pixel
				const unsigned int a7_b = colorBuffer[i + (px_width - 1) * 4 + 0];
				const unsigned int a7_g = colorBuffer[i + (px_width - 1) * 4 + 1];
				const unsigned int a7_r = colorBuffer[i + (px_width - 1) * 4 + 2];
				const double a7_ratio = (double)(a7_b) / (double)(a7_b + a7_g + a7_r);

				const unsigned int a8_b = colorBuffer[i + px_width * 4 + 0];
				const unsigned int a8_g = colorBuffer[i + px_width * 4 + 1];
				const unsigned int a8_r = colorBuffer[i + px_width * 4 + 2];
				const double a8_ratio = (double)(a8_b) / (double)(a8_b + a8_g + a8_r);

				const unsigned int a9_b = colorBuffer[i + (px_width + 1) * 4 + 0];
				const unsigned int a9_g = colorBuffer[i + (px_width + 1) * 4 + 1];
				const unsigned int a9_r = colorBuffer[i + (px_width + 1) * 4 + 2];
				const double a9_ratio = (double)(a9_b) / (double)(a9_b + a9_g + a9_r);

				if (a1_ratio + a2_ratio + a3_ratio + a4_ratio + a6_ratio + a7_ratio + a8_ratio + a9_ratio > (.4/(x+1) * 8) && a5_ratio > 0.5) {
					patternBuffer[i] = 255;
					patternBuffer[i + 1] = 0;
					patternBuffer[i + 2] = 0;
					patternBuffer[i + 3] = 255;
				}
				else {
					patternBuffer[i] = 0;
					patternBuffer[i + 1] = 0;
					patternBuffer[i + 2] = 0;
					patternBuffer[i + 3] = 255;
				}

			}
			catch (exception e) {
				ofLog(OF_LOG_VERBOSE, "fuckin error mate: index: %d, px_width: %d, px_height: %d", i, px_width, px_height);
			}
		}
		colorBuffer = patternBuffer;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	takePicture = true;
	const ofVec2f curr = ofVec2f(x, y);
	switch(state++ % 4) {
	case 0:
		ofLogVerbose("setting top left");
		topLeft = curr;
		break;
	case 1:
		ofLogVerbose("setting top right");
		topRight = curr;
		break;
	case 2:
		ofLogVerbose("setting bottom right");
		bottomRight = curr;
		break;
	case 3:
		ofLogVerbose("setting bottom left");
		bottomLeft = curr;
		break;
	default:
		ofLogVerbose("shit is broken");
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
