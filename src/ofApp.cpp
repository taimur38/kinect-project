#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	HRESULT hr;

	ofSetFrameRate(30);

	ofSetFullscreen(true);
	//ofDisableArbTex();
	//ofDisableNormalizedTexCoords();
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
	//texture.enableMipmap();

	img.allocate(px_width, px_height, ofImageType::OF_IMAGE_COLOR);
	//pixelz.allocate(px_width, px_height, ofImageType::OF_IMAGE_COLOR);

	//boxaroo.loadImage("navy.png");
	boxaroo.loadImage("white.png");
	calibrater = new Calibrater(kinect);

	auto corners = Corners();
	corners.bottomLeft = ofVec2f(418, 210);
	corners.bottomRight = ofVec2f(156, 195);
	corners.topLeft = ofVec2f(422, 68);
	corners.topRight = ofVec2f(167, 49);

	shaderProg.load("test.vert", "test.frag");

	//calibrater->setCorners(corners);
	//calibrated = true;
}

int zones = 4;
//--------------------------------------------------------------
void ofApp::update() {

	if (calibrated) {

		IDepthFrame* df;
		depthBuffer = calibrater->getMappedDepthFrame(&df);

		auto w = calibrater->getMappedWidth();
		auto h = calibrater->getMappedHeight();
		if (colorBuffer.size() != w * h * 4) {
			colorBuffer.resize(w * h * 4);
		}

		int maxDist = calibrater->avgCalibrationDepth;

		for (int i = 0; i < depthBuffer.size(); i++) {
			int rawDepth = depthBuffer[i];

			int depth = 0;
			if (rawDepth != 0 && rawDepth < .95 * maxDist) {
				depth = (1 - ((float)rawDepth) / maxDist) * 255;
			}

			colorBuffer[i * 4] = depth;
			colorBuffer[i * 4 + 1] = depth;
			colorBuffer[i * 4 + 2] = depth;
			colorBuffer[i * 4 + 3] = depth;
		}

		texture.loadData(&colorBuffer[0], w, h, GL_BGRA);

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
		calibrater->Draw();
	}
	else {

		plane.mapTexCoords(0, 0, newWidth, newHeight);

		texture.bind();
		shaderProg.begin();
		shaderProg.setUniform1f("time", ofGetElapsedTimef());

		ofTranslate(w / 2, h / 2);
		plane.drawFaces();
		shaderProg.end();
		texture.unbind();

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
		newWidth = calibrater->getMappedWidth();
		newHeight = calibrater->getMappedHeight();

		ofLog(OF_LOG_VERBOSE, "dimensions: (%d, %d)", newWidth, newHeight);

		copyBuffer.resize(newWidth * newHeight * 4);
		plane.set(ofGetViewportWidth(), ofGetViewportHeight(), newWidth, newHeight, OF_PRIMITIVE_TRIANGLES);
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