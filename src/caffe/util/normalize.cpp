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

void rand_dilate_image(Mat& image){
  // randomly dilate image 
  //cout << "Maybe will distort image..." << endl;
  static boost::minstd_rand intgen;
  static boost::uniform_01<boost::minstd_rand> gen(intgen);
  double sample = gen();
  if (sample > 0.3){
    //cout << "Dilating image..." << endl;
    dilate(image,image,Mat());
  }
}

void rand_rotation(Mat& image){
  // random rotation plus random resize 
  static boost::minstd_rand intgen;
  static boost::uniform_01<boost::minstd_rand> gen(intgen);
  // we rotate the image half the time
  if (gen() > 0.7){
    // rotate image
    double angle = 4 * (2 * gen() - 1); 
    //cout << "Angle: " << angle << endl;
    image = rotate( image, angle);
  }
}

void rand_scale(Mat& image){
  static boost::minstd_rand intgen;
  static boost::uniform_01<boost::minstd_rand> gen(intgen);
  // now resize image
  double dist = 0.08;
  //cout << "Scaling image" << endl;
  double nRows = image.rows;
  double nCols = image.cols;
  int nCols_new = nCols;
  int nRows_new = nRows;
  
  if (nRows > nCols){
    // distort height
    if (nRows > 7){
      nRows_new = ceil(nRows * (1 + dist*(2*gen()-1)));
    }
  }
  else{
    // distort width
    if (nCols > 7){
      nCols_new = ceil(nCols * (1 + dist*(2*gen()-1)));
    }
  }
  Size size_new(nCols_new,nRows_new);
  resize( image, image, size_new, 0, 0, INTER_AREA);
}

void distort_image(Mat& image, const string& latex){
  // random rotation plus random resize 
  static boost::minstd_rand intgen;
  static boost::uniform_01<boost::minstd_rand> gen(intgen);
  // initialize special characters not to rotate 
  //static string array_0[] = {"1","7"};
  //static string array_1[] = {"a","9","q","g","b","6","5","s","\\int","+","t","(",")","c","C"};
  //static vector<string> no_rotation(array_0,array_0 + 2);
  //static vector<string> no_scale(array_1, array_1 + 2);
  //bool can_rotate = find(no_rotation.begin(), no_rotation.end(), latex) == no_rotation.end();
  //bool can_scale = find(no_scale.begin(), no_scale.end(), latex) == no_scale.end();
  bool can_rotate=true, can_scale=false;

  // if we cannot find the element in the list of symbols to skip
  if (can_rotate) rand_rotation(image); 
  if (can_scale) rand_scale(image);
  if (can_rotate) rand_rotation(image);
  image = crop_image(image);
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
  if (datum->is_normal()) return true;

  Mat char_image = datum_to_image(datum);
  Mat norm_img;
 // distort training images
  if (datum->is_train()){
    if (datum->is_inkml()){
      rand_dilate_image(char_image);
    }
    distort_image(char_image, datum->latex());
    norm_img = rand_process_char(char_image, imSize, charSize);
  }
  else{
    // usual normalization
    norm_img = process_char(char_image, imSize, charSize);
  }

  datum->set_height(norm_img.rows);
  datum->set_width(norm_img.cols);
  datum->set_is_normal(true);
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
  int nRows_new = round(nRows * size_factor);
  int nCols_new = round(nCols * size_factor);
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


Mat rand_process_char(Mat& roi, const int imSize, const int charSize){
  // takes cropped greyscale (white on black) image and returns centered, resized image plus random jitter
  static boost::minstd_rand intgen;
  static boost::uniform_01<boost::minstd_rand> gen(intgen);
  static const double halfSize = imSize / 2;
  Rect sub_image_rect;
  Mat big_image = Mat::zeros(imSize,imSize, CV_8UC1);
  Mat big_image_roi; 
  static double xdiff,ydiff,jitter_x,jitter_y;
  static double roi_midx, roi_midy;
  // distort character size a bit
  //int dist_charSize = round(1*(2*gen()-1) + charSize);
  resize_roi(roi,charSize);	
  roi_midx = (roi.cols - 1.0) / 2.0;
  roi_midy = (roi.rows - 1.0) / 2.0;

  // randomly apply random jitter
  if (gen() > 0.7)
    jitter_x = 2*(2*gen() - 1);
  else
    jitter_x = 0;
  
  if (gen() > 0.7)
    jitter_y = 2*(2*gen() - 1);
  else
    jitter_y = 0;

  //cout << "jitter x: " << jitter_x << endl; 
  //cout << "jitter y: " << jitter_y << endl; 
  xdiff = jitter_x + halfSize - roi_midx;
  ydiff = jitter_y + halfSize - roi_midy;
  sub_image_rect = Rect( xdiff, ydiff, roi.cols, roi.rows);
  big_image_roi = big_image(sub_image_rect);
  roi.copyTo(big_image_roi);
  return big_image;	
} 

Mat process_char(Mat& roi, const int imSize, const int charSize){
  // takes cropped greyscale (white on black) image and returns centered, resized image
  static const double halfSize = imSize / 2;
  Rect sub_image_rect;
  Mat big_image = Mat::zeros(imSize,imSize, CV_8UC1);
  Mat big_image_roi; 
  double xdiff,ydiff;
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

