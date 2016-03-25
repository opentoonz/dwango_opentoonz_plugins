DWANGO OpenToonz Plugins
=================

[opentoonz_plugin_utility](https://github.com/opentoonz/opentoonz_plugin_utility) を利用し、Dwangoが開発した Plugin のサンプルです。実行には `OpenCV` のランタイムが必要になります。
Windows 環境では、[Visual Studio 2013 の Visual C++ 再頒布可能パッケージ](https://www.microsoft.com/ja-jp/download/details.aspx?id=40784) も必要です。

## OpenCV3 のインストール

### OSX

homebrew のインストールされている OSX では下記コマンドで OpenCV3 をインストールできます。

```
brew install opencv3
brew ln opencv3 --force
```

### Windows

Windows では [OpenCV for Windows VERSION 3.1](http://opencv.org/) をダウンロードして利用してください。
実行には `opencv\build\x64\vc12\bin` 以下の dll (`opencv_world310.dll` など) を Toonz 本体から参照できるパスの通っているディレクトリ (たとえば実行ファイルのあるディレクトリ) に配置する必要があります。
