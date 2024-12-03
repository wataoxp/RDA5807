# RDA5807

## Overview
RadioIC RDA5807M for STM32

## Requirement
* STM32CubeIDE
* STM32G030F6P6(Cortex-M0+)
* Nucleo F446RE(Cortex-m4)
* Clang

## Description
Communicate with TeraTerm using UART

Edited File
* main.c
* RDA5807.c
* RDA5807.h
* menu.c
* menu.h

##備考
HALライブラリを使用しているので、さまざまなSTM32ファミリで動作させることが可能かと思います。
* Nucleoボードを使用する場合
デフォルトでUARTが有効になっています。ボーレート等を確認してTeraTerm側の設定も済ませてください。
またI2Cを利用します。こちらも.iocファイルから設定してください。

* BluePill、BlackPill等のボード、またはMCU単体を使用する場合
ST-LINKやUSBシリアル変換でPCと通信ができるように接続してください。
こちらではデフォルトでUARTも設定されていないので、有効にしてください(デフォルトの設定を利用しています)

* 編集箇所
Radio.cおよびmenu.cはUART、I2Cの情報をアドレスで受け取るようになっているので、基本的に編集は不要です。
.iocファイルからmain.cを自動生成する場合、以下の部分を編集してください。
 1.Includes
 2.各種変数の宣言
 3.main関数内のInit、whileループの内容

main.cをそのままコピーし、その後.iocファイルを編集するのがおすすめです。
UARTやI2Cの名前が異なることでビルドエラーが出ますので、その箇所を修正してください。

## Licence
[MIT](https://github.com/wataoxp/Radio/blob/main/LICENSE)



 
