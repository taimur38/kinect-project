#include "Calibrater.h"

Calibrater::Calibrater(IKinectSensor* _kinect) {
	kinect = _kinect;

	HRESULT hr;

	hr = kinect->get_ColorFrameSource(&colorFrameSource);
	if (FAILED(hr)) {
		ofLog(OF_LOG_VERBOSE, "couldnt get color frame source");
	}
	hr = kinect->get_DepthFrameSource(&depthFrameSource);

	if (FAILED(hr)) {
		ofLog(OF_LOG_VERBOSE, "couldnt get depth frame source");
	}

	hr = colorFrameSource->OpenReader(&colorFrameReader);
	if (FAILED(hr))
		ofLog(OF_LOG_VERBOSE, "couldnt get color frame reader");
	hr = depthFrameSource->OpenReader(&depthFrameReader);
	if (FAILED(hr))
		ofLog(OF_LOG_VERBOSE, "couldnt get depth frame reader");

	colorFrameSource->CreateFrameDescription(ColorImageFormat::ColorImageFormat_Bgra, &fd);
	depthFrameSource->get_FrameDescription(&depthFd);

	kinect->get_CoordinateMapper(&cMapper);

	fd->get_Width(&color_width);
	fd->get_Height(&color_height);
	fd->get_BytesPerPixel(&bpp);

	depthFd->get_Width(&depth_width);
	depthFd->get_Height(&depth_height);

	rawColorBuffer.resize(color_width * color_height * bpp);
	mappedColorBuffer.resize(depth_width * depth_height * bpp);
	filteredColorBuffer.resize(depth_width * depth_height * bpp);

	colorToDepthBuffer.resize(color_width*color_height);
	depthBuffer.resize(depth_height*depth_width);

	//devTexture.allocate(depth_width, depth_height, GL_BGRA);
	devTexture.allocate(depth_width, depth_height, GL_RGBA);
	devTexture.enableMipmap();

	boxaroo.loadImage("white.png");

}

vector<unsigned short>& Calibrater::getMappedDepthFrame(IDepthFrame** depthFrame) {

	unsigned int depthSize = 0;
	unsigned short* depthBuff = nullptr;

	getDepthBuffer(depthFrame, &depthBuff, &depthSize);

	std::fill(depthBuffer.begin(), depthBuffer.end(), 0);
	for (int i = 0; i < depthSize; i++) {
		unsigned short depth = depthBuff[i];
		int mappedIndex = convertIndex(i);
		if(mappedIndex > -1)
			depthBuffer[mappedIndex] = depth;
	}

	return depthBuffer;

}

void Calibrater::Calibrate() {

	IColorFrame* colorFrame = nullptr;

	//std::fill(mappedColorBuffer.begin(), mappedColorBuffer.end(), 0);

	getColorBuffer(&colorFrame, mappedColorBuffer);
	//devTexture.loadData(&mappedColorBuffer[0], depth_width, depth_height, GL_BGRA);

	findSquare();
	setCorner();

	cornersCalibrated++;

	if (colorFrame)
		colorFrame->Release();

}

void Calibrater::Draw() {

	int s = 20;
	int h = ofGetViewportHeight();
	int w = ofGetViewportWidth();

	if (!calibrated) {

		boxaroo.draw(w * (cornersCalibrated % 2) - s / 2, h * (cornersCalibrated / 2) - s / 2, s, s);

		//if(cornersCalibrated > 0)
	}
}

int Calibrater::convertIndex(int incomingImage) {

	// we know 4 corners. we know the x,y position that would have corresponded to.
	// 

	//TODO: we should only be dealing with depth width and depth height.
	int originalXPosition = (incomingImage / 4) % color_width;
	int originalYPosition = (incomingImage / 4) / color_width;

	if (originalXPosition > corners.topLeft.x)
		return -1;
	if(originalXPosition < corners.topRight.x)
		return -1;
	if (originalYPosition < corners.topLeft.y)
		return -1;
	if (originalYPosition > corners.bottomRight.y)
		return -1;

	// were in the money zone 
	// we know we need to 

	int translatedX = originalXPosition - corners.topRight.x;
	int translatedY = originalYPosition - corners.topRight.y;

	// flip this guy. 0,0 => (newWidth, newHeight)

	int flippedX = newWidth - translatedX;
	//int flippedY = newHeight - translatedY;
	int flippedY = translatedY;

	// convert this to a buffer position.

	return (flippedY * newWidth + flippedX) * 4;
}

void Calibrater::setCorner() {

	switch (cornersCalibrated % 4) {
	case 0:
		ofLog(OF_LOG_VERBOSE, "setting top left");
		corners.topLeft = getCenterOfSquare();
		ofLog(OF_LOG_VERBOSE, "top left: %f, %f", corners.topLeft.x, corners.topLeft.y);
		break;
	case 1:
		ofLog(OF_LOG_VERBOSE, "setting top right");
		corners.topRight = getCenterOfSquare();
		ofLog(OF_LOG_VERBOSE, "top right: %f, %f", corners.topRight.x, corners.topRight.y);
		break;
	case 2:
		ofLog(OF_LOG_VERBOSE, "setting bottom left");
		corners.bottomLeft = getCenterOfSquare();
		ofLog(OF_LOG_VERBOSE, "bottom left: %f, %f", corners.bottomLeft.x, corners.bottomLeft.y);
		break;
	case 3:
		ofLog(OF_LOG_VERBOSE, "setting bottom right");
		corners.bottomRight = getCenterOfSquare();
		ofLog(OF_LOG_VERBOSE, "bottom right: %f, %f", corners.bottomRight.x, corners.bottomRight.y);
		break;
	}

}
void Calibrater::findSquare() {

	// b,g,r,a values - colorbuffer[0] is the b value of first pixel. 
	// we will loop and always keep i as the blue value of center of the grid.

	// what if we assume its the brightest spot?

	//filteredColorBuffer = mappedColorBuffer;

	const int max = 255 * 3;

	double brightest = 0;
	int brightestIndex = -1;
	//for (int x = 0; x < 5; x++) {

		for (int i = (depth_width + 1) * 4; i < mappedColorBuffer.size() - (depth_width + 1)*4; i += 4) {

			try {

				// 3-pixel wide row above center 
				const unsigned int a1_b = mappedColorBuffer[i - (depth_width + 1) * 4];
				const unsigned int a1_g = mappedColorBuffer[i - (depth_width + 1) * 4 + 1];
				const unsigned int a1_r = mappedColorBuffer[i - (depth_width + 1) * 4 + 2];
				const double a1_ratio = (double)(a1_b + a1_g + a1_r) / (double)(max);
				//ofLog(OF_LOG_VERBOSE, "%f", a1_ratio);

				const unsigned int a2_b = mappedColorBuffer[i - depth_width * 4];
				const unsigned int a2_g = mappedColorBuffer[i - depth_width * 4 + 1];
				const unsigned int a2_r = mappedColorBuffer[i - depth_width * 4 + 2];
				const double a2_ratio = (double)(a2_b + a1_g + a1_r) / (double)(max);

				const unsigned int a3_b = mappedColorBuffer[i - (depth_width - 1) * 4];
				const unsigned int a3_g = mappedColorBuffer[i - (depth_width - 1) * 4 + 1];
				const unsigned int a3_r = mappedColorBuffer[i - (depth_width - 1) * 4 + 2];
				const double a3_ratio = (double)(a3_b + a3_g + a3_r) / (double)(max);

				// current row of pixel
				const unsigned int a4_b = mappedColorBuffer[i - 4];
				const unsigned int a4_g = mappedColorBuffer[i - 3];
				const unsigned int a4_r = mappedColorBuffer[i - 2];
				const double a4_ratio = (double)(a4_b + a4_g + a4_r) / (double)(max);

				const unsigned int a5_b = mappedColorBuffer[i]; // center
				const unsigned int a5_g = mappedColorBuffer[i + 1]; // center
				const unsigned int a5_r = mappedColorBuffer[i + 2]; // center
				const double a5_ratio = (double)(a5_b + a5_g + a5_r) / (double)(max);

				const unsigned int a6_b = mappedColorBuffer[i + 4];
				const unsigned int a6_g = mappedColorBuffer[i + 5];
				const unsigned int a6_r = mappedColorBuffer[i + 6];
				const double a6_ratio = (double)(a6_b + a6_g + a6_r) / (double)(max);


				// row below pixel
				const unsigned int a7_b = mappedColorBuffer[i + (depth_width - 1) * 4 + 0];
				const unsigned int a7_g = mappedColorBuffer[i + (depth_width - 1) * 4 + 1];
				const unsigned int a7_r = mappedColorBuffer[i + (depth_width - 1) * 4 + 2];
				const double a7_ratio = (double)(a7_b + a7_g + a7_r) / (double)(max);

				const unsigned int a8_b = mappedColorBuffer[i + depth_width * 4 + 0];
				const unsigned int a8_g = mappedColorBuffer[i + depth_width * 4 + 1];
				const unsigned int a8_r = mappedColorBuffer[i + depth_width * 4 + 2];
				const double a8_ratio = (double)(a8_b + a8_g + a8_r) / (double)(max);

				const unsigned int a9_b = mappedColorBuffer[i + (depth_width + 1) * 4 + 0];
				const unsigned int a9_g = mappedColorBuffer[i + (depth_width + 1) * 4 + 1];
				const unsigned int a9_r = mappedColorBuffer[i + (depth_width + 1) * 4 + 2];
				const double a9_ratio = (double)(a9_b + a9_g + a9_r) / (double)(max);

				//if (a1_ratio + a2_ratio + a3_ratio + a4_ratio + a6_ratio + a7_ratio + a8_ratio + a9_ratio > (.6 * 8) && a5_ratio > 0.5) {
				/*
				if(a5_ratio > 0.9) { 
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
				*/
				const double allRatio = a1_ratio + a2_ratio + a3_ratio + a4_ratio + a5_ratio + a6_ratio + a7_ratio + a8_ratio + a9_ratio;

				if (allRatio > brightest) {
					brightest = allRatio;
					brightestIndex = i;
				}

			}
			catch (exception e) {
				ofLog(OF_LOG_VERBOSE, "fuckin error mate: index: %d, px_width: %d, px_height: %d", i, color_width, color_height);
			}
		}

		//mappedColorBuffer = filteredColorBuffer;
		for (int i = 0; i < mappedColorBuffer.size(); i += 4) {
			if (i == brightestIndex) {
				filteredColorBuffer[i] = 255;
				filteredColorBuffer[i + 1] = 255;
				filteredColorBuffer[i + 2] = 255;
				filteredColorBuffer[i + 3] = 255;
			}
			else {
				filteredColorBuffer[i] = 0;
				filteredColorBuffer[i + 1] = 0;
				filteredColorBuffer[i + 2] = 0;
				filteredColorBuffer[i + 3] = 255;
			}
		}
	//}
}

ofVec2f Calibrater::getCenterOfSquare() {
	
	double avg_x = 0;
	double avg_y = 0;

	int blues = 0;
	for (int i = 0; i < filteredColorBuffer.size(); i += 4) {
		if (filteredColorBuffer[i] == 255)
			blues++;
	}

	for (int i = 0; i < filteredColorBuffer.size(); i += 4)
	{
		const unsigned int blue_val = filteredColorBuffer[i];
		if (blue_val == 255) {
			int px_val = i / 4;
			int x = px_val % depth_width;
			int y = px_val / depth_width;

			avg_x += (double)x / blues;
			avg_y += (double)y / blues;
		}
	}

	ofLog(OF_LOG_VERBOSE, "px_width: %d, px_height: %d, blues: %d", color_width, color_height, blues);

	ofLog(OF_LOG_VERBOSE, "(%f, %f)", avg_x, avg_y);
	return ofVec2f(avg_x, avg_y);
}

// returns depth-mapped color buffer
void Calibrater::getColorBuffer(IColorFrame** colorFrame, std::vector<BYTE>& inputVector) {

	if (inputVector.size() != depth_width * depth_height * bpp) {
		inputVector.resize(depth_width * depth_height * bpp);
	}

	if (rawColorBuffer.size() != color_width * color_height * bpp) {
		rawColorBuffer.resize(color_width * color_height * bpp);
	}

	unsigned int depthSize = 0;
	unsigned short* depthBuff = nullptr;
	IDepthFrame* df = nullptr;

	getDepthBuffer(&df, &depthBuff, &depthSize);

	HRESULT hr;

	int attempts = 0;
	do {
		if (attempts++ > 0) {
			ofLog(OF_LOG_VERBOSE, "colorbuffer attempts: %d", attempts);
			Sleep(1000);
		}

		hr = colorFrameReader->AcquireLatestFrame(colorFrame);
		if (SUCCEEDED(hr)) {
			ofLogVerbose("aquired color frame");
			hr = (*colorFrame)->CopyConvertedFrameDataToArray(rawColorBuffer.size(), &rawColorBuffer[0], ColorImageFormat::ColorImageFormat_Bgra);
		}
		if (SUCCEEDED(hr)) {
			ofLogVerbose("copied converted data to array");
			hr = cMapper->MapColorFrameToDepthSpace(depth_width*depth_height, depthBuff, color_width*color_height, &colorToDepthBuffer[0]);
		}
		if (SUCCEEDED(hr)) {
			ofLogVerbose("mapped color frame to depth space");
			for (int i = 0; i < rawColorBuffer.size(); i += 4) {
				const DepthSpacePoint mapped = colorToDepthBuffer[i/4];
				int depthX = (int)(mapped.X + 0.5f);
				int depthY = (int)(mapped.Y + 0.5f);
				
				const int mappedIndex = (depthX + (depthY * depth_width)) * 4;

				if (mapped.X < 0 || mapped.X > depth_width || mapped.Y < 0 || mapped.Y > depth_height)
					continue;

				inputVector[mappedIndex] = rawColorBuffer[i];
				inputVector[mappedIndex + 1] = rawColorBuffer[i + 1];
				inputVector[mappedIndex + 2] = rawColorBuffer[i + 2];
				inputVector[mappedIndex + 3] = rawColorBuffer[i + 3];
			}

			/*
			for (int i = 0; i < colorToDepthBuffer.size(); i++) {
				const DepthSpacePoint mapped = colorToDepthBuffer[i];

				int depthX = (int)(mapped.X + 0.5f);
				int depthY = (int)(mapped.Y + 0.5f);

				if (depthX >= 0 && depthX < depth_width && depthY >= 0 && depthY < depth_height)
				{
					int depthIndex = (depthY * depth_width) + depthX;
					int colorIndex = i * 4;

					inputVector[depthIndex] = rawColorBuffer[colorIndex + 0];
					inputVector[depthIndex + 1] = rawColorBuffer[colorIndex + 1];
					inputVector[depthIndex + 2] = rawColorBuffer[colorIndex + 2];
					inputVector[depthIndex + 3] = rawColorBuffer[colorIndex + 3];
				}

			}*/
		}
		
	} while (FAILED(hr));


	if (df) {
		df->Release();
	}
}

void Calibrater::getDepthBuffer(IDepthFrame** depthFrame, unsigned short** depthBuff, unsigned int* size) {

	HRESULT hr;

	int attempts = 0;
	do {
		if (attempts++ > 0) {
			ofLog(OF_LOG_VERBOSE, "depthbuffer attempts: %d", attempts);
		}

		hr = depthFrameReader->AcquireLatestFrame(depthFrame);
		if (SUCCEEDED(hr)) {
			hr = (*depthFrame)->AccessUnderlyingBuffer(size, depthBuff);
		}

	} while (FAILED(hr));
}