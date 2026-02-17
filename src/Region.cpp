#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

struct Region {
  std::unordered_map<uint32_t, int> colorCounts;
  cv::Point seedPoint;

  Region() { colorCounts.reserve(512); }

  void addPixel(const cv::Vec3b &color) {
    uint32_t key = (color[2] << 16) | (color[1] << 8) | color[0];
    int count = ++colorCounts[key];
  }

  cv::Vec3b getMaxColor() const {
    if (colorCounts.empty())
      return cv::Vec3b(0, 0, 0);

    auto it = std::max_element(
        colorCounts.begin(), colorCounts.end(),
        [](const auto &a, const auto &b) { return a.second < b.second; });

    uint32_t key = it->first;
    return cv::Vec3b(key & 0xFF, (key >> 8) & 0xFF, (key >> 16) & 0xFF);
  }

  void clear() {
    colorCounts.clear();
    seedPoint = cv::Point(-1, -1);
  }
};