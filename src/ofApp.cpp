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
void ofApp::update() {

	if (calibrated) {

		HRESULT hr;
		IColorFrame* colorFrame;
		hr = colorFrameReader->AcquireLatestFrame(&colorFrame);
		if (SUCCEEDED(hr)) {
			hr = colorFrame->CopyConvertedFrameDataToArray(colorBuffer.size(), &colorBuffer[0], ColorImageFormat::ColorImageFormat_Bgra);

			//TODO: loop through colorBuffer, get new x/y pos, write values to new buffer, and load THAT one.
			texture.loadData(&patternBuffer[0], px_width, px_height, GL_BGRA);
		}
		else {
			ofLogError("didn't get latest frame");
		}

		if (colorFrame)
			colorFrame->Release();
	}

}

void ofApp::setCorner() {

	switch (state % 4) {
	case 0:
		ofLog(OF_LOG_VERBOSE, "setting top left");
		topLeft = getCenterOfBlueSquare();
		ofLog(OF_LOG_VERBOSE, "top left: %f, %f", topLeft.x, topLeft.y);
		break;
	case 1:
		ofLog(OF_LOG_VERBOSE, "setting top right");
		topRight = getCenterOfBlueSquare();
		ofLog(OF_LOG_VERBOSE, "top right: %f, %f", topRight.x, topRight.y);
		break;
	case 3:
		ofLog(OF_LOG_VERBOSE, "setting bottom right");
		bottomRight = getCenterOfBlueSquare();
		ofLog(OF_LOG_VERBOSE, "top left: %f, %f", bottomRight.x, bottomRight.y);
		break;
	case 2:
		ofLog(OF_LOG_VERBOSE, "setting bottom left");
		bottomLeft = getCenterOfBlueSquare();
		ofLog(OF_LOG_VERBOSE, "bottom left: %f, %f", bottomLeft.x, bottomLeft.y);
		break;
	}

}

ofVec2f ofApp::getCenterOfBlueSquare() {
	
	double avg_x = 0;
	double avg_y = 0;

	int blues = 0;
	for (int i = 0; i < patternBuffer.size(); i += 4) {
		if (patternBuffer[i] == 255)
			blues++;
	}

	for (int i = 0; i < patternBuffer.size(); i += 4)
	{
		const unsigned int blue_val = patternBuffer[i];
		if (blue_val == 255) {
			int px_val = i / 4;
			int x = px_val % px_width;
			int y = px_val / px_width;

			avg_x += (double)x / blues;
			avg_y += (double)y / blues;
		}
	}

	ofLog(OF_LOG_VERBOSE, "px_width: %d, px_height: %d, blues: %d", px_width, px_height, blues);

	ofLog(OF_LOG_VERBOSE, "(%f, %f)", avg_x, avg_y);
	return ofVec2f(avg_x, avg_y);
}

//--------------------------------------------------------------
void ofApp::draw() {

	//texture.draw(0, 0);
	int s = 400;
	int w = 1920;
	int h = 1080;
	w = ofGetViewportWidth();
	h = ofGetViewportHeight();

	boxaroo.draw(w * (state%2) - s/2, h * (state/2) - s/2, s, s); // place square. top left, top right, bottom left, bottom right.
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

ofVec2f ofApp::convertPoint(ofVec2f cameraPoint) {

	// assuming topleft,topright,bottomleft,bottomright are set.
	// in practice we want to take things we see from camera and convert it into screen.

	if (cameraPoint.x < topLeft.x || cameraPoint.x > topRight.x || cameraPoint.y < topLeft.y || cameraPoint.y > bottomRight.y)
		return ofVec2f(-100, -100);

	const double cameraDiagonal = sqrt(((topLeft.x - bottomRight.x) * (topLeft.x - bottomRight.x)) + (topLeft.y - bottomRight.y) * (topLeft.y - bottomRight.y));
	const double screenDiagonal = sqrt((ofGetViewportWidth() * ofGetViewportWidth()) + (ofGetViewportHeight() * ofGetViewportHeight()));

	const double scaleCameraWidth = ofGetViewportWidth() / (bottomRight.x - topLeft.x);   
	const double scaleCameraHeight = ofGetViewportHeight() / (bottomRight.y - topLeft.y);

	const double scaleCameraToScreen = screenDiagonal / cameraDiagonal; // if same aspect ratio

	//topLeft is 0,0 in our world but topLeft in the camera world.
	// so when we get a cameraValue at topleft, when we convert it it should come out to 0,0

	ofVec2f scaledTopLeft = ofVec2f(topLeft);
	scaledTopLeft.x *= scaleCameraWidth;
	scaledTopLeft.y *= scaleCameraHeight;

	ofVec2f convertedPoint = ofVec2f();

	convertedPoint.x = cameraPoint.x * scaleCameraWidth - scaledTopLeft.x;
	convertedPoint.y = cameraPoint.y * scaleCameraHeight - scaledTopLeft.y;

	return convertedPoint;
}

int ofApp::getModifiedX(int cameraX) {

	if (cameraX < topLeft.x) {
		return -100;
	}

	if (cameraX > topRight.x) {
		return 100;
	}

	float scale = ofGetViewportWidth() / (topRight.x - topLeft.x);
	float offset = topLeft.x; // subtract this?
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	ofLog(OF_LOG_VERBOSE, "%d", key);

	if (key == 32) { // space
		Calibrate();
		state++;
	}

	if (state >= 4) {
		calibrated = true;
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

}

void ofApp::Calibrate()
{
	HRESULT hr;
	IColorFrame* colorFrame;
	hr = colorFrameReader->AcquireLatestFrame(&colorFrame);
	if (SUCCEEDED(hr)) {
		hr = colorFrame->CopyConvertedFrameDataToArray(colorBuffer.size(), &colorBuffer[0], ColorImageFormat::ColorImageFormat_Bgra);

		if (SUCCEEDED(hr)) {

			findBlueSquare();
			setCorner();
		}
	}
	else {
		ofLogError("didn't get latest frame");
	}

	if(colorFrame)
		colorFrame->Release();
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
