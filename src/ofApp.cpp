#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	HRESULT hr;

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

	//img.allocate(px_width * bpp, px_height * bpp, ofImageType::OF_IMAGE_COLOR);
	texture.allocate(px_width, px_height, GL_RGBA);

}

//--------------------------------------------------------------
void ofApp::update(){

	HRESULT hr;
	IColorFrame* colorFrame;
	hr = colorFrameReader->AcquireLatestFrame(&colorFrame);
	if (SUCCEEDED(hr)) {
		hr = colorFrame->CopyConvertedFrameDataToArray(colorBuffer.size(), &colorBuffer[0], ColorImageFormat::ColorImageFormat_Bgra);

		if (SUCCEEDED(hr)) {
			texture.loadData(&colorBuffer[0], px_width, px_height, GL_BGRA);
		}
	}
	else {
		ofLogError("didn't get latest frame");
	}

	if(colorFrame)
		colorFrame->Release();


}

//--------------------------------------------------------------
void ofApp::draw(){

	texture.draw(0, 0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
