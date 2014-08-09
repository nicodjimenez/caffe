/*
 * DisplayImage.cpp
 *
 *  Created on: Aug 3, 2014
 *      Author: nicodjimenez
 */
#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

//int charSize = 28;
//int halfSize = imSize / 2;
//double DOWNSIZE_FACTOR=0.7;

Rect bin_bounding_rect(Mat& image){
/* Scans a binarized greyscale image for black pixels.  
*  Returns bounding box for all nonwhite pixels.
*/
  int channels = image.channels();
  CV_Assert(channels == 1);
  //cout << "Number of channels: " << channels << endl;
  int nRows = image.rows;
  int nCols = image.cols;
  int jmax = 0,jmin = nCols;
  int imax = 0,imin = nRows;
  uchar* row;
  for ( int i = 0; i < nRows; ++i){
    row = image.ptr<uchar>(i);
    for ( int j = 0; j< nCols; ++j){
      if (row[j] > 0){
        jmax = max(jmax,j);
	jmin = min(jmin,j);
	imax = max(imax,i);
	imin = min(imin,i);
	}
     }
  }
  // we need to add 1 to get proper width and height
  Rect rect_out(jmin,imin,1+jmax-jmin,1+imax-imin);
  return rect_out;
}

void resize_roi(Mat& image,const int charSize){
  Mat resized_image;
  double nRows = image.rows;
  double nCols = image.cols;
  double size_factor = charSize / max(nRows,nCols); 
  //cout << "Size factor: " << size_factor << endl;
  int nRows_new = ceil(nRows * size_factor);
  int nCols_new = ceil(nCols * size_factor);
  //cout << "New rows: " << nRows_new << endl;
  //cout << "New cols: " << nCols_new << endl;
  Size size_new(nCols_new,nRows_new);
  resize( image, image, size_new);
  //cout << "Just resized image" << endl;
}

Point get_com(Mat& image){
  // returns center of mass coordinate point of a greyscale image
  int channels = image.channels();
  CV_Assert(channels == 1);
  int nRows = image.rows;
  int nCols = image.cols;
  int pix_val;
  double pix_sum=0, xcom=0, ycom=0;
  uchar* row;
  for ( int i = 0; i < nRows; ++i){
    row = image.ptr<uchar>(i);
    for ( int j = 0; j< nCols; ++j){
      pix_val = row[j];
      ycom += pix_val * i; 
      xcom += pix_val * j;
      pix_sum += pix_val;
    }
  }
  ycom = ycom / pix_sum;
  xcom = xcom / pix_sum;
  Point com_point = Point( xcom, ycom); 
  return com_point;
}

Mat CropImage(Mat& image, const int imSize, const int charSize){
  // takes raw greyscale (white on black) image and returns centered, resized image
  Rect bound_rect = bin_bounding_rect(image);
  //cout << "Just computed bounding rectangle..." << endl;
  //cout << "bound_rect.x = " << bound_rect.x << endl;
  // cout << "bound_rect.y = " << bound_rect.y << endl;
 // cout << "bound_rect.width = " << bound_rect.width << endl;
 // cout << "bound_rect.height = " << bound_rect.height << endl;
  static const double halfSize = imSize / 2;
  Rect sub_image_rect;
  Mat big_image = Mat::zeros(imSize,imSize, CV_8UC1);
  Mat roi(image,bound_rect);
  //cout << "Just selected roi..." << endl;
  Mat big_image_roi; 
  //Point com_point;
  int xdiff,ydiff;
  double roi_midx, roi_midy;

  resize_roi(roi,charSize);	
  roi_midx = (roi.cols - 1.0) / 2.0;
  roi_midy = (roi.rows - 1.0) / 2.0;
  xdiff = halfSize - roi_midx;
  ydiff = halfSize - roi_midy;
  sub_image_rect = Rect( xdiff, ydiff, roi.cols, roi.rows);
  // COM centering method
  //com_point = get_com(roi);
  //xdiff = halfSize - com_point.x; 
  //ydiff = halfSize - com_point.y;
  //sub_image_rect = Rect( xdiff, ydiff, roi.cols, roi.rows);
  
  big_image_roi = big_image(sub_image_rect);
  roi.copyTo(big_image_roi);
  
  // debug
  //com_point = get_com(big_image);
  //cout << "New X com: " << com_point.x << endl;
  //cout << "New Y com: " << com_point.y << endl;

  return big_image;	
}

