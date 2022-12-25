#! /bin/sh

# https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip

BASEDIR="deps/glfw-3.3.8"

INFO="[\033[32mINFO\033[0m] "
ERROR="[\033[31mERROR\033[0m] "
WARNING="[\033[33mWARN\033[0m] "

read -r -p "This script will build GLFW in your system directory, continue? [Y/n] " inp

build_glfw() {
    echo $INFO"start building demo"
    glfw_build_dir=$BASEDIR/build
    if [ ! -d $glfw_build_dir ]; then
        mkdir $glfw_build_dir
    fi
    cd $glfw_build_dir
    cmake ..
    make -j4

    make install
    if [ $? -eq 0 ]; then
        echo $INFO"GLFW built successfully"
    else
        echo $ERROR"GLFW built failed, try sudo"
        exit -1
    fi
}

case $inp in
[yY][eE][sS] | [yY])
    build_glfw
    ;;
[nN][oO] | *)
    echo $WARNING"aborting"
    exit -1
    ;;
esac