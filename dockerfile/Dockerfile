FROM nvidia/cuda:10.0-cudnn7-devel-ubuntu18.04

ARG DEBIAN_FRONTEND=noninteractive

ENV TZ=Asia/Taipei
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update

RUN apt-get install -y wget &&\
    git

RUN apt-get install -y libopencv-dev

# Install CMake
RUN version=3.18 &&\
    build=1 &&\
    mkdir -p ~/temp &&\
    cd ~/temp &&\
    wget https://cmake.org/files/v$version/cmake-$version.$build-Linux-x86_64.sh &&\
    mkdir /opt/cmake &&\
    sh cmake-$version.$build-Linux-x86_64.sh --prefix=/opt/cmake --skip-license

RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
