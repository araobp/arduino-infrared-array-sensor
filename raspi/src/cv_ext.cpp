#include "cv_ext.hpp"

/*
 * Interpolate image
 *
 * The reason why I use this function instead of cv::resize() is that
 * the original image (8x8) is too small for bicubic interpolation.
 * I found that repeating bicubic interpolation works well.
 */
void interpolate(Mat &src, Mat &dst, int repeat) {
  resize(src, dst, Size(src.cols*4, src.rows*4), 0, 0, INTER_CUBIC);
  for (int i = 2.0; i <= (float)repeat; i += 1.0) {
    resize(dst, dst, Size(dst.cols*4, dst.rows*4), 0, 0, INTER_CUBIC);
  }
}

/*
 * Magnify image
 *
 * The reason why I use this function instead of cv::resize() is that
 * it is impossible to disable interpolation on cv::resize().
 * This function just supports magnification without interpolation.
 */
void magnify(Mat &src, Mat &dst, int magnification) {
  uint8_t pixel;
  for (int y = 0; y < src.rows; y++) {
    for (int x = 0; x < src.cols; x++) {
      pixel = src.at<uint8_t>(y,x);
      for (int yy = 0; yy < magnification; yy++) {
        for (int xx = 0; xx < magnification; xx++) {
          dst.at<uint8_t>(y*magnification+yy, x*magnification+xx) = pixel;
        }
      }
    }
  }
}

/*
 *  Super-impose temperature data on the image
 */
void putTempText(Mat &src, int magnification, vector<string> &temp, bool colorBlack, bool negative) {

  int font = FONT_HERSHEY_SIMPLEX;
  int x_offset = 12*magnification/64;
  int y_offset = magnification - 22*magnification/64;
  int xx, yy;
  int i = 0;
  int color = 255;
  float fontSize;

  if (colorBlack) {
  color = 0;
  }

  if (negative) {
    fontSize = (float)magnification/96.0;
  } else {
    fontSize = (float)magnification/64.0;
  }

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      xx = x_offset + x * magnification;
      yy = y_offset + y * magnification;
      string &t = temp.at(i);
      putText(src, t, Point(xx, yy), font, fontSize, Scalar(color,color,color), 1+magnification/64, LINE_AA);
      i++;
    }
  }
}

