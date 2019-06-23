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
  int applyInterpolation;
  bool applyBinalization;
  bool applyColorMapHot;
} args;

const char optString[] = "tbm:iBH";

// Display command usage
void displayUsage(void) {
  cout << "Usage: thermo [OPTION...]" << endl;
  cout << "" << endl;
  cout << "-t               show thermography with temperature overlaied" << endl;
  cout << "-m magnification magnify image" << endl;
  cout << "                 ..without interpolation: (8 x m)^2 pixels" << endl;
  cout << "                 ..with interpolation: (8 x 4^m)^2 pixels" << endl;
  cout << "-i               apply bicubic interpolation" << endl;
  cout << "-b               apply blur effect" << endl;
  cout << "-B               apply binalization" << endl;
  cout << "-H               apply COLORMAP_HOT" << endl;
  cout << "-?               show this help" << endl;
}

// Command argument parser based on unistd.h
void argparse(int argc, char * argv[]) {

  args.withTemp = false;
  args.applyBlur = false;
  args.magnification = 32;
  args.applyInterpolation = false;
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
        args.applyInterpolation = true;
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

  if (args.applyInterpolation && (args.magnification > 4)) {
    cout << "magnification must not be larger than 4 for interpolation!" << endl;
    exit(-1);
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
  Mat magnified;
  if (!args.applyInterpolation) { 
    magnified = Mat(Size(8*args.magnification, 8*args.magnification), CV_8U);
  }
  Mat colored;
  vector<string> temp;

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
        magnify(img, magnified, args.magnification, args.applyInterpolation);
        if (args.applyBinalization) {
          threshold(magnified, magnified, BIN_THRES, 255, THRESH_BINARY);
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
          if (!args.applyInterpolation && args.withTemp) {
            putTempText(colored, args.magnification, temp);
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

