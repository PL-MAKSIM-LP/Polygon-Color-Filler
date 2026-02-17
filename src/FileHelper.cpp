#pragma once

#include "Profiler.cpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

static void wtireResultToFile(int repeats = 1) {
  std::string filename = "./result.txt";
  if (std::filesystem::exists(filename)) {
    std::ofstream outFile(filename, std::ios::app);
    if (outFile.is_open()) {
      outFile << timeMap["Main method"] / repeats << "ms" << std::endl;
      outFile.close();
    } else {
      std::cerr << "Cannot open file for writing!" << std::endl;
    }
  } else {
    std::cout << "File does not exist: " << filename << std::endl;
  }
}