#include <iostream>
#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <unistd.h>
#include "serialport.hpp"
#include "cv_ext.hpp"

// Frame delimiter (assuming that the temperature is smaller that 0xFE)
#define BEGIN 0xFE
#define END 0xFF

#define BIN_THRES 130U
#define DIFF_MAGNIFICATION 5

using namespace std;
using namespace cv;

// Command line options
struct {
  bool withTemp;
  bool enableBlur;
  int magnification;
  int repeatInterpolation;
  bool enableBinalization;
  bool enableColorMapHot;
  bool enableDiff;
} args;

const char optString[] = "tbm:i:BHd";

// Display command usage
void displayUsage(void) {
  cout << "Usage: thermo [OPTION...]" << endl;
  cout << "" << endl;
  cout << "   -t                   show thermography with temperature overlaied" << endl;
  cout << "   -m magnificatio      magnify image" << endl;
  cout << "   -i repeat            repeat interpolation (4^repeat magnified)" << endl;
  cout << "   -b                   enable blur effect" << endl;
  cout << "   -d                   enable diff between frames" << endl;
  cout << "   -B                   enable binalization" << endl;
  cout << "   -H                   enable COLORMAP_HOT" << endl;
  cout << "   -?                   show this help" << endl;
}

// Command argument parser based on unistd.h
void argparse(int argc, char * argv[]) {

  args.withTemp = false;
  args.enableBlur = false;
  args.magnification = 16;
  args.repeatInterpolation = 0;
  args.enableBinalization = false;
  args.enableColorMapHot = false;
  args.enableDiff = false;

  int opt;
  opterr = 0;  // disable getopt() error message output

  while ((opt = getopt(argc, argv, optString)) != -1) {
    switch(opt){
      case 't':
        args.withTemp = true;
        break;
      case 'b':
        args.enableBlur = true;
        break;
      case 'm':
        args.magnification = atoi(optarg);
        break;
      case 'i':
        args.repeatInterpolation = atoi(optarg);
        break;
      case 'B':
        args.enableBinalization = true;
        break;
      case 'H':
        args.enableColorMapHot = true;
        break;
      case 'd':
        args.enableDiff = true;
        break;
      default:
        displayUsage();
        exit(1);
        break;
    }
  }
}

int main(int argc, char* argv[]) {

  // Parse command line arguments
  argparse(argc, argv);

  // Open serial port to get file descriptor
  int fd = openSerialPort();

  // Image processing
  char buf[1];
  int bytes_read = 0;
  uint8_t frameBuf[64];
  int prevFrame[64];
  int diff;
  int idx = 0;

  Mat img(8, 8, CV_8U, frameBuf);
  Mat interpolated;
  Mat magnified = Mat(Size(8*args.magnification*(int)pow(4.0,(float)args.repeatInterpolation), 8*args.magnification*(int)pow(4.0,(float)args.repeatInterpolation)), CV_8U);
  Mat colored;
  vector<string> temp;
  Mat labels;
  vector<string> labelsStr;

  while (true) {
    bytes_read = read(fd, &buf, 1);
    for (int i = 0; i < bytes_read; i++) {
      if (buf[i] == BEGIN) {
        idx = 0;
        temp.clear();
      } else if (buf[i] == END){
        if (!args.enableDiff) {
          normalize(img, img , 0, 255, NORM_MINMAX);
        }
        if (args.repeatInterpolation > 0) {
          interpolate(img, interpolated, args.repeatInterpolation);
          magnify(interpolated, magnified, args.magnification);
        } else {
          if (args.enableBinalization) {
            threshold(img, img, BIN_THRES, 255, THRESH_BINARY);
            connectedComponents(img, labels, 8, CV_16U);
            labelsStr.clear();
            for (int y = 0; y < img.rows; y++) {
              for (int x = 0; x < img.cols; x++) {
                labelsStr.push_back(to_string(labels.at<uint16_t>(y,x)));
              }
            }
          }
          magnify(img, magnified, args.magnification);
        }
        if (args.enableBlur) {
          blur(magnified, magnified, Size(11,11), Point(-1,-1));
        }
        magnified.convertTo(colored, CV_8UC3);
        if (args.enableColorMapHot) {
          applyColorMap(colored, colored, COLORMAP_HOT);
        } else {
          applyColorMap(colored, colored, COLORMAP_JET);
        }
        if (idx >= 64) {
          if (!args.repeatInterpolation) {
            if (args.enableDiff) {
              putTempText(colored, args.magnification, temp, args.enableColorMapHot, true);
            } else if (args.withTemp) {
              putTempText(colored, args.magnification, temp, args.enableColorMapHot);
            } else if (args.enableBinalization) {
              putTempText(colored, args.magnification, labelsStr, args.enableColorMapHot);
            }
          }
          imshow("Thermography", colored);
        }
        char c = (char)waitKey(25);
        if (c == 'q') {
          close(fd);
          exit(0);
        }
      } else {
        if (args.enableDiff) {
          diff = (int)buf[i] - prevFrame[idx];
          frameBuf[idx] = (uint8_t)(diff * DIFF_MAGNIFICATION + 128);  // add 0x80(128) so that negative values can be seen.
          prevFrame[idx++] = (int)buf[i];
          temp.push_back(to_string(diff));  // in Celsius (x 0.25)
        } else {
          frameBuf[idx++] = (uint8_t)buf[i];
          temp.push_back(to_string((buf[i]+2)/4));  // in Celsius (x 0.25)
        }
      }
    }
  }
}

