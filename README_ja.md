*This project has been created as part of the 42 curriculum by yhamada.*

## Description
この課題は、「食事する哲学者の問題」を「オフィスのUSBドングル争奪戦」に翻案したもので、デッドロック回避・飢餓状態回避・公平なスケジューリング（FIFO/EDF）・正確なタイマー管理という並行処理の四大テーマを、pthreadのミューテックスと条件変数だけで実装させるものである。

## Instructions

### 必要な環境
- C コンパイラ（`cc`）と `make`
- POSIX スレッド（pthread）が使える環境（Linux / macOS）

### コンパイル
リポジトリのルートで次を実行すると、実行ファイル `codexion` ができる。

```bash
make
```

`-Wall -Wextra -Werror -pthread` 付きでコンパイルされる。

| コマンド | 内容 |
|---|---|
| `make` / `make all` | `codexion` をビルドする |
| `make clean` | オブジェクトファイル（`obj/`）を削除する |
| `make fclean` | `clean` に加えて `codexion` も削除する |
| `make re` | `fclean` してからビルドし直す |

### 実行方法
引数は8つで、すべて必須。

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| 引数 | 単位 | 意味 |
|---|---|---|
| `number_of_coders` | 人 | コーダーの数（＝ドングルの数）。1 以上 |
| `time_to_burnout` | ms | 最後のコンパイル開始（またはシミュレーション開始）からこの時間内に次のコンパイルを始めないと燃え尽きる |
| `time_to_compile` | ms | コンパイルにかかる時間。この間ドングルを2つ持つ |
| `time_to_debug` | ms | デバッグにかかる時間 |
| `time_to_refactor` | ms | リファクタリングにかかる時間 |
| `number_of_compiles_required` | 回 | 全員がこの回数コンパイルしたらシミュレーションが終わる |
| `dongle_cooldown` | ms | ドングルを返してから、再び使えるようになるまでの時間 |
| `scheduler` | — | ドングルを誰に渡すかの規則。`fifo`（要求した順）または `edf`（燃え尽きる期限が近い順） |

数値は 0〜2147483647 の整数のみ受け付ける。負の数・小数・数字以外を含む値・`fifo` / `edf` 以外の scheduler はエラーになり、標準エラー出力に理由と使い方を表示して終了する（終了コード 1）。

### 実行例

```bash
# 誰も燃え尽きず、全員が7回コンパイルして終わる
./codexion 5 800 200 200 200 7 0 fifo

# 期限が短すぎて、誰かが燃え尽きる
./codexion 4 310 200 100 100 5 0 edf

# クールダウンあり
./codexion 4 600 100 100 100 5 50 edf

# コーダー1人（ドングルが1つしかないのでコンパイルできず燃え尽きる）
./codexion 1 800 200 200 200 5 0 fifo
```

### 出力
コーダーの状態が変わるたびに、標準出力に1行ずつ出る。先頭はシミュレーション開始からの経過時間（ms）、次がコーダー番号。

```
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
0 3 has taken a dongle
0 3 has taken a dongle
0 3 is compiling
200 1 is debugging
...
1204 3 burned out
```

シミュレーションは、誰かが燃え尽きたとき（`burned out` が最後の行になる）か、全員が `number_of_compiles_required` 回コンパイルしたときに終わる。

## Resources
- Introduction To Threads (pthreads) | C Programming Tutorial  
https://www.youtube.com/watch?v=ldJ8WGZVXZk&t=176s
- スレッドの紹介  
https://www.youtube.com/watch?v=LOfGJcVnvAk
- 食事する哲学者の問題  
https://www.youtube.com/watch?v=FYUi-u7UWgw
- 食の哲学者  
https://www.youtube.com/watch?v=iQVgyzgSUYM
### AI Resources
- subjectの翻訳
- pthreadについて理解するための壁打ち
- pthread関数の使用例の生成、解説
- 設計の相談
- 関数名の相談
- heap操作の解説
- スタベーションになるシナリオの図式化
- テストケースの作成

## Blocking cases handled
### デッドロック防止とコフマンの条件

#### デットロックのシナリオ
- 全員が同じタイミングで左側のドングルだけ確保してしまうと、誰も右側を取れず、円環状にお互いを待ち続けて永遠に止まる。(典型的デットロック)。

#### デットロック回避の方法
- コフマンの4条件（デットロックになりうる条件）のうち、どれかを崩す
  - 相互排他（Mutual Exclusion）資源（データや機器など）を同時に1つの処理しか使えない状態です。他の処理が使うには、終わるまで待つ必要があります。
  - 保持と待機（Hold and Wait）ある資源を自分で持ったまま、別の新しい資源が空くのを待っている状態です。
  - 横取り不可（Non-preemption）ほかの処理がすでに使っている資源を、強制的に奪い取ることができない状態です。持ち主が自分で手放すのを待つしかありません。
  - 循環待ち（Circular Wait）処理同士が輪（円環）のように、お互いの資源の解放を待ち合っている状態です（例：処理Aは処理Bの資源を待ち、処理Bは処理Aの資源を待つ）。  

#### デットロック対応策
  - 取り合いを一元管理する　→　スケジュール管理のため
  - 2つ同時に取れるときだけ実際に取る　→　ドングルを取得する際の分岐でチェック

### 飢餓状態の防止
#### スタベーションとは
- システム全体は動き続けているのに、特定の人だけがいつまでも資源をもらえない状態のこと。
- デッドロックと違って決まった形はなく、「誰を先にするか」のルールの弱点として起きる。なので、自分が選んだルールで何が起きるかを考える必要がある。

#### この課題で起こり得るスタベーション
1. **基本のルール**：ドングルごとに待ち行列を作り、前にいる人から取る（FIFO は要求した順、EDF は期限が近い順）。
2. **問題**：前の人がまだ取れないと、ドングルが空いている後ろの人まで待たされる（先頭ブロッキング）。4人でも1人ずつしかコンパイルできず、間に合うはずの条件でも燃え尽きた。
3. **追い越しを許すと飢餓が起きる**：2 を直すために「前の人が取れないなら追い越してよい」とすると、真ん中の人が困る。コーダーは左右2つのドングルが同時に空いていないと取れないので、両隣が交互に使い続けると、真ん中の人は永遠に取れない。

#### 対応策：期限に間に合うときだけ追い越しを許す（`src/scheduler_rival.c`）
- 前にいる人（R）を追い越してよいのは、次の両方を満たすときだけ。
  1. R が今すぐは取れない
  2. 自分が先に使っても、R の期限に間に合う（`現在時刻 + time_to_compile + dongle_cooldown + 5ms ≦ R の期限`）
- 1 で空いているドングルを無駄にせず、2 で追い越しによって R が燃え尽きることを防ぐ。R の期限が近づくと誰も R を追い越せなくなる。

### クールダウンの処理

#### 求められていること
- ドングルを返してから `dongle_cooldown` ms が経つまで、そのドングルは誰も取れない。

#### 実装
- ドングルごとに「次に使えるようになる時刻」（`dongle_cooldown_until[]`）を持つ。
- 返すときに `現在時刻 + dongle_cooldown` を記録する（`release_dongles`）。
- 取れるのは「空いている」かつ「その時刻を過ぎている」ときだけ（`dongle_ready`）。
- どちらの値も `dongle_mutex` の中で読み書きするので、返した直後の中途半端な状態を他のコーダーが見ることはない。

#### 待ち方
- 取れない理由によって待ち方を変えている（`wait_for_dangles`）。

| 取れない理由 | 待ち方 | 理由 |
|---|---|---|
| 誰かが使用中 | `pthread_cond_wait` | 返すときに `pthread_cond_broadcast` で起こしてもらえる |
| クールダウン中 | `pthread_cond_timedwait`（クールダウンが明ける時刻まで） | クールダウンは時間が経てば明けるだけで、誰も起こしてくれない。`cond_wait` だと寝たままになってしまう |

- 時刻を指定して寝るので、短い間隔で何度も確認する（ポーリング）必要がなく、CPU を無駄に使わない。

### 正確なバーンアウト検出

### ログのシリアライズ

## Thread synchronization mechanisms



- 解決策で対処した並行処理上の問題をすべて説明する「**Blocking cases handled**」セクション（例：デッドロック防止とコフマンの条件、飢餓状態の防止、クールダウンの処理、正確なバーンアウト検出、ログのシリアライズなど）。
- 実装で使用した具体的なスレッディングプリミティブ（`pthread_mutex_t`、`pthread_cond_t`、カスタムイベント実装）と、それらが共有資源（ドングル、ログ、モニターの状態）へのアクセスをどのように調整しているかを説明する「**Thread synchronization mechanisms**」セクション。競合状態がどのように防止され、コーダーとモニターの間でスレッドセーフな通信がどのように実現されているかの例を含めてください。