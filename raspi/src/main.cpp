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
} commandArgs;

const char optString[] = "t";

// Display command usage
void displayUsage(void) {
  cout << "Usage: thermo [OPTION...]" << endl;
  cout << "" << endl;
  cout << "-t               show thermography with temperature overlaied" << endl;
  cout << "-?               show this help" << endl;
}

// Command argument parser based on unistd.h
void argparse(int argc, char * argv[]) {

  commandArgs.withTemp = false;

  int opt;
  opterr = 0;  // disable getopt() error message output

  while ((opt = getopt(argc, argv, optString)) != -1) {
    switch(opt){
      case 't':
        commandArgs.withTemp = true;
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
      for (int yy = 0; yy < MAGNIFY; yy++) {
        for (int xx = 0; xx < MAGNIFY; xx++) {
          dst.at<uint8_t>(y*MAGNIFY+yy, x*MAGNIFY+xx) = pixel;
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
  int x_offset = 12;
  int y_offset = MAGNIFY - 22;
  int xx, yy;
  int i = 0;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      xx = x_offset + x * MAGNIFY;
      yy = y_offset + y * MAGNIFY;
      string &t = temp.at(i);
      putText(src, t, Point(xx, yy), font, 1, Scalar(255,255,255), 2, LINE_AA);
      i++;
    }
  }
}

int main(int argc, char* argv[]) {

  int fd = openSerialPort();

  // Image processing
  char buf[1];
  int bytes_read = 0;
  uint8_t frameBuf[64];
  int idx = 0;

  Mat img(8, 8, CV_8U, frameBuf);
  Mat enlarged(Size(8*MAGNIFY, 8*MAGNIFY), CV_8U);
  Mat colored;
  vector<string> temp;

  argparse(argc, argv);

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
        blur(enlarged, enlarged, Size(11,11), Point(-1,-1));
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

