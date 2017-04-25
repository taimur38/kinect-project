#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	HRESULT hr;

	ofSetFrameRate(30);

	ofSetFullscreen(true);
	ofDisableArbTex();
	ofSetBackgroundColor(ofColor::black);
	ofSetLogLevel(OF_LOG_VERBOSE);

	hr = GetDefaultKinectSensor(&kinect);

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
	hr = kinect->get_DepthFrameSource(&depthFrameSource);
	if (FAILED(hr)) {
		ofLogError("init: error getting frame source");
		return;
	}
	ofLogVerbose("got frame source");

	hr = colorFrameSource->OpenReader(&colorFrameReader);
	hr = depthFrameSource->OpenReader(&depthFrameReader);

	if (FAILED(hr)) {
		ofLogError("init: error getting frame reader");
		return;
	}
	ofLogVerbose("got frame reader");

	colorFrameSource->CreateFrameDescription(ColorImageFormat::ColorImageFormat_Bgra, &fd);
	depthFrameSource->get_FrameDescription(&depthFd);

	kinect->get_CoordinateMapper(&cMapper);

	fd->get_Width(&px_width);
	fd->get_Height(&px_height);
	fd->get_BytesPerPixel(&bpp);
	colorBuffer.resize(depth_width * depth_height * bpp);
	rawColorBuffer.resize(px_width * px_height * bpp);
	patternBuffer.resize(px_width * px_height * bpp);
	copyBuffer.resize(px_width * px_height * bpp);

	depthFd->get_Width(&depth_width);
	depthFd->get_Height(&depth_height);
	depthFd->get_BytesPerPixel(&depth_bpp);

	depthBuffer.resize(depth_width*depth_height);
	depth2xyz.resize(depth_width * depth_height);

	colorToDepthBuffer.resize(px_width * px_height);

	depth2rgb = new ColorSpacePoint[depth_width * depth_height];

	texture.allocate(depth_width, depth_height, GL_RGBA);
	texture.enableMipmap();

	img.allocate(px_width, px_height, ofImageType::OF_IMAGE_COLOR);
	//pixelz.allocate(px_width, px_height, ofImageType::OF_IMAGE_COLOR);

	//boxaroo.loadImage("navy.png");
	boxaroo.loadImage("white.png");
	calibrater = new Calibrater(kinect);
}

//--------------------------------------------------------------
void ofApp::update() {

	if (calibrated) {

		IDepthFrame* df;
		depthBuffer = calibrater->getMappedDepthFrame(&df);

		if (colorBuffer.size() != depth_width * depth_height * 4) {
			colorBuffer.resize(depth_width * depth_height * 4);
		}

		unsigned short maxDist = 0;

		for (int i = 0; i < depthBuffer.size(); i++)
			if (depthBuffer[i] > maxDist)
				maxDist = depthBuffer[i];


		for (int i = 0; i < depthBuffer.size(); i++) {
			unsigned short depth = depthBuffer[i];
			if(depth == 0) {
				colorBuffer[i * 4] = 0;
				colorBuffer[i * 4 + 1] = 0;
				colorBuffer[i * 4 + 2] = 0;
				colorBuffer[i * 4 + 3] = 255;
			}
			else if (depth < maxDist * .9 ) {
				float depthRatio = ((float)depth) / maxDist;
				colorBuffer[i * 4] = depthRatio*255;
				colorBuffer[i * 4 + 1] = depthRatio*255;
				colorBuffer[i * 4 + 2] = depthRatio*255;
				colorBuffer[i * 4 + 3] = 255;
			}
			else {
				colorBuffer[i * 4] = 0;
				colorBuffer[i * 4 + 1] = 255;
				colorBuffer[i * 4 + 2] = 0;
				colorBuffer[i * 4 + 3] = 255;
			}
		}

		texture.loadData(&colorBuffer[0], calibrater->getMappedWidth(), calibrater->getMappedHeight(), GL_BGRA);

		if (df)
			df->Release();

	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	//texture.draw(0, 0);
	int s = 10;
	int h = ofGetViewportHeight();
	int w = ofGetViewportWidth();

	if (!calibrated) {
		//ofLog(OF_LOG_VERBOSE, "state is: %d", state);
		//boxaroo.draw(w * (state % 2) - s / 2, h * (state / 2) - s / 2, s, s); // place square. top left, top right, bottom left, bottom right.
		calibrater->Draw();

	}
	else {
		texture.draw(0, 0, w, h);
	}
}



//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	ofLog(OF_LOG_VERBOSE, "%d", key);

	if (key == 32 && !calibrated) { // space
		ofLog(OF_LOG_VERBOSE, "calibrating...");
		Calibrate();
		ofLog(OF_LOG_VERBOSE, "done");
		state++;
	}

	if (state == 4 && !calibrated) {
		calibrated = true;
		newWidth = std::abs(topRight.x - topLeft.x);
		newHeight = std::abs(topRight.y - bottomRight.y);

		ofLog(OF_LOG_VERBOSE, "locations:\nTL: (%f, %f)\n TR: (%f, %f)\n BL: (%f, %f)\n BR: (%f, %f)", topLeft.x, topLeft.y, topRight.x, topRight.y, bottomLeft.x, bottomLeft.y, bottomRight.x, bottomRight.y);

		copyBuffer.resize(newWidth * newHeight * 4);
	}
}

void ofApp::Calibrate()
{
	calibrater->Calibrate();
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
