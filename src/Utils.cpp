#pragma once
#include "Constantes.cpp"
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>

int MergeImages(std::string folder) {
  cv::Mat color = cv::imread(folder + colorImagePath, cv::IMREAD_COLOR); // BGR
  if (color.empty()) {
    std::cout << "Не удалось загрузить цвет!" << std::endl;
    return -1;
  }

  cv::Mat gray =
      cv::imread(folder + maskPath, cv::IMREAD_GRAYSCALE) > 1; // gray_scale
  if (gray.empty()) {
    std::cout << "Не удалось загрузить маску!" << std::endl;
    return -1;
  }

  cv::Mat result = cv::Mat::zeros(color.size(), color.type());
  for (int y = 0; y < color.rows; y++) {
    const cv::Vec3b *rowColor = color.ptr<cv::Vec3b>(y);
    const uchar *rowGray = gray.ptr<uchar>(y);
    cv::Vec3b *rowResult = result.ptr<cv::Vec3b>(y);

    for (int x = 0; x < color.cols; x++) {
      if (rowGray[x] != 0)
        rowResult[x] = rowColor[x];
    }
  }

  cv::imwrite(folder + combinedPath, result);

  return 0;
}