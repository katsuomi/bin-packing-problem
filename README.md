# bin-packing-problem
The bin packing problem (BPP) is the problem of finding a solution to put some items in several bins. All items must go into one of the bins,
and make sure that the items do not overlap. No part of the item should protrude from the bin. 
It is not possible to rotate all items and bins, because items and bins are handled in two-dimensional space. 
The objective is to minimize the amount of bin usage. In this paper, we use a simple search algorithm to derive a solution to the problem.

# 1. はじめに
二次元ビンパッキング問題とは, 
与えられた長方形の製品を横幅と縦幅がともに不変な長方形の母材に詰め込み, 使用する母材の量の最小化を目的とする問題であり, 代表的な組合せ最適化問題の一つである.
本研究では, 二次元ビンパッキング問題に対して, 近似解を求めるアルゴリズムを提案する.

# 2. 問題説明
本研究で扱う二次元ビンパッキング問題では, 扱う母材, 詰め込む製品は全て長方形であり, 母材や製品の回転は許さない. 
また, 全ての製品が, 母材からはみ出ることなく詰め込まれるものとし, 製品の重なりを許さない.
以下では, 用語定義をを2.1 節, 入力を2.2 節, 制約条件を2.3 節, 出力を2.4 節で説明する.

## 2-1. 用語定義
本研究で用いるいくつかの用語を以下に定義する.

• ビン
横幅と縦幅がともに不変な長方形の母材をビンと定義する. 
ビンb は, 幅Wb, 高さHb を持つ.
ビンの集合をB = {1, 2, 3} と記す. 全てのビンの幅と高さはWb = 6000, Hb = 3210 と一定である.

• アイテム
詰め込む長方形をアイテムと呼ぶ. アイテム数を n とし, 詰め込むアイテムの集合をI = {1, 2, . . ., n} とする. 各製品i は, 幅Wi, 高さHi を持つ.

• 列
全てのアイテムをビンに格納した時, 属しているビンが同じかつx 座標が同じアイテムの集合を列と定義する.

• 右の空き容量
全てのアイテムをビンに格納した時, あるビンに着目し, そのビンの右端のアイテムとビンの幅との空いた容量を右の空き容量と定義する.

## 2-2. 入力
本研究で取り扱う入力は以下の通りである.

• アイテム集合I

## 2-3. 制約条件
本研究で取り扱う制約条件の詳細は以下の通りである.

• 配置制約

全てのアイテムがいずれかのビンに属し, 互いのアイテムの重なりを許さない. また, 全てのアイテムが, ビンからはみ出ることなく配置される.

• 回転制約

ビンやアイテムの回転は許さない.

## 2-4. 出力
本研究で取り扱う出力は以下の通りである.

• 使用したビンの数
• ビンの充填率

充填率を, 以下のように定義する:
配置するアイテムの面積合計/ビンの面積合計
(最後に使用されたビンの面積のみ, そのビンに配置されたアイテムの最も右のx 座標×ビンの高さ)

# 3. アルゴリズム

提案するアルゴリズムの手続きは以下である.

-----------
Algorithm
Input: アイテム集合I
Output: 使用したビンの数, それぞれのビンの充填率

Step 1. 全てのアイテムを幅順で降順ソートする[1].

Step 2. ある列に着目し, その列に格納されたアイテムが全て, 他の列に格納できる場合, 他の列に格納する.

Step 3. Step 2 を満たす列がなくなるまで, Step 2 を繰り返す.

Step 4. ある列に着目し, その列に格納されたアイテムが全て, あるビンの右の空き容量に格納できる場合, その右の空き容量に格納する.

Step 5. Step 4 を満たす列がなくなるまで, Step 4 を繰り返す.
-----------

# 4. 計算実験

## 4-1. 実験環境
実験は, i Mac (CPU: 3.5 GHz Intel Core i5, メモリ:32.00 GB 2400 MHz DDR4) を用いて行う. 使用言語はC である.

## 4-2. 実験結果
表1 に, 与えられた問題例に対する実験結果を示す.

<img width="940" alt="スクリーンショット 2020-06-23 17 05 57" src="https://user-images.githubusercontent.com/36298285/85377302-dc4b2480-b573-11ea-9613-b82bf22451e4.png">

# 5. 参考文献
[1] 茨木俊秀, C によるアルゴリズムとデータ構造,オーム社(2014)







