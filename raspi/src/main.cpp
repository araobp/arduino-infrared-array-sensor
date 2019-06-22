#include <iostream>
#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <unistd.h>
#include "main.hpp"
#include "serialport.hpp"

// Frame delimiter (assuming that the temperature is smaller that 0xFE)
#define BEGIN 0xFE
#define END 0xFF

using namespace std;
using namespace cv;

// Command line options
struct {
  bool withTemp;
  bool enableBlur;
  int magnify;
} commandArgs;

const char optString[] = "tbs:";

// Display command usage
void displayUsage(void) {
  cout << "Usage: thermo [OPTION...]" << endl;
  cout << "" << endl;
  cout << "-t               show thermography with temperature overlaied" << endl;
  cout << "-b               enable blur effect" << endl;
  cout << "-s magnify       size ((8 x n)^2 pixels)" << endl;
  cout << "-?               show this help" << endl;
}

// Command argument parser based on unistd.h
void argparse(int argc, char * argv[]) {

  commandArgs.withTemp = false;
  commandArgs.withTemp = false;
  commandArgs.magnify = MAGNIFY;

  int opt;
  opterr = 0;  // disable getopt() error message output

  while ((opt = getopt(argc, argv, optString)) != -1) {
    switch(opt){
      case 't':
        commandArgs.withTemp = true;
        break;
      case 'b':
        commandArgs.enableBlur = true;
        break;
      case 's':
        commandArgs.magnify = atoi(optarg);
        break;
      default:
        displayUsage();
        exit(1);
        break;
    }
  }
}

/*
 * Enlarge image: 32 times larger that the origial
 */
void enlarge(Mat &src, Mat &dst) {
  uint8_t pixel;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      pixel = src.at<uint8_t>(y,x);
      for (int yy = 0; yy < commandArgs.magnify; yy++) {
        for (int xx = 0; xx < commandArgs.magnify; xx++) {
          dst.at<uint8_t>(y*commandArgs.magnify+yy, x*commandArgs.magnify+xx) = pixel;
        }
      }
    }
  }
}

/*
 * Super-impose temperature data on the image
 */
void putTempText(Mat &src, vector<string> &temp) {

  int font = FONT_HERSHEY_SIMPLEX;
  int x_offset = 12*commandArgs.magnify/64;
  int y_offset = commandArgs.magnify - 22*commandArgs.magnify/64;
  int xx, yy;
  int i = 0;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      xx = x_offset + x * commandArgs.magnify;
      yy = y_offset + y * commandArgs.magnify;
      string &t = temp.at(i);
      putText(src, t, Point(xx, yy), font, (float)commandArgs.magnify/64.0, Scalar(255,255,255), 2, LINE_AA);
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
  Mat enlarged(Size(8*commandArgs.magnify, 8*commandArgs.magnify), CV_8U);
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
        enlarge(img, enlarged);
        if (commandArgs.enableBlur) {
          blur(enlarged, enlarged, Size(11,11), Point(-1,-1));
        }
        enlarged.convertTo(colored, CV_8UC3);
        applyColorMap(colored, colored, COLORMAP_JET);
        if (idx >= 64) {
          if (commandArgs.withTemp) {
            putTempText(colored, temp);
          }
          imshow("Thermography", colored);
        }
        char c = (char)waitKey(25);
        if (c == 'q') {
          close(fd);
          exit(0);
        }
      } else {
        frameBuf[idx++] = buf[i];
        temp.push_back(to_string((buf[i]+2)/4));  // in Celsius (x 0.25)
      }
#endif
    }
  }
}

