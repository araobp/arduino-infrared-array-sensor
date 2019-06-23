#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void magnify(Mat &src, Mat &dst, int magnification, bool interpolation=false); 

void putTempText(Mat &src, int magnification, vector<string> &temp); 
