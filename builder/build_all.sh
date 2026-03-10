PROJECT_ROOT=/root/code

cd $PROJECT_ROOT
mkdir -p _cmake_build
cd _cmake_build

cmake -Dlog4cxx_DIR=/build_libs/log4cxx/log4cxx-1.3.1/lib/cmake/log4cxx -DOpenCV_DIR=/build_libs/opencv/opencv-4.9.0/lib/cmake/opencv4 ..
make -j`nproc` camerapro
make -j`nproc` datapro1
make -j`nproc` datapro2
make -j`nproc` turretpro
make -j`nproc` manager
make -j`nproc` calibration
make -j`nproc` dummy
