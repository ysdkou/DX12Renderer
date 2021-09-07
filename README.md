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

##動画
link:[youtube](https://youtu.be/jouK1hIypUA)
[![](https://img.youtube.com/vi/jouK1hIypUA/0.jpg)](https://www.youtube.com/watch?v=jouK1hIypUA)

