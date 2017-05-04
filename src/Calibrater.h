#include <vector>

#include "ofMain.h"
#include "Kinect.h"

using namespace std;

struct Corners {
	ofVec3f topLeft;
	ofVec3f topRight;
	ofVec3f bottomLeft;
	ofVec3f bottomRight;
};

class Calibrater {
public:

	Calibrater(IKinectSensor* kinect);

	void Calibrate();
	void Draw();

	vector<unsigned short>& getMappedDepthFrame(IDepthFrame** depthFrame);

	int getMappedWidth();
	int getMappedHeight();

	void getColorBuffer(IColorFrame** cf, vector<BYTE>& inputVector);
	void getDepthBuffer(IDepthFrame** df, unsigned short** depthBuff, unsigned int* size);

	void setCorners(Corners _corners);

	int convertIndex(int x, short d);

private:
	IKinectSensor* kinect;

	IColorFrameSource* colorFrameSource;
	IColorFrameReader* colorFrameReader;
	IFrameDescription* fd;

	IDepthFrameSource* depthFrameSource;
	IDepthFrameReader* depthFrameReader;
	IFrameDescription* depthFd;

	ICoordinateMapper* cMapper;

	int color_width;
	int color_height;
	unsigned int bpp;

	int depth_width;
	int depth_height;

	int newWidth;
	int newHeight;

	ofTexture devTexture;

	vector<BYTE> rawColorBuffer;
	vector<BYTE> mappedColorBuffer;
	vector<BYTE> filteredColorBuffer;
	vector<DepthSpacePoint> colorToDepthBuffer;
	vector<unsigned short> depthBuffer;

	bool calibrated = false;
	Corners corners;

	ofImage boxaroo;

	int cornersCalibrated = 0;

	void findSquare();
	ofVec3f getCenterOfSquare(unsigned short* depthBuff);
	void setCorner(unsigned short* depthBuff);
};
