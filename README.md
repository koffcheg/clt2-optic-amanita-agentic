# clt2-optic
detection UAV by optical cameras

## Project structure
- camerapro - program for capture images
- datapro1 - program for process data from single camera
- datapro2 - program for combine and process results from several datapro1
- gtests - unit tests (google)
- common - whole project scope common units 
- test.mock - mock (testing) programs
  - cam-pro-mock (camerapro stub) - program for testing exchange vs cam-pro and dp1. It mocks camerapro functionality: generates some data and transfer it through “real-channel” to datapro1.mock (or datapro1). It’s for testing the exchange channel.
  - datapro1-mock (datapro1 stub) - program for testing receiving data from IPC. It mocks datapro1 functionality: saves receives data

### Useful data:
    /proc/sys/fs/mqueue/msg_max - max number messages in (posix) queue, read/write
    /dev/mqueue/ - virtual folder with posix-queue
    /dev/shm/ - virtual folder with posix shared-mem objects


## Building project manually
### Build&install log4cxx logger
- install deps:
>apt-get install build-essential libapr1-dev libaprutil1-dev gzip zip
- download  source code from https://logging.apache.org/log4cxx/latest_stable/download.html, ex: https://dlcdn.apache.org/logging/log4cxx/1.3.1/apache-log4cxx-1.3.1.tar.gz
- unpack
> tar zxvf apache-log4cxx-1.3.1.tar.gz
- go to src dir
> cd apache-log4cxx-1.3.1

> mkdir build 

>cd build

- configure build STATIC lib to CUSTOM dir:
> cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/home/u/lib/log4cxx-1.3.1 -DBUILD_SHARED_LIBS=OFF DBUILD_TESTING=OFF ..
- build :
>cmake --build . -j
- install built lib to custom dir:
>make install


### Build&install opencv-lib
- install some deps:
>sudo apt install libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
- download source code from repo: https://github.com/opencv/opencv (last release), ex: v-4.9.0: https://github.com/opencv/opencv/archive/refs/tags/4.9.0.zip
>wget -O opencv.zip https://github.com/opencv/opencv/archive/refs/tags/4.9.0.zip
- download source code opencv_contrib from repo: https://github.com/opencv/opencv_contrib (last release), ex: v-4.9.0: https://github.com/opencv/opencv_contrib/archive/refs/tags/4.9.0.zip
>wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/refs/tags/4.9.0.zip
- unpack
- after unpacking, go to opencv_contrib, from the “modules” folder inside it, select the xfeatures2d folder, and copy it to the Opencv “modules” folder.
- create folder for building
>mkdir build
- goto dir for building
>cd build
- configure STATIC build to CUSTOM dir:
>cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_opencv_world=ON -DBUILD_opencv_xfeatures2d=ON -DOPENCV_ENABLE_NONFREE=ON -DWITH_GTK=ON -DWITH_GTK_2_X=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/home/u/lib/opencv-4.9.0  ../opencv-4.9.0
- build with several thread (choose several less than CPU number):
>cmake --build .. -j12
- install built lib to custom dir:
>make install
    
### Clipper2 lib
- download source code from repo: https://github.com/AngusJohnson/Clipper2/tree/main (last release), ex: https://github.com/AngusJohnson/Clipper2/archive/refs/heads/main.zip
- unpack
- create folder for building
>mkdir build
- goto dir for building
>cd build
- configure STATIC build to CUSTOM dir (maybe disabling some features or tests or examples):
>cmake -DCMAKE_INSTALL_PREFIX=/home/u/lib/clipper2 ..
- build:
>cmake --build .
- install built lib to custom dir:
>make install
- on building main project use config: -DClipper2_DIR=/home/u/lib/clipper2/lib/cmake/clipper2
### Build&install fmt
Download and unzip the sources
```
wget -nc -O fmt.zip https://github.com/fmtlib/fmt/releases/download/11.0.2/fmt-11.0.2.zip
unzip -n fmt.zip
```
- prepare build directory
>mkdir -p build && cd build
- Initialize build (mind INSTALL_DIR)
>cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE ..
- Build and install
>sudo make -j`nproc` install

### Install some dependencies
- install jansson
> sudo apt-get install libjansson-dev

### Build the project with cmake
    mkdir build
    cd build
    cmake -Dlog4cxx_DIR=/home/u/lib/log4cxx-1.2.0/lib/cmake/log4cxx -DOpenCV_DIR=/home/u/lib/opencv-4.9.0/lib/cmake/opencv4 ..
    cmake --build ..

if opencv installed into system dir, then skip -DDOpenCV_DIR part of command

if log4cxx installed into system dir, then skip -DCMAKE_PREFIX_PATH part of command

## Building project with docker

Pre-install docker and docker-compose and go to the project's root<br>
Prepare build container<br>
```
./builder/prepare_docker.sh
```

Build project
```
docker compose run build
```

Or a separate component
```
docker compose run build_manager
docker compose run build_camerapro
docker compose run build_datapro1
docker compose run build_datapro2
docker compose run build_calibration
```

To manually access build container use:
```
docker compose run console
```


### Improvised precommit with docker

Following scriзt can be used for manual precommit. It will prevent you from pushing broken code into your branch<br>
Prerequisites:
- create private secure fork of original repo (git@github.com:coffeeman2010/clt2-optic.git)
- please prefer secure services like github.com or bitbucket.org;
- put following script into a `run_precommit.sh` file in a empty directory and configure `WORKING_BRANCH` and `FORK_REPO` variables:

```
sudo rm -rf ./workspace
mkdir workspace && cd workspace

WORKING_BRANCH=feature/manager
FORK_REPO=git@bitbucket.org:TheMamont/camproc.git
MAIN_REPO=git@github.com:coffeeman2010/clt2-optic.git

git clone -b $WORKING_BRANCH --single-branch $FORK_REPO .

if ./builder/prepare_docker.sh ; then
 echo "Docker container built successfully"
else
 echo "Docker container build error"
 exit 1
fi

if docker-compose run build ; then
 echo "App built successfully"
else
 echo "App build error"
 exit 1
fi

git remote add prod $MAIN_REPO
git push prod $WORKING_BRANCH
```
Then commit your code to your fork repro and run:
```
./run_precommit.sh
```
If precommit was successfull it will push a branch from your fork to the original repo, otherwise it will display an error

