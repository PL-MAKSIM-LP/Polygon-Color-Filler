count=10

g++ main.cpp -O0 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run application with 0 level of optimization" && ./build/app ./data/main/ $count && echo
g++ main.cpp -O1 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run application with 1 level of optimization" && ./build/app ./data/main/ $count && echo
g++ main.cpp -O2 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run application with 2 level of optimization" && ./build/app ./data/main/ $count && echo
g++ main.cpp -O3 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run application with 3 level of optimization" && ./build/app ./data/main/ $count && echo

g++ mainMultithread.cpp -O0 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run Multithread application with 0 level of optimization" && ./build/app ./data/main/ $count && echo
g++ mainMultithread.cpp -O1 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run Multithread application with 1 level of optimization" && ./build/app ./data/main/ $count && echo
g++ mainMultithread.cpp -O2 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run Multithread application with 2 level of optimization" && ./build/app ./data/main/ $count && echo
g++ mainMultithread.cpp -O3 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run Multithread application with 3 level of optimization" && ./build/app ./data/main/ $count && echo