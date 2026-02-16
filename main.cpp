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

            {
              Profiler p(" region.seedPoint");
              region.seedPoint = cv::Point(x, y);
            }
            {
              Profiler p("setTo 0");
              tempMask.setTo(0);
            }

            {
              Profiler p("floodFill first");
              // Используем tempMask вместо maskPath!
              region.half =
                  cv::floodFill(maskImage, tempMask, cv::Point(x, y), 0, &rect,
                                cv::Scalar(10), cv::Scalar(10),
                                4 | cv::FLOODFILL_MASK_ONLY) /
                  2;
            }

            // count++;

            // cv::imwrite(folder + "tempMask_" + std::to_string(count) +
            // ".png",
            //             tempMask * 255);

            {
              Profiler p("regionMask = tempMask");
              regionMask = tempMask(cv::Rect(rect.x + 1, rect.y + 1, rect.width,
                                             rect.height))
                               .clone();
            }
            {
              Profiler p("addPixel");
              // Используем regionMask вместо maskLine из maskPath
              for (int ry = 0; ry < rect.height; ry++) {
                const cv::Vec3b *row =
                    colorImage.ptr<cv::Vec3b>(rect.y + ry) + rect.x;
                const uchar *maskRow = regionMask.ptr<uchar>(ry);

                for (int rx = 0; rx < rect.width; rx++) {
                  if (maskRow[rx] != 0) {
                    region.addPixel(row[rx]);
                  }
                }
              }
            }

            {
              Profiler p("getMaxColor");
              color = region.getMaxColor();
            }

            {
              Profiler p(" result(rect).setTo");
              // Используем regionMask для быстрой заливки!
              result(rect).setTo(color, regionMask);
            }
            {
              Profiler p("floodFill last");
              cv::floodFill(maskImage, maskPath, cv::Point(x, y), 0, &rect,
                            cv::Scalar(10), cv::Scalar(10),
                            4 | cv::FLOODFILL_MASK_ONLY);
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