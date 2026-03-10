PROJECT_ROOT=/root/code

cd $PROJECT_ROOT
mkdir _cmake_build
cd _cmake_build

cmake -Dlog4cxx_DIR=/build_libs/log4cxx/log4cxx-1.3.1/lib/cmake/log4cxx -DOpenCV_DIR=/build_libs/opencv/opencv-4.9.0/lib/cmake/opencv4 ..
make -j`nproc` datapro1
