# Darknet Integration in Cpp

## 簡介

這個Repository使用C++實作YoloTalk當中的YOLO Device，並提供一個初階的Python Wrapper。關於較完善的Python Wrapper版本請參見[Darknet Integration](https://gitlab.com/lxchen-lab117/yolotalk/darknet-integration)。

這個版本的YOLO Device是基於YOLO作者提供的框架Darknet，並且依照YoloTalk的需求添加上一些功能 (如添加辨識範圍)。為了不去修改Darknet原始程式而造成維護上的困難，這個Repository當中有一個檔案`DarknetStaticLib.cmake`。這個檔案在程式編譯前會被插入Darknet的`CMakeLists.txt`當中，使得Darknet能夠被編譯成一個Static Library。如此，這個Repository就可以把Darknet當作Library使用。

**[Note]** Darknet和本Repository的程式會分開編譯，Darknet先編譯完成後再編譯本程式。

## 使用說明

**[Note]** 大部分時候請直接使用較完整的[Darknet Integration](https://gitlab.com/lxchen-lab117/yolotalk/darknet-integration)，不要使用本Repository的程式。Darknet Integration是基於這個Reporsitory的程式擴充YoloDevice的功能，如加入Deep SORT等功能。

這裡說明如何使用Python呼叫本Repository的程式。

1. 安裝相依套件

    ```bash
    sudo apt-get install -y cmake git build-essential
    ```

2. 安裝本程式與其Python Wrapper

    ```bash
    pip install git+https://gitlab.com/lxchen-lab117/yolotalk/darknet-integration.git
    ```

3. 在Python檔中您可以使用以下程式碼呼叫初階的`YoloDevice`物件。

    ```python
    import libyolotalk.core as lyt
    device = lyt.YoloDevice(
        cfg="./weights/yolov4.cfg",
        weights="./weights/yolov4.weights",
        name_list="./weights/coco.names",
        url="rtsp://iottalk:iottalk2019@140.113.237.220:554/live2.sdp",
        show_msg=True,
        polygon=[(10, 20), (30, 40), (50, 60)],
    )
    ```

## 開發說明

### Requriements

```bash
# Install libopencv-dev
sudo apt-get install libopencv-dev

# Clone this repository
git clone --recursive https://gitlab.com/lxchen-lab117/yolotalk/darknet-intergration-in-cpp.git

# Download darknet
git submodule init
git submodule update --recursive
```

### Compile

There are 3 ways to compile this program:

#### Using CMake with build.sh

```bash
bash build.sh
```

Done. The compiled program is in `./builds/main`.

#### Using CMake manually

```bash
# Insert the CMake file of Darknet static library
cp DarknetStaticLib.cmake libs/darknet
NEW_LINE="include(\${CMAKE_CURRENT_SOURCE_DIR}/DarknetStaticLib.cmake)"
echo ${NEW_LINE} >> libs/darknet/CMakeLists.txt

# Start building
cd builds
cmake ../
make -j $(nproc --all)

# Cleanup
cd ../
sed -i -r 's/include\(.*DarknetStaticLib.cmake\).*//gm' libs/darknet/CMakeLists.txt
rm libs/darknet/DarknetStaticLib.cmake
```

Done. The compiled program is in `./builds/main`.

#### Using GCC

The command below may not work. It is recommended to use CMake and Make instead of GCC.

```bash
cd src
gcc -I /mnt/d/PCSLab/YoloTalk/darknet-c/include -L /mnt/d/PCSLab/YoloTalk/darknet-c/linux_x64_cpu  main.cpp -l darknet -o main pkg-config --cflags --libs opencv
```
