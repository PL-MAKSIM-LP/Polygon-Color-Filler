#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

struct Region {
  std::vector<std::pair<uint32_t, int>> colorCounts;
  cv::Point seedPoint;

  uint32_t maxKey = 0;
  int maxCount = 0;
  uint32_t half;

  Region() { colorCounts.reserve(128); }

  void addPixel(const cv::Vec3b &color) {

    uint32_t key = (color[2] << 16) | (color[1] << 8) | color[0];

    auto it = std::find_if(
        colorCounts.begin(), colorCounts.end(),
        [key](const std::pair<uint32_t, int> &p) { return p.first == key; });

    if (it != colorCounts.end()) {
      it->second++;

      if (it->second > maxCount) {
        maxCount = it->second;
        maxKey = key;
      }
    } else {
      colorCounts.push_back({key, 1});

      if (1 > maxCount) {
        maxCount = 1;
        maxKey = key;
      }
    }
  }

  cv::Vec3b getMaxColor() const {
    return cv::Vec3b(maxKey & 0xFF, (maxKey >> 8) & 0xFF,
                     (maxKey >> 16) & 0xFF);
  }

  void clear() {
    colorCounts.clear();
    maxKey = 0;
    maxCount = 0;
    seedPoint = cv::Point(-1, -1);
  }
};