
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <stack>


std::map<std::string, float> timeMap;


struct Profiler
{
    cv::TickMeter tm;
    std::string tag;

    Profiler(std::string _tag)
    {
        tag = _tag;
        tm.start();
    }

    ~Profiler()
    {
        tm.stop();
        timeMap[tag] += tm.getTimeMilli();
    }
};