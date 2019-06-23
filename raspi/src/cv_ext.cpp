#include "cv_ext.hpp"

/*
 * Magnify image
 */
void magnify(Mat &src, Mat &dst, int magnification, bool interpolation) {
  uint8_t pixel;
  int size;
  if (!interpolation) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        pixel = src.at<uint8_t>(y,x);
        for (int yy = 0; yy < magnification; yy++) {
          for (int xx = 0; xx < magnification; xx++) {
            dst.at<uint8_t>(y*magnification+yy, x*magnification+xx) = pixel;
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
 *  * Super-impose temperature data on the image
 *   */
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

