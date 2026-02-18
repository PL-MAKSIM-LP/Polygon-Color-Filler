#pragma once
#include "Constantes.cpp"
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include <thread>

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

int getRepeats(int argc, char **argv) {
  if (argc < 3)
    return 5;
  try {
    return std::stoi(argv[2]);
  } catch (...) {
    std::cerr << "Invalid number, using default 5" << std::endl;
    return 5;
  }
}

int getNumberOfThreads(int argc, char **argv) {
  int numThreads = 1;
  if (argc > 3 && std::stoi(argv[3]) != 0) {
    numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
      numThreads = 1;
  }
  return numThreads;
}

cv::Mat loadImageOrExit(const std::string &path, int flags) {
  cv::Mat img = cv::imread(path, flags);
  if (img.empty()) {
    std::cerr << "Fail to load: " << path << std::endl;
    exit(-1);
  }
  return img;
}