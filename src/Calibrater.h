#include <vector>

#include "ofMain.h"
#include "Kinect.h"

using namespace std;

struct Corners {
	ofVec2f topLeft;
	ofVec2f topRight;
	ofVec2f bottomLeft;
	ofVec2f bottomRight;
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

	int convertIndex(int x);

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

	ofVec2f getCenterOfSquare();
	void findSquare();
	void setCorner();
};
