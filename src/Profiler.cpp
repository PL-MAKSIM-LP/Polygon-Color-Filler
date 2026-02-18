#pragma once

#include <filesystem>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <stack>
#include <string>

static std::map<std::string, float> timeMap;
// std::mutex mx;

struct Profiler {
  cv::TickMeter tm;
  std::string tag;

  Profiler(std::string _tag) {
    tag = _tag;
    tm.start();
  }

  ~Profiler() {
    // std::lock_guard<std::mutex> lock(mx);
    tm.stop();
    timeMap[tag] += tm.getTimeMilli();
  }
};