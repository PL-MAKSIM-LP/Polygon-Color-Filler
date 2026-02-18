#include "./src/Constantes.cpp"
#include "./src/FileHelper.cpp"
#include "./src/Profiler.cpp"
#include "./src/Region.cpp"
#include "./src/Utils.cpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>

void processRegion(int x, int y, cv::Mat &maskImage, cv::Mat &currentRegionMask,
                   cv::Mat &processedMask, cv::Mat &colorImage,
                   cv::Mat &result);

void processRows(int yStart, int yEnd, cv::Mat &maskImage,
                 cv::Mat &processedMask, cv::Mat &colorImage, cv::Mat &result);

int main(int argc, char **argv) {

  std::filesystem::path dirPath = std::filesystem::path(argv[1]).parent_path();

  std::string folder = dirPath.string() + "/";

  int repeats = getRepeats(argc, argv);
  cv::Mat result;
  int numThreads = getNumberOfThreads(argc, argv);
  std::cout << "Count of threads: " << numThreads << std::endl;

  for (int i = 0; i < repeats; ++i) {

    cv::Mat maskImage = loadImageOrExit(folder + maskPath, cv::IMREAD_COLOR);

    cv::Mat colorImage =
        loadImageOrExit(folder + colorImagePath, cv::IMREAD_COLOR);

    Profiler p("Main method");

    result = maskImage.clone();
    cv::cvtColor(maskImage, maskImage, cv::COLOR_BGR2GRAY);

    cv::Mat processedMask =
        cv::Mat::zeros(maskImage.rows + 2, maskImage.cols + 2, CV_8UC1);

    int rowsPerThread = maskImage.rows / numThreads;

    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {

      int yStart = t * rowsPerThread;

      int yEnd =
          (t == numThreads - 1) ? maskImage.rows : yStart + rowsPerThread;

      threads.emplace_back(processRows, yStart, yEnd, std::ref(maskImage),
                           std::ref(processedMask), std::ref(colorImage),
                           std::ref(result));
    }

    for (auto &th : threads)
      th.join();
  }

  for (const auto &tm : timeMap) {
    std::cout << "average time for: " << tm.first
              << " spended: " << tm.second / repeats << std::endl;
  }

  writeResultToFile(repeats);
  cv::imwrite(folder + resultPath, result);

  return 0;
}

void processRegion(cv::Point seedPoint, cv::Mat &maskImage,
                   cv::Mat &currentRegionMask, cv::Mat &processedMask,
                   cv::Mat &colorImage, cv::Mat &result) {
  Region region;
  region.seedPoint = seedPoint;
  cv::Rect rect;
  cv::Vec3b color;

  {
    // Profiler p("Get region mask");
    cv::floodFill(maskImage, currentRegionMask, region.seedPoint, 0, &rect,
                  cv::Scalar(10), cv::Scalar(10), 4 | cv::FLOODFILL_MASK_ONLY);
  }

  {
    // Profiler p("Count color pixels");
    int startY = std::max(rect.y, 0);
    int endY = std::min(rect.y + rect.height, colorImage.rows);
    int startX = std::max(rect.x, 0);
    int endX = std::min(rect.x + rect.width, colorImage.cols);

    for (int ry = startY; ry < endY; ++ry) {
      const cv::Vec3b *row = colorImage.ptr<cv::Vec3b>(ry);
      uchar *maskRow = currentRegionMask.ptr<uchar>(ry + 1);

      for (int rx = startX; rx < endX; ++rx) {
        if (maskRow[rx + 1] != 0) {
          region.addPixel(row[rx]);
          maskRow[rx + 1] = 0;
        }
      }
    }
  }

  {
    // Profiler p("Fill result by color");
    color = region.getMaxColor();
    cv::floodFill(result, processedMask, region.seedPoint, color, &rect,
                  cv::Scalar(10), cv::Scalar(10), 4);
  }
  region.clear();
}

void processRows(int yStart, int yEnd, cv::Mat &maskImage,
                 cv::Mat &processedMask, cv::Mat &colorImage, cv::Mat &result) {

  cv::Mat currentRegionMask =
      cv::Mat::zeros(maskImage.rows + 2, maskImage.cols + 2, CV_8UC1);

  for (int y = yStart; y < yEnd; ++y) {

    const uchar *rowMaskImage = maskImage.ptr<uchar>(y);
    const uchar *rowMaskPath = processedMask.ptr<uchar>(y + 1);

    for (int x = 0; x < maskImage.cols; ++x) {

      uchar m = rowMaskImage[x];

      if (m >= 255 && rowMaskPath[x + 1] == 0) {
        processRegion(cv::Point(x, y), maskImage, currentRegionMask,
                      processedMask, colorImage, result);
      }
    }
  }
}