FOLDER="build"
if [ ! -d "$FOLDER" ]; then
    mkdir -p "$FOLDER"
fi

> result.txt
echo "$(grep 'model name' /proc/cpuinfo | uniq | awk -F': ' '{print $2}'), $(grep -c '^processor' /proc/cpuinfo) cores" >> result.txt 

folder=${DATA_PATH:-"./data/main/"}
count=${REPEATS:-10}
mainfile="main.cpp"

echo OneThread_O0 >> result.txt 
g++ $mainfile -O0 -o ./build/app  `pkg-config --cflags --libs opencv4` && 
echo "Run application with 0 level of optimization" && 
./build/app $folder $count && echo

echo OneThread_O1 >> result.txt 
g++ $mainfile -O1 -o ./build/app  `pkg-config --cflags --libs opencv4` && 
echo "Run application with 1 level of optimization" && 
./build/app $folder $count && echo

echo OneThread_O2 >> result.txt
g++ $mainfile -O2 -o ./build/app  `pkg-config --cflags --libs opencv4` && 
echo "Run application with 2 level of optimization" && 
./build/app $folder $count && echo

echo OneThread_O3 >> result.txt
g++ $mainfile -O3 -o ./build/app  `pkg-config --cflags --libs opencv4` && 
echo "Run application with 3 level of optimization" && 
./build/app $folder $count && echo

echo MultithreadThread_O0 >> result.txt 
g++ $mainfile -O0 -o ./build/app  `pkg-config --cflags --libs opencv4` && 
echo "Run Multithread application with 0 level of optimization" && ./build/app $folder $count 1 && echo

echo MultithreadThread_O1 >> result.txt 
g++ $mainfile -O1 -o ./build/app  `pkg-config --cflags --libs opencv4` && 
echo "Run Multithread application with 1 level of optimization" && ./build/app $folder $count 1 && echo

echo MultithreadThread_O2 >> result.txt 
g++ $mainfile -O2 -o ./build/app  `pkg-config --cflags --libs opencv4` && 
echo "Run Multithread application with 2 level of optimization" && 
./build/app $folder $count 1 && echo

echo MultithreadThread_O3 >> result.txt 
g++ $mainfile -O3 -o ./build/app  `pkg-config --cflags --libs opencv4` && 
echo "Run Multithread application with 3 level of optimization" && 
./build/app $folder $count 1 && echo

python buildHistogram.py 