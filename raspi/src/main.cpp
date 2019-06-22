#include <iostream>
#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <unistd.h>
#include "serialport.hpp"

// Frame delimiter (assuming that the temperature is smaller that 0xFE)
#define BEGIN 0xFE
#define END 0xFF

using namespace std;
using namespace cv;

// Command line options
struct {
  bool withTemp;
  bool applyBlur;
  int magnification;
  int applyInterpolation;
  bool applyBinalization;
} args;

const char optString[] = "tbm:iB";

// Display command usage
void displayUsage(void) {
  cout << "Usage: thermo [OPTION...]" << endl;
  cout << "" << endl;
  cout << "-t               show thermography with temperature overlaied" << endl;
  cout << "-m magnification magnify image" << endl;
  cout << "                 ..without interpolation: (8 x m)^2 pixels" << endl;
  cout << "                 ..with interpolation: (4 x 4^m)^2 pixels" << endl;
  cout << "-i               apply bicubic interpolation" << endl;
  cout << "-b               apply blur effect" << endl;
  cout << "-B               apply binalization" << endl;
  cout << "-?               show this help" << endl;
}

// Command argument parser based on unistd.h
void argparse(int argc, char * argv[]) {

  args.withTemp = false;
  args.applyBlur = false;
  args.magnification = 32;
  args.applyInterpolation = false;
  args.applyBinalization = false;

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

/*
 * Enlarge image
 */
void enlarge(Mat &src, Mat &dst, int magnification, bool interpolation=false) {
  uint8_t pixel;
  int size;
  if (!interpolation) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        pixel = src.at<uint8_t>(y,x);
        for (int yy = 0; yy < magnification; yy++) {
          for (int xx = 0; xx < magnification; xx++) {
            dst.at<uint8_t>(y*magnification+yy, x*args.magnification+xx) = pixel;
          }
        }
      }
    }
  } else {
    resize(src, dst, Size(8*4, 8*4), 0, 0, INTER_CUBIC);
    for (int i = 2.0; i <= (float)magnification; i += 1.0) {
      size = (int)pow(4.0, i);
      resize(dst, dst, Size(size, size), 0, 0, INTER_CUBIC);
    }
  }
}

/*
 * Super-impose temperature data on the image
 */
void putTempText(Mat &src, int magnification, vector<string> &temp) {

  int font = FONT_HERSHEY_SIMPLEX;
  int x_offset = 12*magnification/64;
  int y_offset = magnification - 22*magnification/64;
  int xx, yy;
  int i = 0;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      xx = x_offset + x * magnification;
      yy = y_offset + y * magnification;
      string &t = temp.at(i);
      putText(src, t, Point(xx, yy), font, (float)magnification/64.0, Scalar(255,255,255), 1+magnification/64, LINE_AA);
      i++;
    }
  }
}

int main(int argc, char* argv[]) {

  argparse(argc, argv);

  int fd = openSerialPort();

  // Image processing
  char buf[1];
  int bytes_read = 0;
  uint8_t frameBuf[64];
  int idx = 0;

  Mat img(8, 8, CV_8U, frameBuf);
  Mat enlarged;
  if (!args.applyInterpolation) { 
    enlarged = Mat(Size(8*args.magnification, 8*args.magnification), CV_8U);
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
        enlarge(img, enlarged, args.magnification, args.applyInterpolation);
        if (args.applyBinalization) {
          threshold(enlarged, enlarged, 130, 255, THRESH_BINARY);
        }
        if (args.applyBlur) {
          blur(enlarged, enlarged, Size(11,11), Point(-1,-1));
        }
        enlarged.convertTo(colored, CV_8UC3);
        applyColorMap(colored, colored, COLORMAP_JET);
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

