# 这是什么
小妹直播, master是原版obs, gonghui分支是开发版. 编译请切换到gonghui分支.

# 编译方法
## macOS
* 安装homebrew
* 通过homebrew安装ffmpeg, qt, x264, xvid, freetype
* 安装XQuartz
* 去JetBrains下载一个CLion
* 设置环境变量QTDIR, 用brew安装的应该是在```/usr/local/opt/qt```
* 用CLion打开项目根目录即可, 它会自动执行cmake配置, 缺省的编译目录是```cmake-build-debug```

## Windows 10
* 安装Visual Studio 2017
* 安装CMake
* 安装qt
* 设置环境变量QTDIR指向```qt安装目录/5.9.1/msvc2017_64```, 如果要编译32位版本则换成其它目录, 但是经过一些实验, 用msvc2017_64似乎好一些, 其它的有些莫名其妙的部署问题
* 设置环境变量DepsPath指向```项目目录/win_deps_vs2015/win64```, 如果要编译32位版本则把```win64```换成```win32```, 推荐编译64位版本, 理由同上
* 打开CMake GUI, 源代码目录设置为项目目录, 编译目录设置为```项目目录/build```, 依次点击Configure, Generate, Open Project, 直接编译即可