#include "Region.cpp"
#include "constantes.cpp"
#include "profiler.cpp"
#include "utils.cpp"
#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <stack>
#include <string>

int main(int argc, char **argv) {
  std::filesystem::path dirPath = std::filesystem::path(argv[1]).parent_path();
  std::string folder = dirPath.string() + "/";
  std::cout << folder << std::endl;

  // MergeImages(folder);

  cv::Mat maskImage = cv::imread(folder + maskPath, cv::IMREAD_COLOR);
  if (maskImage.empty()) {
    std::cout << "Не удалось загрузить маску!" << std::endl;
    return -1;
  }

  cv::Mat colorImage = cv::imread(folder + colorImagePath);
  if (colorImage.empty()) {
    std::cout << "Не удалось загрузить изображение!" << std::endl;
    return -1;
  }

  cv::Mat result;

  {
    Profiler p("Main method");
    result = maskImage.clone();
    cv::cvtColor(maskImage, maskImage, cv::COLOR_BGR2GRAY);
    cv::Mat maskPath =
        cv::Mat::zeros(maskImage.rows + 2, maskImage.cols + 2, CV_8UC1);

    for (int y = 0; y < maskImage.rows; y++) {
      const uchar *rowMaskImage = maskImage.ptr<uchar>(y);
      const uchar *rowMaskPath = maskPath.ptr<uchar>(y + 1);

      for (int x = 0; x < maskImage.cols; ++x) {
        uchar m = rowMaskImage[x];
        if (m > 240 && rowMaskPath[x + 1] == 0) {
          Region region;
          region.seedPoint = cv::Point(x, y);

          cv::Rect rect;
          {
            Profiler p("first fill");
            region.half = cv::floodFill(maskImage, maskPath, cv::Point(x, y), 0,
                                        &rect, cv::Scalar(10), cv::Scalar(10),
                                        4 | cv::FLOODFILL_MASK_ONLY) /
                          2;
          }

          cv::Vec3b color;
          {
            Profiler p("count pixels");
            for (int y = rect.y; y < rect.y + rect.height; y++) {

              const cv::Vec3b *row = colorImage.ptr<cv::Vec3b>(y);
              const uchar *maskLine = maskPath.ptr<uchar>(y + 1);

              for (int x = rect.x; x < rect.x + rect.width; x++) {
                if (maskLine[x + 1] != 0) {
                  region.addPixel(row[x]);
                }
              }
            }
          }

          color = region.getMaxColor();

          {
            Profiler p("final fill");
            cv::floodFill(result, cv::Point(x, y), region.getMaxColor(), &rect,
                          cv::Scalar(0, 0, 0), cv::Scalar(0, 0, 0), 4);
          }
        }
      }
    }
  }

  for (const auto &tm : timeMap) {
    std::cout << "time for: " << tm.first << " spended: " << tm.second
              << std::endl;
  }
  cv::imwrite(folder + resultPath, result);

  return 0;
}