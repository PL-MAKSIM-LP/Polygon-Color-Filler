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
   
     if (argc < 2) {
        std::cout << "Usage: ./app <image_path>" << std::endl;
        return -1;
    }


    // Загрузка изображения
    cv::Mat image = cv::imread(argv[1]);

    if (image.empty()) {
        std::cout << "Не удалось загрузить изображение!" << std::endl;
        return -1;
    }

    // Создаем маску для floodFill
    cv::Mat mask = cv::Mat::zeros(image.rows + 2, image.cols + 2, CV_8UC1);

    int count = 0; // счетчик областей

    // Проходим по всем пикселям
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            cv::Vec3b color = image.at<cv::Vec3b>(y, x);

            // Если пиксель белый и не залит
            if (color[0] > 240 && color[1] > 240 && color[2] > 240 && mask.at<uchar>(y + 1, x + 1) == 0) {
                
                // Новый случайный цвет
                cv::Scalar newColor(rand() % 256, rand() % 256, rand() % 256);

                // Заливка области
                cv::Rect rect;
                cv::floodFill(image, mask, cv::Point(x, y), newColor, &rect,
                              cv::Scalar(10, 10, 10), cv::Scalar(10, 10, 10), 4);
                count++;
            }
        }
    }
    
    cv::imwrite("file.png", image);
    std::cout << "Всего залито областей: " << count << std::endl;

    // Ждем, чтобы пользователь увидел финальный результат
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}
