DWANGO OpenToonz Plugins
=================

[opentoonz_plugin_utility](https://github.com/opentoonz/opentoonz_plugin_utility) を利用し、Dwangoが開発した Plugin のサンプルです。実行には `OpenCV` のランタイムが必要になります。
Windows 環境では、[Visual Studio 2013 の Visual C++ 再頒布可能パッケージ](https://www.microsoft.com/ja-jp/download/details.aspx?id=40784) も必要です。

## ビルド済みプラグインの利用

ここのプラグインを利用する最も簡単な方法は、同行しているビルド済みプラグインをそのまま利用することだと思います。

`bin/{osx,win}/dwango_opentoonz_plugins.zip`がここで紹介しているプラグインをビルドしてzipに固めたものなので、これを展開して、中の.pluginファイルのうち利用したいものを `{Opentoonzのインストールされたディレクトリ}/plugins/` 以下にコピーしてください。OSXなら`/Applications/OpenToonz/OpenToonz_1.0_stuff/plugins/`、windowsなら`C:\OpenToonz 1.0 stuff\plugins\`であることが多いと思います。

この後OpenToonzを再起動すればプラグインが追加されますが、OpenCV3を利用しているため、そちらの準備ができていないとプラグインが認識されません。Opencv3については、以下のインストール方法に従ってください。

## OpenCV3 のインストール

ここで配布しているプラグインを利用するためには、OpenCV3がインストールされている必要があります。

### OSX

homebrew のインストールされている OSX では下記コマンドで OpenCV3 をインストールできます。

```
brew install opencv3
brew ln opencv3 --force
```

### Windows

Windows では [OpenCV for Windows VERSION 3.1](http://opencv.org/) をダウンロードして利用してください。
実行には `opencv\build\x64\vc12\bin` 以下の dll (`opencv_world310.dll` など) を Toonz 本体から参照できるパスの通っているディレクトリ (たとえば実行ファイルのあるディレクトリ) に配置する必要があります。
