FROM ubuntu:22.04 as build-env


RUN apt update
RUN apt install -y lsb-release build-essential libtool autoconf gzip zip unzip wget libssl-dev
RUN apt install -y gcc g++
RUN apt install -y software-properties-common
RUN apt install -y libaprutil1-dev
RUN apt install -y libapr1-dev libaprutil1-dev 

RUN mkdir -p /build_libs/boost 
RUN cd /build_libs/boost && \
	wget https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.gz && tar zxvf boost_1_85_0.tar.gz && cd boost_1_85_0 && \
	./bootstrap.sh --with-toolset=gcc CC=/usr/bin/gcc CXX=/usr/bin/g++ && \
	./b2 -j`nproc` cxxstd=20 --build-type=minimal --layout=system variant=debug link=static threading=multi runtime-link=static install

ARG CMAKE_VERSION=3.28
ARG CMAKE_BUILD=6
ARG CMAKE_ARCHIVE=cmake-$CMAKE_VERSION.$CMAKE_BUILD.tar.gz
ARG CMAKE_URL=https://cmake.org/files/v$CMAKE_VERSION/$CMAKE_ARCHIVE
RUN mkdir -p /build_libs/cmake
RUN cd /build_libs/cmake && wget $CMAKE_URL && tar -xzvf $CMAKE_ARCHIVE && cd cmake-$CMAKE_VERSION.$CMAKE_BUILD/ && \
	./bootstrap --parallel=`nproc` && make -j`nproc` && make install

RUN mkdir -p /build_libs/log4cxx
RUN cd /build_libs/log4cxx && \
    wget https://dlcdn.apache.org/logging/log4cxx/1.3.1/apache-log4cxx-1.3.1.tar.gz && \
    tar zxvf apache-log4cxx-1.3.1.tar.gz && \
    cd apache-log4cxx-1.3.1 && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/build_libs/log4cxx/log4cxx-1.3.1 -DBUILD_SHARED_LIBS=OFF DBUILD_TESTING=OFF .. && \
    make -j`nproc` && \
    make install

RUN apt install -y libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev

RUN mkdir -p /build_libs/opencv
RUN cd /build_libs/opencv && \
    wget -O opencv.zip https://github.com/opencv/opencv/archive/refs/tags/4.9.0.zip && \
    wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/refs/tags/4.9.0.zip && \
    unzip opencv.zip && \
    unzip opencv_contrib.zip && \
    cp -r ./opencv_contrib-4.9.0/modules/xfeatures2d ./opencv-4.9.0/modules/ && \
    mkdir build_opencv && cd build_opencv && \
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_opencv_world=ON -DBUILD_opencv_xfeatures2d=ON -DOPENCV_ENABLE_NONFREE=ON -DWITH_GTK=ON -DWITH_GTK_2_X=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/build_libs/opencv/opencv-4.9.0 ../opencv-4.9.0 && \
    make -j`nproc` && \
    make install

RUN mkdir -p /build_libs/clipper2
RUN cd /build_libs/clipper2 && \
    wget -O clipper2.zip https://github.com/AngusJohnson/Clipper2/releases/download/Clipper2_1.4.0/Clipper2_1.4.0.zip && \
    unzip clipper2.zip && \
    cd CPP && \
    mkdir build_clipper2 && cd build_clipper2 && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCLIPPER2_UTILS=OFF -DCLIPPER2_EXAMPLES=OFF -DCLIPPER2_TESTS=OFF .. && \
    cmake --build . -j`nproc` && \
    make install

RUN mkdir -p /build_libs/fmt
RUN cd /build_libs/fmt && \
    wget -O fmt.zip https://github.com/fmtlib/fmt/archive/refs/tags/11.0.2.zip && \
    unzip fmt.zip && \
    cd fmt-11.0.2 && \
    mkdir build_fmt && cd build_fmt && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE .. && \
    cmake --build . -j`nproc` && \
    cmake --install .


RUN apt install -y libjansson-dev




	


