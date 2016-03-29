DWANGO OpenToonz Plugins
=================

[opentoonz_plugin_utility](https://github.com/opentoonz/opentoonz_plugin_utility) を利用し、DWANGO が開発したプラグインの実装サンプルです。実行には `OpenCV3` のランタイムが必要になります。
Windows 環境では、[Visual Studio 2013 の Visual C++ 再頒布可能パッケージ](https://www.microsoft.com/ja-jp/download/details.aspx?id=40784) も必要です。

## ビルド済みプラグインの利用

プラグインを利用する最も簡単な方法は、ビルド済みプラグインをそのまま利用することです。

`bin/{osx,win}/dwango_opentoonz_plugins.zip` がここで紹介しているプラグインをビルドして zip に固めたものなので、これを展開して、中の `.plugin` ファイルのうち使用したいものを `{OpenToonz の Stuff フォルダ}/plugins/` 以下にコピーしてください。OSX なら `/Applications/OpenToonz/OpenToonz_1.0_stuff/plugins/`、Windows なら `C:\OpenToonz 1.0 stuff\plugins` がデフォルトのパスです。

その後、OpenToonz を再起動すればプラグインが追加されますが、`OpenCV3` を利用しているため、そちらの準備ができていないと `OpenToonz` が起動されなくなります。`OpenCV3` については、以下のインストール方法に従ってください。

個々のプラグインについては、[こちらのマニュアル](./doc/sample_plugins_manual.md) を参照してください。


## OpenCV3 のインストール

### OSX

homebrew のインストールされている OSX では下記コマンドで `OpenCV3` をインストールできます。

```
brew install opencv3
brew ln opencv3 --force
```

### Windows

Windows では [OpenCV for Windows VERSION 3.1](http://opencv.org/) をダウンロードして利用してください。
実行には `opencv\build\x64\vc12\bin` 以下の dll (`opencv_world310.dll` など) を Toonz 本体から参照できるパスの通っているディレクトリ (たとえば実行ファイルのあるディレクトリ) に配置する必要があります。
