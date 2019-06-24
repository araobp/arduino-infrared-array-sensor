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

using namespace std;
using namespace cv;

// Command line options
struct {
  bool withTemp;
  bool applyBlur;
  int magnification;
  int repeat;
  bool applyBinalization;
  bool applyColorMapHot;
} args;

const char optString[] = "tbm:i:BH";

// Display command usage
void displayUsage(void) {
  cout << "Usage: thermo [OPTION...]" << endl;
  cout << "" << endl;
  cout << "  -t                 show thermography with temperature overlaied" << endl;
  cout << "  -m magnification   magnify image" << endl;
  cout << "  -i repeat          apply bicubic interpolation (4^repeat magnified)" << endl;
  cout << "  -b                 apply blur effect" << endl;
  cout << "  -B                 apply binalization" << endl;
  cout << "  -H                 apply COLORMAP_HOT" << endl;
  cout << "  -?                 show this help" << endl;
}

// Command argument parser based on unistd.h
void argparse(int argc, char * argv[]) {

  args.withTemp = false;
  args.applyBlur = false;
  args.magnification = 16;
  args.repeat = 0;
  args.applyBinalization = false;
  args.applyColorMapHot = false;

  int opt;
  opterr = 0;  // disable getopt() error message output

  while ((opt = getopt(argc, argv, optString)) != -1) {
    switch(opt){
      case 't':
        args.withTemp = true;
        break;
      case 'b':
        args.applyBlur = true;
        break;
      case 'm':
        args.magnification = atoi(optarg);
        break;
      case 'i':
        args.repeat = atoi(optarg);
        break;
      case 'B':
        args.applyBinalization = true;
        break;
      case 'H':
        args.applyColorMapHot = true;
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
  int idx = 0;

  Mat img(8, 8, CV_8U, frameBuf);
  Mat interpolated;
  Mat magnified = Mat(Size(8*args.magnification*(int)pow(4.0,(float)args.repeat), 8*args.magnification*(int)pow(4.0,(float)args.repeat)), CV_8U);
  Mat colored;
  vector<string> temp;
  Mat labels;
  vector<string> labelsStr;

  while (true) {
    bytes_read = read(fd, &buf, 1);
    for (int i = 0; i < bytes_read; i++) {
#ifdef CHAR_FORMAT
      if (buf[i] == ',') {
        cout << endl;
      } else {
        cout << buf[i];
      }
#else
      if (buf[i] == BEGIN) {
        idx = 0;
        temp.clear();
      } else if (buf[i] == END){
        normalize(img, img , 0, 255, NORM_MINMAX);
        if (args.repeat > 0) {
          interpolate(img, interpolated, args.repeat);
          magnify(interpolated, magnified, args.magnification);
        } else {
          if (args.applyBinalization) {
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
        if (args.applyBlur) {
          blur(magnified, magnified, Size(11,11), Point(-1,-1));
        }
        magnified.convertTo(colored, CV_8UC3);
        if (args.applyColorMapHot) {
          applyColorMap(colored, colored, COLORMAP_HOT);
        } else {
          applyColorMap(colored, colored, COLORMAP_JET);
        }
        if (idx >= 64) {
          if (!args.repeat) {
            if (args.withTemp) {
              putTempText(colored, args.magnification, temp, args.applyColorMapHot);
            } else if (args.applyBinalization) {
              putTempText(colored, args.magnification, labelsStr, args.applyColorMapHot);
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
        frameBuf[idx++] = (uint8_t)buf[i];
        temp.push_back(to_string((buf[i]+2)/4));  // in Celsius (x 0.25)
      }
#endif
    }
  }
}

