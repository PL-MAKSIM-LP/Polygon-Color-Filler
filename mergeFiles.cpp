#include "src/Utils.cpp"
#include <filesystem>
#include <string>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Error: Please provide the path to the input file."
              << std::endl;
    return 1;
  }

  std::filesystem::path dirPath = std::filesystem::path(argv[1]).parent_path();
  std::string folder = dirPath.string() + "/";

  if (!std::filesystem::exists(folder) ||
      !std::filesystem::is_directory(folder)) {
    std::cerr << "Error: Folder does not exist: " << folder << std::endl;
    return 1;
  }

  MergeImages(folder);
}