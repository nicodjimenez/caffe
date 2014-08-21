#ifndef NICO_NORMAL
#define NICO_NORMAL

#include <string>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "caffe/proto/caffe.pb.h"

void display_datum(caffe::Datum* datum);

cv::Rect bin_bounding_rect(cv::Mat& image);

void resize_roi(cv::Mat& image, const int charSize);

cv::Point get_com(cv::Mat& image);

//cv::Mat CropImage(cv::Mat& image, const int imSize, const int charSize);

cv::Mat datum_to_image(caffe::Datum* datum);

cv::Mat process_char(cv::Mat& roi, const int imSize, const int charSize);

cv::Mat rand_process_char(cv::Mat& roi, const int imSize, const int charSize);

void rand_rotate(cv::Mat& image);

void rand_dilate_image(cv::Mat& image);

void rand_scale(cv::Mat& image);

cv::Mat rotate( cv::Mat& image, double angle);

void distort_image(cv::Mat& image, const std::string& latex);

cv::Mat crop_image(cv::Mat& image);

bool NormalizeDatumImage(caffe::Datum* datum, const int imSize, const int charSize);

#endif
