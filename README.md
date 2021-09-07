# DX12DefferedRendringDemo
DX12で制作したレンダリングデモ作品です。

ディファードシェーディング、物理ベースの計算モデルを採用しています。

## GBuffer構成
### GBuffer0
Format:R8G8B8A8Unorm
RGB:Albedo
A:Rougness

### GBuffer1
Format:R8G8B8A8Unorm
RGB:WorldNormal
A:Metallic

### GBuffer2
Format:R8G8B8A8Unorm
R:AO(ammbient)
GBA:未使用

### その他
※ワールド座標はDepthBufferから復元しています。

## シェーダーモデル
UE4で採用されている、物理ベースの計算モデルを採用しています。
以下のサイトを参考にいたしました。

https://learnopengl.com/PBR/Theory

## ソースコード概説
主な描画処理はDefferedRenderer.cppで行っています。
シェーダーは以下の構成となります。

GBBufferパスシェーダー:GBufferPassPixel.hlsl GBufferPassVertex.hlsl
Lightパスシェーダー:LightPassPixel.hlsl LightPassVertex.hlsl
## 動画

[![](https://img.youtube.com/vi/jouK1hIypUA/0.jpg)](https://www.youtube.com/watch?v=jouK1hIypUA)

## 実行ファイル
x64\Debug\\Project3.exe

### 以下のチャンネルでその他デモ作品もアップロードしています
[link](https://www.youtube.com/channel/UC01yHOlNz_1FV1cxvnLI26A)
