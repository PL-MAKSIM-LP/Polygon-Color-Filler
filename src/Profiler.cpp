#pragma once

#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <stack>
#include <string>

static std::map<std::string, float> timeMap;

struct Profiler {
  cv::TickMeter tm;
  std::string tag;

  Profiler(std::string _tag) {
    tag = _tag;
    tm.start();
  }

  ~Profiler() {
    tm.stop();
    timeMap[tag] += tm.getTimeMilli();
  }
};