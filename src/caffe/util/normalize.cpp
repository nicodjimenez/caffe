/*
 * DisplayImage.cpp
 *
 *  Created on: Aug 3, 2014
 *      Author: nicodjimenez
 */
#include <iostream>
#include "caffe/util/normalize.hpp"
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_01.hpp>

//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "caffe/proto/caffe.pb.h"
//#include "caffe/common.hpp"

using namespace cv;
using namespace std;
using namespace caffe;

Mat rotate(Mat& image, double angle){
  Point center = Point(image.cols/2.0,image.rows/2.0);
  Mat rot_mat = getRotationMatrix2D(center, angle, 1.0);
  Rect bbox = RotatedRect(center, image.size(), angle).boundingRect();
  rot_mat.at<double>(0,2) += bbox.width/2.0 - center.x; 
  rot_mat.at<double>(1,2) += bbox.height/2.0 - center.y;
  Mat dst;
  warpAffine(image, dst, rot_mat, bbox.size());
  return dst;
}

Mat rand_dilate_image(Mat& image){
  // randomly dilate image 
  //cout << "Maybe will distort image..." << endl;
  static boost::minstd_rand intgen;
  static boost::uniform_01<boost::minstd_rand> gen(intgen);
  double sample = gen();
  if (sample > 0.5){
    //cout << "Dilating image..." << endl;
    dilate(image,image,Mat());
  }
  return image;
}

Mat distort_image(Mat& image){
  // random rotation plus random resize 
  static boost::minstd_rand intgen;
  static boost::uniform_01<boost::minstd_rand> gen(intgen);
  
  // rotate image
  double angle = 12*(gen() - 0.5); 
  Mat new_image = rotate( image, angle);

  // now resize image
  double dist = 0.15;
  double nRows = new_image.rows;
  double nCols = new_image.cols;
  int nCols_new, nRows_new;
 
  if (nRows > 7)
    nRows_new = ceil(nRows * (1 + dist*(2*gen()-1)));
  else
    nRows_new = nRows;
  
  if (nCols > 7)
    nCols_new = ceil(nCols * (1 + dist*(2*gen()-1)));
  else
    nCols_new = nCols;

  Size size_new(nCols_new,nRows_new);
  resize( new_image, new_image, size_new, 0, 0, INTER_AREA);
  
  // rotate image
  angle = 12*(gen() - 0.5); 
  new_image = rotate( new_image, angle);
  new_image = crop_image(new_image);
  
  return new_image;
}

Mat crop_image(Mat& image){
  Rect bound_rect = bin_bounding_rect(image);
  Mat cropped_image(image,bound_rect); 
  return cropped_image;
}

bool NormalizeDatumImage(Datum* datum, const int imSize, const int charSize){
  /* Modifies datum's data attribute
  / so that the image is normalized.
  */
  Mat char_image = datum_to_image(datum);
 
  if (datum->is_inkml()){
    char_image = rand_dilate_image(char_image);
  }

 // distort training images
  if (datum->is_train() and !datum->is_mnist() ){
    char_image = distort_image(char_image);
  }
  
  Mat norm_img = process_char(char_image, imSize, charSize);
  datum->set_height(norm_img.rows);
  datum->set_width(norm_img.cols);
  datum->clear_data();
  datum->clear_float_data();
  
  string* datum_string = datum->mutable_data();
    for (int h = 0; h < norm_img.rows; ++h) {
      for (int w = 0; w < norm_img.cols; ++w) {
        datum_string->push_back(
          static_cast<char>(norm_img.at<uchar>(h, w)));
        }
      }
  return true;
}

void display_datum(Datum* datum){
  Mat image = datum_to_image(datum);
  namedWindow( "Display window", WINDOW_AUTOSIZE );
  imshow( "Display window", image);
  waitKey(0);
}

Mat datum_to_image(Datum* datum){
  /* Converts datum to greyscale image.
  /  Debugging version will just display image.
  */
  const string& datum_string = datum->data();
  int imWidth = datum->width();
  int imHeight = datum->height();
  Mat out_image = Mat::zeros(imHeight, imWidth, CV_8UC1);   
  uchar* row;
    for (int i = 0; i < out_image.rows; ++i) {
    row = out_image.ptr<uchar>(i); 
      for (int j = 0; j < out_image.cols; ++j) {
        row[j] = static_cast<uchar>(datum_string[j + i*imWidth]);
        }
      }
  return out_image;
}

Rect bin_bounding_rect(Mat& image){
/* Scans a binarized greyscale image for black pixels.  
*  Returns bounding box for all nonwhite pixels.
*/
  int channels = image.channels();
  CV_Assert(channels == 1);
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
  // overwrites image with resized image
  double nRows = image.rows;
  double nCols = image.cols;
  double size_factor = charSize / max(nRows,nCols); 
  int nRows_new = ceil(nRows * size_factor);
  int nCols_new = ceil(nCols * size_factor);
  Size size_new(nCols_new,nRows_new);
  if (size_factor < 1)
    resize( image, image, size_new, 0, 0, INTER_AREA);
  else
    resize( image, image, size_new, 0, 0, INTER_LINEAR);
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

Mat process_char(Mat& roi, const int imSize, const int charSize){
  // takes cropped greyscale (white on black) image and returns centered, resized image
  static const double halfSize = imSize / 2;
  Rect sub_image_rect;
  Mat big_image = Mat::zeros(imSize,imSize, CV_8UC1);
  Mat big_image_roi; 
  int xdiff,ydiff;
  double roi_midx, roi_midy;
  resize_roi(roi,charSize);	
  roi_midx = (roi.cols - 1.0) / 2.0;
  roi_midy = (roi.rows - 1.0) / 2.0;
  xdiff = halfSize - roi_midx;
  ydiff = halfSize - roi_midy;
  sub_image_rect = Rect( xdiff, ydiff, roi.cols, roi.rows);
  big_image_roi = big_image(sub_image_rect);
  roi.copyTo(big_image_roi);
  return big_image;	
}

/*
Mat CropImage(Mat& image, const int imSize, const int charSize){
  Rect bound_rect = bin_bounding_rect(image);
  Mat char_image(image,bound_rect);
  Mat big_image = process_char(char_image, imSize, charSize);
  return big_image;
}

Mat old_CropImage(Mat& image, const int imSize, const int charSize){
  // takes raw greyscale (white on black) image and returns centered, resized image
  Rect bound_rect = bin_bounding_rect(image);
  static const double halfSize = imSize / 2;
  Rect sub_image_rect;
  Mat big_image = Mat::zeros(imSize,imSize, CV_8UC1);
  Mat roi(image,bound_rect);
  Mat big_image_roi; 
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
  
  return big_image;	
}
*/ 
