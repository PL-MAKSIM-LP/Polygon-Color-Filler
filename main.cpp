#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

std::string maskPath = "mask.png";
std::string colorImagePath = "color_mask.png";
std::string combinedPath = "_combined.png";
std::string resultPath = "_result.png";

std::string
intToHex(int number)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << number;
    return ss.str();
}

struct Region
{
    std::unordered_map<uint32_t, int> counts;

    int halfSize;
    cv::Point seedPoint;

    void addPixel(const cv::Vec3b &color)
    {
        uint32_t key = (color[2] << 16) | (color[1] << 8) | color[0];
        ++counts[key];
    }

    cv::Vec3b getMaxColor() const
    {

        uint32_t maxKey = 0;
        int maxValue = 0;

        for (const auto &kv : counts)
        {
            if (kv.second > maxValue)
            {
                maxValue = kv.second;
                maxKey = kv.first;
            }
        }

        return cv::Vec3b(
            maxKey & 0xFF,
            (maxKey >> 8) & 0xFF,
            (maxKey >> 16) & 0xFF);
    }
};

int compareImages(std::string folder);

int main(int argc, char **argv)
{
    std::filesystem::path dirPath = std::filesystem::path(argv[1]).parent_path();
    std::string folder = dirPath.string() + "/";
    std::cout << folder << std::endl;

    compareImages(folder);

    // 1️⃣ Загружаем маску (бинарное изображение)
    cv::Mat maskImage = cv::imread(folder + maskPath, cv::IMREAD_COLOR);
    if (maskImage.empty())
    {
        std::cout << "Не удалось загрузить маску!" << std::endl;
        return -1;
    }

    cv::Mat result = maskImage.clone();
    cv::cvtColor(maskImage, maskImage, cv::COLOR_BGR2GRAY);

    // 2️⃣ Загружаем цветное изображение
    cv::Mat colorImage = cv::imread(folder + colorImagePath);
    if (colorImage.empty())
    {
        std::cout << "Не удалось загрузить изображение!" << std::endl;
        return -1;
    }
    // return 0;

    std::cout << "start processing" << std::endl;
    // Маска для floodFill (rows+2, cols+2)
    cv::Mat maskPath = cv::Mat::zeros(maskImage.rows + 2, maskImage.cols + 2, CV_8UC1);

    std::vector<Region> regions;

    // 1️⃣ Сначала считаем все области по белым пикселям маски
    for (int y = 0; y < maskImage.rows; y++)
    {
        for (int x = 0; x < maskImage.cols; x += 5)
        {
            uchar m = maskImage.at<uchar>(y, x);
            if (m > 240 && maskPath.at<uchar>(y + 1, x + 1) == 0)
            {
                Region region;
                region.seedPoint = cv::Point(x, y);

                cv::Rect rect;
                // floodFill только на маске
                cv::floodFill(maskImage, maskPath, cv::Point(x, y), 0,
                              &rect, cv::Scalar(10), cv::Scalar(10),
                              4 | cv::FLOODFILL_MASK_ONLY);

                region.halfSize = (rect.width * rect.height) / 2;

                // std::cout << "________________________________" << std::endl;
                // std::cout << "X = " << rect.x << std::endl;
                // std::cout << "Y = " << rect.y << std::endl;
                // std::cout << "height = " << rect.height << std::endl;
                // std::cout << "width = " << rect.width << std::endl;
                // std::cout << "________________________________" << std::endl;

                for (int y = rect.y; y < rect.y + rect.height; y++)
                {
                    for (int x = rect.x; x < rect.x + rect.width; x++)
                    {
                        if (maskPath.at<uchar>(y + 1, x + 1) != 0)
                        {
                            cv::Vec3b color = colorImage.at<cv::Vec3b>(y, x);
                            region.addPixel(color);
                        }
                    }
                }

                auto color = region.getMaxColor();
                std::cout << "Region: (" << region.seedPoint.x << " " << region.seedPoint.y << ") ";
                std::cout << "Color: ("
                          << intToHex((int)color[2]) << intToHex((int)color[1]) << intToHex((int)color[0]) << ")" << std::endl;

                cv::floodFill(result, cv::Point(x, y), color, &rect,
                              cv::Scalar(10, 10, 10), cv::Scalar(10, 10, 10), 4);
            }
        }
    }

    cv::imwrite(folder + resultPath, result);

    return 0;
}

int compareImages(std::string folder)
{
    std::cout << folder + colorImagePath << std::endl;
    cv::Mat color = cv::imread(folder + colorImagePath, cv::IMREAD_COLOR); // BGR
    if (color.empty())
    {
        std::cout << "Не удалось загрузить цвет!" << std::endl;
        return -1;
    }
    std::cout << folder + maskPath << std::endl;
    cv::Mat gray = cv::imread(folder + maskPath, cv::IMREAD_GRAYSCALE) > 1; // 1 канал
    if (gray.empty())
    {
        std::cout << "Не удалось загрузить маску!" << std::endl;
        return -1;
    }

    cv::Mat result = cv::Mat::zeros(color.size(), color.type()); // чорне зображення
    for (int y = 0; y < color.rows; y++)
    {
        std::cout << "\n";
        const cv::Vec3b *rowColor = color.ptr<cv::Vec3b>(y);
        const uchar *rowGray = gray.ptr<uchar>(y);
        cv::Vec3b *rowResult = result.ptr<cv::Vec3b>(y);

        for (int x = 0; x < color.cols; x++)
        {
            if (rowGray[x] != 0)            // якщо піксель не чорний
                rowResult[x] = rowColor[x]; // копіюємо кольоровий
        }
    }

    cv::imwrite(folder + combinedPath, result);

    return 0;
}
