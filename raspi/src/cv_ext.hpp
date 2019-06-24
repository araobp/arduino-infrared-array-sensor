#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void interpolate(Mat &src, Mat &dst, int repeat=1); 

void magnify(Mat &src, Mat &dst, int magnification); 

void putTempText(Mat &src, int magnification, vector<string> &temp, bool colorBlack=false); 
