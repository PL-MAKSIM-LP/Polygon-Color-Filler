
#include "./src/Constantes.cpp"
#include "./src/FileHelper.cpp"
#include "./src/Profiler.cpp"
#include "./src/Region.cpp"
#include "./src/Utils.cpp"
#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

void processSeed(Region &region, cv::Mat &maskImage, cv::Mat &tempMask,
                 cv::Mat &maskPath, cv::Mat &colorImage, cv::Mat &result) {

  const cv::Vec3b *row;
  uchar *maskRow;
  cv::Mat regionMask;
  cv::Vec3b color;
  cv::Rect rect;

  {
    // Profiler p("floodFill first");
    cv::floodFill(maskImage, tempMask, region.seedPoint, 0, &rect,
                  cv::Scalar(10), cv::Scalar(10), 4 | cv::FLOODFILL_MASK_ONLY);
  }

  {
    // Profiler p("addPixel");
    int startY = std::max(rect.y, 0);
    int endY = std::min(rect.y + rect.height, colorImage.rows);
    int startX = std::max(rect.x, 0);
    int endX = std::min(rect.x + rect.width, colorImage.cols);

    for (int ry = startY; ry < endY; ++ry) {
      row = colorImage.ptr<cv::Vec3b>(ry);
      maskRow = tempMask.ptr<uchar>(ry + 1);

      for (int rx = startX; rx < endX; ++rx) {
        if (maskRow[rx + 1] != 0) {
          {
            region.addPixel(row[rx]);
          }
          maskRow[rx + 1] = 0;
        }
      }
    }
  }

  {
    // Profiler p("floodFill last");
    color = region.getMaxColor();
    cv::floodFill(result, maskPath, region.seedPoint, color, &rect,
                  cv::Scalar(10), cv::Scalar(10), 4);
  }
  region.clear();
}

void processRows(int yStart, int yEnd, cv::Mat &maskImage, cv::Mat &tempMask,
                 cv::Mat &maskPath, cv::Mat &colorImage, cv::Mat &result) {
  for (int y = yStart; y < yEnd; ++y) {
    const uchar *rowMaskImage = maskImage.ptr<uchar>(y);
    const uchar *rowMaskPath = maskPath.ptr<uchar>(y + 1);

    for (int x = 0; x < maskImage.cols; ++x) {
      uchar m = rowMaskImage[x];
      if (m >= 255 && rowMaskPath[x + 1] == 0) {
        Region region;
        region.seedPoint = cv::Point(x, y);
        processSeed(region, maskImage, tempMask, maskPath, colorImage, result);
      }
    }
  }
}

int main(int argc, char **argv) {
  std::filesystem::path dirPath = std::filesystem::path(argv[1]).parent_path();
  std::string folder = dirPath.string() + "/";

  int repeats;

  try {
    int value2 = std::stoi(argv[2]);
    repeats = value2;
    std::cout << "Use count of repeats: " << repeats << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Invalid number use default count of repeats: 5" << std::endl;
    repeats = 5;
  }

  for (int i = 0; i < repeats; ++i) {

    cv::Mat maskImage = cv::imread(folder + maskPath, cv::IMREAD_COLOR);
    if (maskImage.empty()) {
      std::cout << "Fail to load mask" << std::endl;
      return -1;
    }

    cv::Mat colorImage = cv::imread(folder + colorImagePath);
    if (colorImage.empty()) {
      std::cout << "Fail to load color image" << std::endl;
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

      int numThreads = std::thread::hardware_concurrency();
      int rowsPerThread = maskImage.rows / numThreads;
      std::vector<std::thread> threads;

      for (int t = 0; t < numThreads; ++t) {
        int yStart = t * rowsPerThread;
        int yEnd =
            (t == numThreads - 1) ? maskImage.rows : yStart + rowsPerThread;

        threads.emplace_back(processRows, yStart, yEnd, std::ref(maskImage),
                             std::ref(tempMask), std::ref(maskPath),
                             std::ref(colorImage), std::ref(result));
      }

      for (auto &th : threads)
        th.join();
    }

    if (i < repeats - 1)
      continue;

    for (const auto &tm : timeMap) {
      std::cout << "average time for: " << tm.first
                << " spended: " << tm.second / repeats << std::endl;
    }

    wtireResultToFile(repeats);
    cv::imwrite(folder + resultPath, result);
  }

  return 0;
}