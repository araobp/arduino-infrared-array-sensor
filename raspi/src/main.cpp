/**
Reference for serial port on Linux:
https://github.com/xanthium-enterprises/Serial-Port-Programming-on-Linux/blob/master/USB2SERIAL_Read/Reciever%20(PC%20Side)/SerialPort_read.c
*/

#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>

/*
 * Port connected to Arduino
 */
//#define PORT "/dev/serial/by-id/usb-Arduino_Srl_Arduino_Uno_8543533323135181C251-if00"
#define PORT "/dev/ttyACM0"

// Frame delimiter (assuming that the temperature is smaller that 0xFE)
#define BEGIN 0xFE
#define END 0xFF

#define MAGNIFY 64 

using namespace std;
using namespace cv;

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
      //putText(src, temp[i++], Point(xx, yy), font, 1, Scalar(255,255,255), 1, LINE_AA);
      string &t = temp.at(i);
      putText(src, t, Point(xx, yy), font, 1, Scalar(255,255,255), 2, LINE_AA);
      i++;
    }
  }
}

int main(int argc, char* argv[]) {

  int fd;
  struct termios settings;

  fd = open(PORT, O_RDONLY | O_NOCTTY);

  if (fd == -1) {
    cout << "Error in opening tty" << endl;
    exit(-1);
  }

  // Serial port settings
  tcgetattr(fd, &settings);

  cfsetospeed(&settings,B115200);

  settings.c_cflag &= ~PARENB;
  settings.c_cflag &= ~CSTOPB;
  settings.c_cflag &= ~CSIZE;
  settings.c_cflag |=  CS8;  
  settings.c_cflag &= ~CRTSCTS;
  settings.c_cflag |= CREAD | CLOCAL;
  settings.c_iflag &= ~(IXON | IXOFF | IXANY);
  settings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  settings.c_oflag &= ~OPOST;

  // Blocking mode
  settings.c_cc[VMIN] = 1;
  settings.c_cc[VTIME] = 0;

  if((tcsetattr(fd,TCSANOW,&settings)) != 0) {
    cout << "Error in setting attributes" << endl;
    exit(-1);
  }

  tcflush(fd, TCIFLUSH);

  // Image 
  char buf[1];
  int bytes_read = 0;
  uint8_t frameBuf[64];
  int idx = 0;

  Mat img(8, 8, CV_8U, frameBuf);
  Mat enlarged(Size(8*MAGNIFY, 8*MAGNIFY), CV_8U);
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
        blur(enlarged, enlarged, Size(11,11), Point(-1,-1));
        enlarged.convertTo(colored, CV_8UC3);
        applyColorMap(colored, colored, COLORMAP_JET);
        if (idx >= 64) {
          putTempText(colored, temp);
        }
        imshow("Thermography", colored);
        char c = (char)waitKey(25);
        if (c == 'q') {
          close(fd);
          exit(0);
        }
      } else {
        frameBuf[idx++] = buf[i];
        temp.push_back(to_string(buf[i]/4));
      }
#endif
    }
  }
}

