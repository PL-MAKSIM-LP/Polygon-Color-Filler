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
    cv::Mat tempMask =
        cv::Mat::zeros(maskImage.rows + 2, maskImage.cols + 2, CV_8UC1);

    int count = 0;
    for (int y = 0; y < maskImage.rows; y++) {
      {
        Profiler p("rows");
        const uchar *rowMaskImage = maskImage.ptr<uchar>(y);
        const uchar *rowMaskPath = maskPath.ptr<uchar>(y + 1);

        for (int x = 0; x < maskImage.cols; ++x) {
          uchar m = rowMaskImage[x];
          if (m > 240 && rowMaskPath[x + 1] == 0) {
            Region region;
            cv::Mat regionMask;
            cv::Vec3b color;
            cv::Rect rect;
            region.seedPoint = cv::Point(x, y);

            {
              Profiler p("floodFill first");
              cv::floodFill(maskImage, tempMask, cv::Point(x, y), 0, &rect,
                            cv::Scalar(10), cv::Scalar(10),
                            4 | cv::FLOODFILL_MASK_ONLY);
            }

            {
              Profiler p("addPixel");

              // Ограничиваем rect внутри изображения
              int startY = std::max(rect.y, 0);
              int endY = std::min(rect.y + rect.height, colorImage.rows);
              int startX = std::max(rect.x, 0);
              int endX = std::min(rect.x + rect.width, colorImage.cols);

              for (int ry = startY; ry < endY; ++ry) {
                const cv::Vec3b *row = colorImage.ptr<cv::Vec3b>(ry);
                uchar *maskRow = tempMask.ptr<uchar>(
                    ry + 1); // +1 если tempMask для floodFill

                for (int rx = startX; rx < endX; ++rx) {
                  if (maskRow[rx + 1] != 0) { // +1 для маски
                    region.addPixel(row[rx]);
                    maskRow[rx + 1] = 0;
                  }
                }
              }
            }

            {
              Profiler p("floodFill last");
              color = region.getMaxColor();
              cv::floodFill(result, maskPath, region.seedPoint, color, &rect,
                            cv::Scalar(10), cv::Scalar(10), 4);
            }
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
