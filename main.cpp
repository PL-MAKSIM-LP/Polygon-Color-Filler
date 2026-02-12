#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

struct Region {
    int totalPixels = 0; // всего пикселей
    std::map<std::tuple<int,int,int>, int> colorCount; // подсчет по цветам (B,G,R)
    cv::Point seedPoint; // точка начала области
};


int main(int argc, char** argv)
{
   
       if (argc < 3) {
        std::cout << "Использование: " << argv[0] << " <mask.png> <image.jpg>" << std::endl;
        return -1;
    }

    // 1️⃣ Загружаем маску (бинарное изображение)
    cv::Mat maskImage = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
    if (maskImage.empty()) {
        std::cout << "Не удалось загрузить маску!" << std::endl;
        return -1;
    }

    // 2️⃣ Загружаем цветное изображение
    cv::Mat colorImage = cv::imread(argv[2]);
    if (colorImage.empty()) {
        std::cout << "Не удалось загрузить изображение!" << std::endl;
        return -1;
    }

    // Маска для floodFill (rows+2, cols+2)
    cv::Mat mask = cv::Mat::zeros(maskImage.rows + 2, maskImage.cols + 2, CV_8UC1);

    std::vector<Region> regions;

    // 1️⃣ Сначала считаем все области по белым пикселям маски
    for (int y = 0; y < maskImage.rows; y++) {
        for (int x = 0; x < maskImage.cols; x++) {
            uchar m = maskImage.at<uchar>(y, x);
            if (m > 240 && mask.at<uchar>(y+1, x+1) == 0) { // белый пиксель и не обработан
                Region region;
                region.seedPoint = cv::Point(x, y);

                cv::Rect rect;
                // floodFill только на маске
                cv::floodFill(maskImage, mask, cv::Point(x, y), 0,
                              &rect, cv::Scalar(10), cv::Scalar(10),
                              4 | cv::FLOODFILL_MASK_ONLY);

                // Считаем пиксели по цветам на исходном изображении
                for (int yy = rect.y; yy < rect.y + rect.height; yy++) {
                    for (int xx = rect.x; xx < rect.x + rect.width; xx++) {
                        if (mask.at<uchar>(yy+1, xx+1) != 0) {
                            cv::Vec3b c = colorImage.at<cv::Vec3b>(yy, xx);
                            region.totalPixels++;
                            std::tuple<int,int,int> key = std::make_tuple(c[0],c[1],c[2]);
                            region.colorCount[key]++;
                        }
                    }
                }

                regions.push_back(region);
            }
        }
    }

    // 2️⃣ Выводим статистику
    for (size_t i = 0; i < regions.size(); i++) {
        std::cout << "Область " << i << " (точка: " << regions[i].seedPoint.x << "," << regions[i].seedPoint.y << "): "
                  << regions[i].totalPixels << " пикселей" << std::endl;
        for (auto &p : regions[i].colorCount) {
            auto [b,g,r] = p.first;
            std::cout << "   Цвет (" << r << "," << g << "," << b << "): " << p.second << std::endl;
        }
    }

    return 0;
}
