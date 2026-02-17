FOLDER="build"
if [ ! -d "$FOLDER" ]; then
    mkdir -p "$FOLDER"
fi

> result.txt
echo "$(grep 'model name' /proc/cpuinfo | uniq | awk -F': ' '{print $2}'), $(grep -c '^processor' /proc/cpuinfo) cores" >> result.txt 

count=5
folder="./data/main/"

echo OneThread_O0 >> result.txt 
g++ main.cpp -O0 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run application with 0 level of optimization" && ./build/app $folder $count && echo

echo OneThread_O1 >> result.txt 
g++ main.cpp -O1 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run application with 1 level of optimization" && ./build/app $folder $count && echo

echo OneThread_O2 >> result.txt
g++ main.cpp -O2 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run application with 2 level of optimization" && ./build/app $folder $count && echo

echo OneThread_O3 >> result.txt
g++ main.cpp -O3 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run application with 3 level of optimization" && ./build/app $folder $count && echo

echo MultithreadThread_O0 >> result.txt 
g++ mainMultithread.cpp -O0 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run Multithread application with 0 level of optimization" && ./build/app $folder $count && echo

echo MultithreadThread_O1 >> result.txt 
g++ mainMultithread.cpp -O1 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run Multithread application with 1 level of optimization" && ./build/app $folder $count && echo

echo MultithreadThread_O2 >> result.txt 
g++ mainMultithread.cpp -O2 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run Multithread application with 2 level of optimization" && ./build/app $folder $count && echo

echo MultithreadThread_O3 >> result.txt 
g++ mainMultithread.cpp -O3 -o ./build/app  `pkg-config --cflags --libs opencv4` && echo "Run Multithread application with 3 level of optimization" && ./build/app $folder $count && echo

python buildHistogram.py 