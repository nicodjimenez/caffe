#ifndef NICO_NORMAL
#define NICO_NORMAL

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

//cv::Rect bin_bounding_rect(cv::Mat& image);
//void resize_roi(cv::Mat& image, const int charSize);
//cv::Point get_com(cv::Mat& image);
cv::Mat CropImage(cv::Mat& image, const int imSize, const int charSize);

#endif
