DWANGO OpenToonz Plugins
=================

`dwango_opentoonz_plugins`, which use [opentoonz_plugin_utility](https://github.com/opentoonz/opentoonz_plugin_utility), are plugin examples for plugin developers.
The plugins require OpenCV3 runtime. [Visual C++ Redistributable Packages for Visual Studio 2013](https://www.microsoft.com/en-US/download/details.aspx?id=40784) is also required on Windows.

## How to use prebuilt plugins

`bin/{osx,win}/dwango_opentoonz_plugins.zip` is a zip archive which containes prebuild plugins.

0. Copy `.plugin` files, which you want, to `${path-to-opentoonz-stuff}/plugins/`.
  - `${path-to-opentoonz-stuff}` is `/Applications/OpenToonz/OpenToonz_1.0_stuff/plugins/` (OSX) or `C:\OpenToonz 1.0 stuff\plugins` (Windows) by default.
0. Install `OpenCV3`.
0. Restert OpenToonz.

Plugin manual is [here](./doc/sample_plugins_manual.md).

### How to install OpenCV3

#### OSX

You can install `OpenCV3` by `homebrew`.

```
brew install opencv3
brew ln opencv3 --force
```

#### Windows

0. Download [OpenCV for Windows VERSION 3.1](http://opencv.org/).
0. Set `PATH` to `${path-to-opencv3}\build\x64\vc12\bin\`,  
  - Or copy `${path-to-opencv3}\build\x64\vc12\bin\opencv_world310.dll` to `C:\Program Files\OpenToonz 1.0`. 
