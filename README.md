DWANGO OpenToonz Plugins ([日本語](./README_ja.md))
=================

This repository contains example plugins for developers.
These plugins are depend on [opentoonz_plugin_utility](https://github.com/opentoonz/opentoonz_plugin_utility).
They require OpenCV3 runtime. [Visual C++ Redistributable Packages for Visual Studio 2013](https://www.microsoft.com/en-US/download/details.aspx?id=40784) is also required on Windows.

## How to use prebuilt plugins

[osx](https://github.com/opentoonz/dwango_opentoonz_plugins/releases/download/v1.0.0/dwango_opentoonz_plugins_osx.zip) and [win](https://github.com/opentoonz/dwango_opentoonz_plugins/releases/download/v1.0.0/dwango_opentoonz_plugins_win.zip) are zip archives which contain prebuilt plugins.
You can install them with following steps:

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
