# Codexion 実装ロードマップ
### Norm準拠での段階的実装計画

---

## 全体像

```
Phase 0: 準備・設計          [着手前]
Phase 1: 骨組み（Makefile/構造体/引数パース）
Phase 2: 単一コーダーでの状態遷移ループ
Phase 3: ドングル資源とミューテックス導入（2人〜）
Phase 4: クールダウン機能
Phase 5: FIFOスケジューラ（自作優先度付きキュー）
Phase 6: モニタースレッド＋バーンアウト検出（10ms精度）
Phase 7: EDFスケジューラ＋タイブレーク
Phase 8: 終了処理（正常終了・全スレッドの安全な停止）
Phase 9: Norm適合・メモリリーク・データ競合の検証
Phase 10: README作成・提出準備
```

各フェーズは「**動くものを一つ積み上げてから次へ**」という順序になっています。並行処理のバグは後から差し込むと原因特定が困難なため、**常に「今動いている状態」を保ちながら機能を足す**のが鉄則です。

---

## Phase 0：準備・設計（コードを書く前）

### やること
- [ ] 「哲学者の食事問題」を紙の上で一度整理する（コーダー＝哲学者、ドングル＝箸）
- [ ] 状態遷移図を描く：`ドングル待ち → 取得 → compiling → debugging → refactoring → (再びドングル待ち)`
- [ ] コフマンの4条件のうち、どれを崩してデッドロックを防ぐか決める（推奨：**仲介者方式＋All-or-Nothing**）
- [ ] 終了条件のフローを図示する：「誰かバーンアウト」or「全員規定回数コンパイル」
- [ ] ディレクトリ構成を決める（`.c`は最大5関数/ファイルという制約から、ファイル分割が必須）

### 想定ディレクトリ構成（Norm準拠）

```
codexion/
├── Makefile
├── include/
│   └── codexion.h
└── src/
    ├── main.c              (main, 初期化呼び出しのみ)
    ├── parsing.c           (引数パース・バリデーション)
    ├── init.c              (構造体の初期化、mutex/cond init)
    ├── cleanup.c           (destroy, free)
    ├── coder_routine.c     (コーダースレッドのメインループ)
    ├── coder_actions.c     (compile/debug/refactorの各動作)
    ├── dongle.c            (ドングル取得・解放ロジック)
    ├── monitor.c           (モニタースレッド)
    ├── scheduler_fifo.c    (FIFOキュー)
    ├── scheduler_edf.c     (EDFヒープ)
    ├── heap.c              (自作優先度付きキュー本体)
    ├── time_utils.c        (gettimeofdayラッパー、現在時刻取得)
    └── log.c               (ログ出力、mutexで保護)
```

### 構造体設計の下書き（`.h`に書く）

```c
typedef struct s_dongle
{
    int             id;
    int             in_use;
    long            cooldown_until_ms;
    pthread_mutex_t mutex;
}   t_dongle;

typedef struct s_coder
{
    int             id;
    int             compile_count;
    long            last_compile_start_ms;
    int             burned_out;
    t_dongle        *left;
    t_dongle        *right;
    struct s_simulation *sim;   // 共有状態への参照
}   t_coder;

// 全体で共有される情報
typedef struct s_simulation
{
    int             number_of_coders;
    long            time_to_burnout;
    long            time_to_compile;
    long            time_to_debug;
    long            time_to_refactor;
    int             compiles_required;
    long            dongle_cooldown;
    int             scheduler_type;      // 0=fifo, 1=edf
    t_coder         *coders;
    t_dongle        *dongles;
    pthread_mutex_t log_mutex;
    pthread_mutex_t state_mutex;
    pthread_cond_t  state_cond;
    int             simulation_over;
    long            start_time_ms;
}   t_simulation;
```

> **チェック**：グローバル変数を使っていないか？（`t_simulation`は必ず`main`でローカル変数として確保し、各スレッドにはポインタで渡す）

---

## Phase 1：骨組み（Makefile・構造体・引数パース）

### やること
- [ ] `Makefile`を先に作る（`$(NAME)`, `all`, `clean`, `fclean`, `re`）
- [ ] ヘッダーファイル（インクルードガード込み）を作る
- [ ] 引数パース＆バリデーション（`while`ループで実装、`for`禁止に注意）
  - 引数の個数チェック（8個ちょうどか）
  - 各引数が正の整数か（`atoi`だけでなく、文字列が数字のみかを手動チェック）
  - `scheduler`が`fifo`か`edf`のどちらかか（`strcmp`で比較、`switch`禁止）
- [ ] 不正入力時はエラーメッセージを出して終了（メモリリークなしで）

### 完了の目安
```bash
./codexion 5 800 200 200 200 3 100 fifo
# → パースが通り、値が正しく構造体に格納されることを printf で確認できる
```

### Normチェックポイント
- 関数25行以内に収まっているか（パース処理は分割必須）
- 変数宣言は関数冒頭にまとめているか

---

## Phase 2：単一コーダーでの状態遷移ループ

**目的**：まだドングルの取り合いは実装せず、「1人のコーダーが正しくループを回せるか」だけを確認する。

### やること
- [ ] `pthread_create`でコーダー1人分のスレッドを起動
- [ ] `coder_routine`で `compiling → debugging → refactoring` をログ出力しながら`usleep`で時間経過
- [ ] `pthread_join`で正しく回収できるか確認
- [ ] ログフォーマットを仕様通りに揃える（`timestamp_in_ms X is compiling`など）

### 完了の目安
```
0 1 is compiling
201 1 is debugging
401 1 is refactoring
```
のような出力が、1人のコーダーで正しく得られる。

### Normチェックポイント
- `coder_routine`が25行を超えていないか（超えたら即分割）
- ループは`while(1)`（`for`禁止）

---

## Phase 3：ドングル資源とミューテックス導入（2〜3人）

**目的**：ここで初めて「奪い合い」を実装する。デッドロック回避の設計が最も重要になるフェーズ。

### やること
- [ ] 各ドングルに`pthread_mutex_t`を持たせる
- [ ] **仲介者方式（推奨）**：ドングル取得ロジックを1つの共有構造体経由で行う
  - `pthread_mutex_lock` → 両方空いているかチェック → 空いていなければ`pthread_cond_wait`
- [ ] `has taken a dongle`ログを2回出力（左右それぞれ取得時点で）
- [ ] `pthread_cond_broadcast`で解放時に全待機者へ通知

### 完了の目安
- 2〜3人のコーダーで、デッドロックせずに交互にコンパイルできる
- `valgrind --tool=helgrind`でデータ競合が出ない

### Normチェックポイント
- 条件変数の`while`ループ（`if`ではない）になっているか
- mutexのlock/unlockが必ず対になっているか（早期returnで抜け漏れがないか）

### この段階で意図的にテストすること
- [ ] 全員同じ順序でドングルを要求させてみて、デッドロックが**起きないこと**を確認する
- [ ] 大人数（10人以上）でしばらく動かし、固まらないか確認

---

## Phase 4：クールダウン機能

### やること
- [ ] ドングル解放時に`cooldown_until_ms = now + dongle_cooldown`を記録
- [ ] 取得条件に「空いている」だけでなく「クールダウンが終わっている」を追加
- [ ] `pthread_cond_timedwait`を使い、クールダウン終了時刻を目安に再チェックさせる（単純ポーリングにしない）

### 完了の目安
- `dongle_cooldown`を大きくすると、ドングルの再利用に確実に遅延が生まれることをログで確認できる

---

## Phase 5：FIFOスケジューラ（自作優先度付きキュー）

### やること
- [ ] 標準ライブラリ不使用の**二分ヒープ**を自作（`heap.c`）
  - `heap_push`, `heap_pop`, `heap_peek`
  - FIFOの場合、要求到着タイムスタンプをキーにする
- [ ] ドングル要求時にヒープへpush、取得可能になったら先頭から順に許可
- [ ] スケジューラ種別`fifo`の時のみこのロジックを使う分岐（`if`/`else`、`switch`禁止）

### 完了の目安
- 複数コーダーが同時にドングルを要求した際、要求順に許可されることをログで確認

### Normチェックポイント
- ヒープ操作の各関数が25行以内か（push/pop/percolate up/downを分ける）

---

## Phase 6：モニタースレッド＋バーンアウト検出（10ms精度）

**目的**：ここが最も難易度が高いフェーズ。時間制約の実装。

### やること
- [ ] 独立した`pthread_create`でモニタースレッドを起動
- [ ] モニターは短い間隔（例：数ms）で全コーダーの`last_compile_start_ms + time_to_burnout`をチェック
- [ ] 期限超過を検知したら`burned out`ログを出し、`simulation_over`フラグを立てて全スレッドに`broadcast`
- [ ] 全コーダーが`number_of_compiles_required`回達成した場合も同様にフラグを立てて終了

### 完了の目安
- `time_to_burnout`を極端に短く設定してバーンアウトを意図的に発生させ、実測との差が10ms以内であることを確認

### Normチェックポイント
- モニター用の待機ループは`while`（`for`禁止）
- ポーリング間隔もマジックナンバーにせず、意味のある定数名で管理（ただし`#define`はリテラル・定数値のみに使用）

---

## Phase 7：EDFスケジューラ＋タイブレーク

### やること
- [ ] ヒープのキーを「締め切り（`last_compile_start + time_to_burnout`）」に変更した版を実装
- [ ] 締め切りが完全に同一の場合のタイブレークルールを決めて実装（例：コーダーID昇順）
- [ ] `scheduler`引数が`edf`の時にこちらを使うよう分岐

### 完了の目安
- FIFOとEDFで同じパラメータを与えて挙動を比較し、EDFの方が締め切りの近い人を優先していることをログで確認

---

## Phase 8：終了処理（正常終了・全スレッドの安全な停止）

### やること
- [ ] 共有の`simulation_over`フラグをmutexで保護
- [ ] フラグが立ったら`pthread_cond_broadcast`で待機中の全スレッドを起こす
- [ ] 起きたスレッドは必ず「終了フラグが立っているか」を再チェックしてからループを抜ける
- [ ] `main`側で全コーダー＋モニターを`pthread_join`
- [ ] 全mutex/condを`destroy`、`malloc`した構造体を`free`

### 完了の目安
- 正常終了時、`valgrind --leak-check=full`でリークゼロ
- プログラムが確実にプロンプトへ戻ってくる（ハングしない）

---

## Phase 9：Norm適合・メモリリーク・データ競合の最終検証

### チェックリスト
- [ ] `norminette`を全ファイルに実行し、エラーゼロにする
- [ ] `-Wall -Wextra -Werror -pthread`でコンパイルが通る
- [ ] `valgrind --leak-check=full --show-leak-kinds=all`でリークゼロ
- [ ] `valgrind --tool=helgrind`（または`--tool=drd`）でデータ競合ゼロ
- [ ] 極端な引数（人数1人、時間が非常に短い/長い、不正な引数）でクラッシュしないか確認
- [ ] 大人数（20〜50人）・長時間稼働でも固まらないか確認

---

## Phase 10：README作成・提出準備

### README.mdに含める必須セクション（課題書 第VII章準拠）
- [ ] 最初の1行：*This project has been created as part of the 42 curriculum by \<login\>.*（イタリック）
- [ ] Description
- [ ] Instructions（ビルド方法・実行方法）
- [ ] Resources（参考文献＋AI利用箇所の明記）
- [ ] **Blocking cases handled**（デッドロック防止・スタベーション防止・クールダウン処理・正確なバーンアウト検出・ログのシリアライズ）
- [ ] **Thread synchronization mechanisms**（mutex/condの使い方、競合防止の具体例）

### 提出前の最終確認
- [ ] Gitリポジトリへの提出物一式（Makefile、`.c`/`.h`、README.md）
- [ ] ファイル名の再確認
- [ ] 相互評価に備え、コードの各部分を自分の言葉で説明できるか自己チェック

---

## 進め方の原則（繰り返し重要）

1. **常に「今動く最小構成」を保つ**：機能を足すたびに、必ずビルド＆動作確認してから次へ進む
2. **少人数→多人数の順でテスト**：1〜2人で正しく動いてから、10人以上でストレステストする
3. **バグの切り分けはPhase単位で**：Phase 3でデッドロックが出たら、Phase 5以降の複雑さは無視して原因を潰す
4. **Norm違反は後回しにしない**：後からまとめて直そうとすると関数分割のやり直しが大量発生するので、各Phaseの完了条件に「Norm的にも問題ないか」を含める

---

## 推定所要時間の目安（あくまで目安）

| Phase | 目安時間 |
|---|---|
| 0（設計） | 半日〜1日 |
| 1（骨組み） | 半日 |
| 2（単一コーダー） | 半日 |
| 3（ドングル・デッドロック回避） | 1〜2日（最重要かつ最難関） |
| 4（クールダウン） | 半日 |
| 5（FIFO） | 1日 |
| 6（モニター・10ms精度） | 1〜1.5日 |
| 7（EDF＋タイブレーク） | 1日 |
| 8（終了処理） | 1日（デバッグ含む） |
| 9（検証） | 1日 |
| 10（README） | 半日 |

**合計目安：8〜10日程度**（並行処理未経験の場合はもう少し余裕を見ることを推奨）

---

## メモ：設計書を作った後、どこから作るかの決め方

見る観点は次の3つ。

1. **依存関係**：他に頼らず動かして確かめられるものから作る
2. **リスク**：難しいところ、まだ分かっていないところほど早めに手を付ける
3. **確認できる単位**：どのステップも「動かしてOKと言える」状態で終える

### 1. 依存関係を見る：「何がないとそれを試せないか」

設計書3章の依存関係図を下から読む。

```
main ─┬─ parsing
      ├─ init
      ├─ coder_routine ─┬─ dongle ─┬─ scheduler ── heap
      │                 │          └─ dongle_cooldown
      │                 └─ log
      ├─ monitor
      └─ cleanup
```

- `heap` や `time_utils` は他に頼らないので、単体で作ってテストできる
- `dongle` は `scheduler` がないと完成しない
- ただし、下の部品を全部作り切ってから上に進む（横に積む）のは避ける。最後につなぐまで一度も動かないので、バグが出たときに原因が分からなくなる

### 2. 縦に細く通す（ウォーキングスケルトン）

まず「端から端まで一応動く、一番細い形」を作る。中身は空っぽでいい。

```
引数パース → 初期化 → スレッド1本 → ログ出力 → join → free
```

その後、動く状態を保ったまま肉付けを1つずつ足す（このロードマップの Phase 2 がまさにこれ）。

### 3. リスクの高いものを先にやる

| 部品 | 難しさ | 分からなさ | 判断 |
|---|---|---|---|
| 引数パース | 低 | 低 | 後回しでいい（仮の値で進められる） |
| デッドロック回避 | 高 | 高 | **早めに**。ここで設計がひっくり返ると全部やり直しになる |
| ヒープ | 中 | 低 | 単体テストできるので、気分転換に挟める |
| 10ms 精度 | 中 | 中 | 測ってみないと分からないので、早めに一度試す |

分からないところは、本番コードとは別に小さな実験プログラム（スパイク）で確かめてから設計に戻る（`test/test.c` の使い方）。

### 4. 各ステップの完了条件を先に決める

- 要件定義書の受け入れ基準（AT）をそのまま完了条件に使う
  - 例：Phase 3 の完了条件は「AT-05 がデッドロックせずに通る」「helgrind でエラー0」
- 完了条件を書けないステップは、大きすぎるかゴールがあいまい → 分割する

### 手順まとめ

1. 設計書の部品一覧と依存関係図を見る
2. 一番細い「端から端まで動く形」を最初のゴールにする
3. 残りの部品にリスクを付けて、高いものを前にする
4. 依存の都合で順番を微調整する（使う側より先に、使われる側を作る）
5. 各ステップに完了条件（AT や確認するコマンド）を付ける

### 練習

設計書12章の問題13件を、どの順番で直すか考える（どの観点で決めたかも添える）。