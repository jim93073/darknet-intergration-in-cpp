import os
import pathlib
from shutil import copyfile

import setuptools
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext as build_ext_orig

VERSION = None
with open("VERSION.txt") as f:
    VERSION = f.readline().replace("\n", "").replace("\r", "")


class CMakeExtension(Extension):

    def __init__(self, name):
        """
        Source:
        https://stackoverflow.com/questions/42585210/extending-setuptools-extension-to-use-cmake-in-setup-py
        """
        # don't invoke the original build_ext for this special extension
        super().__init__(name, sources=[])


class build_ext(build_ext_orig):
    """
    Source:
    https://stackoverflow.com/questions/42585210/extending-setuptools-extension-to-use-cmake-in-setup-py
    """

    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        super().run()

    def build_cmake(self, ext):
        cwd = pathlib.Path().absolute()

        # these dirs will be created in build_py, so if you don't have
        # any python sources to bundle, the dirs will be missing
        build_temp = pathlib.Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir = pathlib.Path(self.get_ext_fullpath(ext.name))
        extdir.mkdir(parents=True, exist_ok=True)

        # example of cmake args
        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + str(extdir.parent.absolute()),
        ]

        # example of build args
        build_args = [
            '--', '-j4'
        ]

        print("Download Darknet from GitHub")
        self.spawn(["git", "submodule", "init"])
        self.spawn(["git", "submodule", "update", "--recursive"])

        print("Insert Darknet static library configuration")
        copyfile("DarknetStaticLib.cmake", "libs/darknet/DarknetStaticLib.cmake")

        self.spawn(["sed", "-i", "-r", 's/include\\(.*DarknetStaticLib.cmake\\).*//gm', "libs/darknet/CMakeLists.txt"])
        with open("libs/darknet/CMakeLists.txt", "a", encoding="utf-8") as f:
            f.write("\n" + 'include(${CMAKE_CURRENT_SOURCE_DIR}/DarknetStaticLib.cmake)')

        os.chdir(str(build_temp))
        self.spawn(['cmake', str(cwd)] + cmake_args)
        if not self.dry_run:
            self.spawn(['cmake', '--build', '.'] + build_args)
        # Troubleshooting: if fail on line above then delete all possible 
        # temporary CMake files including "CMakeCache.txt" in top level dir.
        os.chdir(str(cwd))

        # cleanup
        print("Cleanup")
        self.spawn(["sed", "-i", "-r", 's/include\\(.*DarknetStaticLib.cmake\\).*//gm', "libs/darknet/CMakeLists.txt"])


with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="yolotalk-core",
    version=VERSION,
    author="Li-Xian Chen",
    author_email="lxchen.cs09g@nctu.edu.tw",
    description="A C++ implementation of YOLO device for YoloTalk.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://gitlab.com/lxchen-lab117/yolotalk/darknet-integration-in-cpp",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "Operating System :: POSIX :: Linux",
    ],
    python_requires='>=3.6',
    ext_modules=[CMakeExtension('libyolotalk.so')],
    install_requires=[
        'numpy',
    ],
    cmdclass={
        'build_ext': build_ext,
    }
)