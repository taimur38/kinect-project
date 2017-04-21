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
	colorBuffer.resize(px_width * px_height * bpp);
	patternBuffer.resize(px_width * px_height * bpp);
	copyBuffer.resize(px_width * px_height * bpp);

	depthFd->get_Width(&depth_width);
	depthFd->get_Height(&depth_height);
	depthFd->get_BytesPerPixel(&depth_bpp);

	depthBuffer.resize(depth_width*depth_height);
	depth2xyz.resize(depth_width * depth_height);

	//img.allocate(px_width * bpp, px_height * bpp, ofImageType::OF_IMAGE_COLOR);
	texture.allocate(px_width, px_height, GL_RGBA);
	texture.enableMipmap();
	img.allocate(px_width, px_height, ofImageType::OF_IMAGE_COLOR);
	//pixelz.allocate(px_width, px_height, ofImageType::OF_IMAGE_COLOR);

	//boxaroo.loadImage("navy.png");
	boxaroo.loadImage("white.png");
}

//--------------------------------------------------------------
void ofApp::update() {

	if (calibrated) {

		HRESULT hr;
		IColorFrame* colorFrame;
		IDepthFrame* depthFrame;
		int fuckups = 0;
		int goodones = 0;

		unsigned int size = 0;
		unsigned short* newbuff = nullptr;
		hr = colorFrameReader->AcquireLatestFrame(&colorFrame);
		//hr = depthFrameReader->AcquireLatestFrame(&depthFrame);
		if (SUCCEEDED(hr)) {
			hr = colorFrame->CopyConvertedFrameDataToArray(colorBuffer.size(), &colorBuffer[0], ColorImageFormat::ColorImageFormat_Bgra);
			//hr = colorFrame->AccessRawUnderlyingBuffer(&size, &newbuff);

			//hr = depthFrame->CopyFrameDataToArray(depthBuffer.size(), &depthBuffer[0]);

			/*hr = depthFrame->AccessUnderlyingBuffer(&size, &newbuff);
			hr = cMapper->MapDepthFrameToColorSpace(depth_width*depth_height, newbuff, depth_width*depth_height, &depth2xyz[0]);

			unsigned short mDepth;
			depthFrameSource->get_DepthMaxReliableDistance(&mDepth);
			*/

			//TODO: halfway through doing stuff with depth...
			//might be time to refactor calibration as a class first.
			if (!SUCCEEDED(hr)) {
				ofLog(OF_LOG_ERROR, "couldnt access buffer");
			}
			// can i push this logic to shader? 
			// not really if i want to make this a thing other peopel can use
			for (int i = 0; i < colorBuffer.size(); i += 4) {
			//for(int i = 0; i < depth2xyz.size(); i++) {
				//const int newIndex = (int)depth2xyz[i].X + (int)depth2xyz[i].Y * px_width;
				int offsetPosition = convertIndex(i);
				if (offsetPosition < 0) {
					/*copyBuffer[newIndex] = 0;
					copyBuffer[newIndex+ 1] = 0;
					copyBuffer[offsetPosition + 2] = 0;
					copyBuffer[offsetPosition + 3] = 255;
					*/
					continue;
				}
				if (offsetPosition + 3 >= copyBuffer.size()) {
					fuckups++;
					continue;
				}
				else {
					goodones++;
				}

				copyBuffer[offsetPosition] = colorBuffer[i];
				copyBuffer[offsetPosition + 1] = colorBuffer[i + 1];
				copyBuffer[offsetPosition + 2] = colorBuffer[i + 2];
				copyBuffer[offsetPosition + 3] = colorBuffer[i + 3];
				
				/*
				unsigned int d = newbuff[i];

				auto mapped = (float)(std::min(d, (unsigned int)(mDepth))) / (float)mDepth / 255;
				
				copyBuffer[offsetPosition] = mapped;
				copyBuffer[offsetPosition + 1] = mapped;
				copyBuffer[offsetPosition + 2] = mapped;
				copyBuffer[offsetPosition + 3] = 1;
				*/
			}

			texture.loadData(&copyBuffer[0], newWidth, newHeight, GL_BGRA);
			//texture.readToPixels(pixelz);
			//img.setFromPixels(pixelz);
		}
		else {
			ofLogError("didn't get latest frame");
		}

		if (colorFrame)
			colorFrame->Release();

		/*if (depthFrame)
			depthFrame->Release();
			*/
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
	int s = 10;
	int w = 1920;
	int h = 1080;
	w = ofGetViewportWidth();
	h = ofGetViewportHeight();

	if (!calibrated) {
		//ofLog(OF_LOG_VERBOSE, "state is: %d", state);
		boxaroo.draw(w * (state % 2) - s / 2, h * (state / 2) - s / 2, s, s); // place square. top left, top right, bottom left, bottom right.
	}
	else {

		//img.draw(0, 0, 1920, 1080);

		texture.draw(0, 0, w, h);
	}
		//texture.draw(0, 0, 1920, 1080);
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

int ofApp::convertIndex(int incomingImage) {

	// we know 4 corners. we know the x,y position that would have corresponded to.
	// 

	int originalXPosition = (incomingImage / 4) % px_width;
	int originalYPosition = (incomingImage / 4) / px_width;

	if (originalXPosition > topLeft.x)
		return -1;
	if(originalXPosition < topRight.x)
		return -1;
	if (originalYPosition < topLeft.y)
		return -1;
	if (originalYPosition > bottomRight.y)
		return -1;

	// were in the money zone 
	// we know we need to 

	int translatedX = originalXPosition - topRight.x;
	int translatedY = originalYPosition - topRight.y;

	// flip this guy. 0,0 => (newWidth, newHeight)

	int flippedX = newWidth - translatedX;
	//int flippedY = newHeight - translatedY;
	int flippedY = translatedY;

	// convert this to a buffer position.

	return (flippedY * newWidth + flippedX) * 4;
}


// todo
int ofApp::convertPoint(int originalXPosition, int originalYPosition) {

	return 1;
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

	return 0;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	ofLog(OF_LOG_VERBOSE, "%d", key);

	if (key == 32 && !calibrated) { // space
		//while (state <= 4) {
			ofLog(OF_LOG_VERBOSE, "calibrating...");
			Calibrate();
			ofLog(OF_LOG_VERBOSE, "done");
			state++;
			//draw();
		//}
	}

	if (state == 4 && !calibrated) {
		calibrated = true;
		newWidth = std::abs(topRight.x - topLeft.x);
		newHeight = std::abs(topRight.y - bottomRight.y);

		ofLog(OF_LOG_VERBOSE, "locations:\nTL: (%f, %f)\n TR: (%f, %f)\n BL: (%f, %f)\n BR: (%f, %f)", topLeft.x, topLeft.y, topRight.x, topRight.y, bottomLeft.x, bottomLeft.y, bottomRight.x, bottomRight.y);

		//ofLog(OF_LOG_VERBOSE, "top left: %f, %f", topLeft.x, topLeft.y);
		copyBuffer.resize(newWidth * newHeight * 4);
		img.allocate(newWidth, newHeight, ofImageType::OF_IMAGE_COLOR);
		//copyBufferSize = newWidth * newHeight * 4;
	}

	/*if (key == 120) { // x
		colorBuffer.clear();
	}*/
}

void ofApp::findBlueSquare() {

	// b,g,r,a values - colorbuffer[0] is the b value of first pixel. 
	// we will loop and always keep i as the blue value of center of the grid.

	patternBuffer = colorBuffer;

	const int max = 255 * 3;

	for (int x = 0; x < 5; x++) {

		for (int i = (px_width + 1) * 4; i < colorBuffer.size() - (px_width + 1)*4; i += 4) {

			try {

				// 3-pixel wide row above center 
				const unsigned int a1_b = colorBuffer[i - (px_width + 1) * 4];
				const unsigned int a1_g = colorBuffer[i - (px_width + 1) * 4 + 1];
				const unsigned int a1_r = colorBuffer[i - (px_width + 1) * 4 + 2];
				const double a1_ratio = (double)(a1_b + a1_g + a1_r) / (double)(max);
				//ofLog(OF_LOG_VERBOSE, "%f", a1_ratio);

				const unsigned int a2_b = colorBuffer[i - px_width * 4];
				const unsigned int a2_g = colorBuffer[i - px_width * 4 + 1];
				const unsigned int a2_r = colorBuffer[i - px_width * 4 + 2];
				const double a2_ratio = (double)(a2_b + a1_g + a1_r) / (double)(max);

				const unsigned int a3_b = colorBuffer[i - (px_width - 1) * 4];
				const unsigned int a3_g = colorBuffer[i - (px_width - 1) * 4 + 1];
				const unsigned int a3_r = colorBuffer[i - (px_width - 1) * 4 + 2];
				const double a3_ratio = (double)(a3_b + a3_g + a3_r) / (double)(max);

				// current row of pixel
				const unsigned int a4_b = colorBuffer[i - 4];
				const unsigned int a4_g = colorBuffer[i - 3];
				const unsigned int a4_r = colorBuffer[i - 2];
				const double a4_ratio = (double)(a4_b + a4_g + a4_r) / (double)(max);

				const unsigned int a5_b = colorBuffer[i]; // center
				const unsigned int a5_g = colorBuffer[i + 1]; // center
				const unsigned int a5_r = colorBuffer[i + 2]; // center
				const double a5_ratio = (double)(a5_b + a5_g + a5_r) / (double)(max);

				const unsigned int a6_b = colorBuffer[i + 4];
				const unsigned int a6_g = colorBuffer[i + 5];
				const unsigned int a6_r = colorBuffer[i + 6];
				const double a6_ratio = (double)(a6_b + a6_g + a6_r) / (double)(max);


				// row below pixel
				const unsigned int a7_b = colorBuffer[i + (px_width - 1) * 4 + 0];
				const unsigned int a7_g = colorBuffer[i + (px_width - 1) * 4 + 1];
				const unsigned int a7_r = colorBuffer[i + (px_width - 1) * 4 + 2];
				const double a7_ratio = (double)(a7_b + a7_g + a7_r) / (double)(max);

				const unsigned int a8_b = colorBuffer[i + px_width * 4 + 0];
				const unsigned int a8_g = colorBuffer[i + px_width * 4 + 1];
				const unsigned int a8_r = colorBuffer[i + px_width * 4 + 2];
				const double a8_ratio = (double)(a8_b + a8_g + a8_r) / (double)(max);

				const unsigned int a9_b = colorBuffer[i + (px_width + 1) * 4 + 0];
				const unsigned int a9_g = colorBuffer[i + (px_width + 1) * 4 + 1];
				const unsigned int a9_r = colorBuffer[i + (px_width + 1) * 4 + 2];
				const double a9_ratio = (double)(a9_b + a9_g + a9_r) / (double)(max);

				//if (a1_ratio + a2_ratio + a3_ratio + a4_ratio + a6_ratio + a7_ratio + a8_ratio + a9_ratio > (.6 * 8) && a5_ratio > 0.5) {
				if(a5_ratio > 0.8) { 
					patternBuffer[i] = 255;
					patternBuffer[i + 1] = 255;
					patternBuffer[i + 2] = 255;
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
			//texture.loadData(&colorBuffer[0], px_width, px_height, GL_BGRA);
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
