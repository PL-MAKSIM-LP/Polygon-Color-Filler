#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <stack>

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
    std::vector<std::pair<uint32_t, int>> colorCounts;
    cv::Point seedPoint;

    uint32_t maxKey = 0;
    int maxCount = 0;
    uint32_t half;

    Region()
    {
        colorCounts.reserve(128);
    }

    bool addPixel(const cv::Vec3b &color)
    {

        uint32_t key = (color[2] << 16) | (color[1] << 8) | color[0];

        auto it = std::find_if(colorCounts.begin(), colorCounts.end(),
                               [key](const std::pair<uint32_t, int> &p)
                               { return p.first == key; });

        if (it != colorCounts.end())
        {
            it->second++;

            if (it->second > maxCount)
            {
                maxCount = it->second;
                maxKey = key;
            }
        }
        else
        {
            colorCounts.push_back({key, 1});

            if (1 > maxCount)
            {
                maxCount = 1;
                maxKey = key;
            }
        }

        return maxCount >= half;
    }

    cv::Vec3b getMaxColor() const
    {
        return cv::Vec3b(
            maxKey & 0xFF,
            (maxKey >> 8) & 0xFF,
            (maxKey >> 16) & 0xFF);
    }

    void clear()
    {
        colorCounts.clear();
        maxKey = 0;
        maxCount = 0;
        seedPoint = cv::Point(-1, -1);
    }
};

int compareImages(std::string folder);

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

void fastFloodFill(cv::Mat &result, const cv::Mat &mask, cv::Point seed, const cv::Vec3b &fillColor);

int main(int argc, char **argv)
{
    std::filesystem::path dirPath = std::filesystem::path(argv[1]).parent_path();
    std::string folder = dirPath.string() + "/";
    std::cout << folder << std::endl;

    compareImages(folder);

    cv::Mat maskImage = cv::imread(folder + maskPath, cv::IMREAD_COLOR);
    if (maskImage.empty())
    {
        std::cout << "Не удалось загрузить маску!" << std::endl;
        return -1;
    }

    cv::Mat colorImage = cv::imread(folder + colorImagePath);
    if (colorImage.empty())
    {
        std::cout << "Не удалось загрузить изображение!" << std::endl;
        return -1;
    }

    cv::Mat result = maskImage.clone();
    {
        Profiler p("Main method");

        cv::cvtColor(maskImage, maskImage, cv::COLOR_BGR2GRAY);

        cv::Mat maskPath = cv::Mat::zeros(maskImage.rows + 2, maskImage.cols + 2, CV_8UC1);

        std::vector<Region> regions;

        for (int y = 0; y < maskImage.rows; y++)
        {
            const uchar *rowMaskImage = maskImage.ptr<uchar>(y);
            const uchar *rowMaskPath = maskPath.ptr<uchar>(y + 1);

            for (int x = 0; x < maskImage.cols; ++x)
            {
                uchar m = rowMaskImage[x];
                if (m > 240 && rowMaskPath[x + 1] == 0)
                {
                    Region region;
                    region.seedPoint = cv::Point(x, y);

                    cv::Rect rect;
                    {
                        auto p = Profiler("first fill");
                        region.half = cv::floodFill(maskImage, maskPath, cv::Point(x, y), 0,
                                                    &rect, cv::Scalar(10), cv::Scalar(10),
                                                    4 | cv::FLOODFILL_MASK_ONLY) /
                                      2;
                    }

                    bool flag = false;
                    cv::Vec3b color;
                    {
                        auto p = Profiler("count pixels");
                        for (int y = rect.y; y < rect.y + rect.height; y++)
                        {

                            const cv::Vec3b *row = colorImage.ptr<cv::Vec3b>(y);
                            const uchar *maskLine = maskPath.ptr<uchar>(y + 1);

                            for (int x = rect.x; x < rect.x + rect.width; x++)
                            {
                                if (maskLine[x + 1] != 0)
                                {
                                    if (region.addPixel(row[x]))
                                    {
                                        color = row[x];
                                        flag = true;
                                    }
                                }
                            }
                            if (flag)
                            {
                                break;
                            }
                        }
                    }

                    if (!flag)
                    {
                        color = region.getMaxColor();
                    }

                    {
                        auto p = Profiler("final fill");
                        // cv::Point seedInROI = region.seedPoint - region.tl();

                        // fastFloodFill(result, maskImage(region).clone(), seedInROI, color);
                        cv::floodFill(result, cv::Point(x, y), color, &rect,
                                      cv::Scalar(0, 0, 0), cv::Scalar(0, 0, 0), 4);
                    }
                }
            }
        }
    }

    for (const auto &tm : timeMap)
    {
        std::cout << "time for: " << tm.first << " spended: " << tm.second << std::endl;
    }
    cv::imwrite(folder + resultPath, result);

    return 0;
}

void fastFloodFill(cv::Mat &result, const cv::Mat &mask, cv::Point seed, const cv::Vec3b &fillColor)
{
    CV_Assert(result.type() == CV_8UC3);
    CV_Assert(mask.type() == CV_8UC1);
    CV_Assert(seed.x >= 0 && seed.x < mask.cols && seed.y >= 0 && seed.y < mask.rows);
    CV_Assert(mask.ptr<uchar>(seed.y)[seed.x] != 0); // seed всередині маски

    // матриця для відвіданих пікселів
    cv::Mat visited = cv::Mat::zeros(mask.size(), CV_8UC1);

    std::stack<cv::Point> s;
    s.push(seed);

    while (!s.empty())
    {
        cv::Point p = s.top();
        s.pop();

        uchar *visitedRow = visited.ptr<uchar>(p.y);
        const uchar *maskRow = mask.ptr<uchar>(p.y);
        cv::Vec3b *resRow = result.ptr<cv::Vec3b>(p.y);

        // перевіряємо, чи вже відвідано або маска == 0
        if (visitedRow[p.x] != 0 || maskRow[p.x] == 0)
            continue;

        visitedRow[p.x] = 1;     // відмічаємо як відвідане
        resRow[p.x] = fillColor; // заливка кольором

        // додаємо сусідів (4-connected)
        if (p.x + 1 < mask.cols)
            s.push(cv::Point(p.x + 1, p.y));
        if (p.x - 1 >= 0)
            s.push(cv::Point(p.x - 1, p.y));
        if (p.y + 1 < mask.rows)
            s.push(cv::Point(p.x, p.y + 1));
        if (p.y - 1 >= 0)
            s.push(cv::Point(p.x, p.y - 1));
    }
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

    cv::Mat result = cv::Mat::zeros(color.size(), color.type());
    for (int y = 0; y < color.rows; y++)
    {
        const cv::Vec3b *rowColor = color.ptr<cv::Vec3b>(y);
        const uchar *rowGray = gray.ptr<uchar>(y);
        cv::Vec3b *rowResult = result.ptr<cv::Vec3b>(y);

        for (int x = 0; x < color.cols; x++)
        {
            if (rowGray[x] != 0)
                rowResult[x] = rowColor[x];
        }
    }

    cv::imwrite(folder + combinedPath, result);

    return 0;
}
