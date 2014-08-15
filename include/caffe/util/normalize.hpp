#ifndef NICO_NORMAL
#define NICO_NORMAL

#include <string>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "caffe/proto/caffe.pb.h"

cv::Rect bin_bounding_rect(cv::Mat& image);

void resize_roi(cv::Mat& image, const int charSize);

cv::Point get_com(cv::Mat& image);

//cv::Mat CropImage(cv::Mat& image, const int imSize, const int charSize);

cv::Mat datum_to_image(caffe::Datum* datum);

cv::Mat process_char(cv::Mat& roi, const int imSize, const int charSize);

cv::Mat rotate( cv::Mat& image, double angle);

cv::Mat distort_image(cv::Mat& image);

bool NormalizeDatumImage(caffe::Datum* datum, const std::string key_str, const int imSize, const int charSize);

#endif
