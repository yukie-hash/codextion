#!/usr/bin/env python3
"""
Codexion テストランナー（提出物ではない。Norm 対象外）

使い方:
    python3 tests/test_codexion.py              # 全テスト（各ケース3回ずつ）
    python3 tests/test_codexion.py --quick      # 各ケース1回だけ（速い）
    python3 tests/test_codexion.py -k burn      # 名前に "burn" を含むテストだけ
    python3 tests/test_codexion.py -n 10        # 各ケース10回（タイミング系の不安定さを見る）
    python3 tests/test_codexion.py --leaks      # macOS の leaks でメモリリークも確認
    python3 tests/test_codexion.py --list       # テスト一覧を表示するだけ

前提:
    - リポジトリ直下で make 済み（./codexion がある）こと。--bin で別のバイナリも指定可
    - 失敗したケースのログは tests/logs/ に保存される

各テストの ID（AT-xx など）は 設計/01_要件定義書.md の受け入れ基準に対応している。
"""

import argparse
import os
import re
import subprocess
import sys
import time

# ---------------------------------------------------------------------------
# 設定
# ---------------------------------------------------------------------------

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG_DIR = os.path.join(ROOT, "tests", "logs")

BURNOUT_TOLERANCE_MS = 10   # 課題: burned out は実際のバーンアウトから 10ms 以内
COOLDOWN_TOLERANCE_MS = 2   # ログ時刻は解放の直後に取るので、数 ms の誤差を許す
EXIT_WARN_MS = 300          # 最後のログからプロセス終了までがこれより長いと WARN

LOG_RE = re.compile(
    r"^(\d+) (\d+) (has taken a dongle|is compiling|is debugging|"
    r"is refactoring|burned out)$"
)

# 端末の色（パイプに流したときは色なし）
if sys.stdout.isatty():
    GREEN, RED, YELLOW, GRAY, BOLD, RESET = (
        "\033[32m", "\033[31m", "\033[33m", "\033[90m", "\033[1m", "\033[0m")
else:
    GREEN = RED = YELLOW = GRAY = BOLD = RESET = ""


# ---------------------------------------------------------------------------
# 実行
# ---------------------------------------------------------------------------

class Result:
    """1回の実行結果"""

    def __init__(self, args, stdout, stderr, code, elapsed_ms, timed_out):
        self.args = args
        self.stdout = stdout
        self.stderr = stderr
        self.code = code
        self.elapsed_ms = elapsed_ms
        self.timed_out = timed_out


def run(binary, args, timeout_s):
    """codexion を実行する。timeout_s 秒で終わらなければ kill してハング扱い"""
    start = time.monotonic()
    try:
        p = subprocess.run([binary] + args, capture_output=True, text=True,
                           timeout=timeout_s)
        elapsed = (time.monotonic() - start) * 1000
        return Result(args, p.stdout, p.stderr, p.returncode, elapsed, False)
    except subprocess.TimeoutExpired as e:
        elapsed = (time.monotonic() - start) * 1000
        out = e.stdout.decode() if isinstance(e.stdout, bytes) else (e.stdout or "")
        err = e.stderr.decode() if isinstance(e.stderr, bytes) else (e.stderr or "")
        return Result(args, out, err, None, elapsed, True)


# ---------------------------------------------------------------------------
# ログ解析
# ---------------------------------------------------------------------------

class Params:
    """引数を名前付きで扱うための入れ物"""

    def __init__(self, args):
        (self.n, self.burnout, self.compile, self.debug, self.refactor,
         self.required, self.cooldown) = map(int, args[:7])
        self.scheduler = args[7]

    def dongles_of(self, coder):
        """コーダー番号(1始まり) → (左ドングル, 右ドングル)。設計書 4.2 と同じ番号付け"""
        return (coder - 1, coder % self.n)


def parse_log(stdout):
    """
    ログを [(ts, coder, msg), ...] にする。
    形式が崩れた行があれば errors に入れる（行の混ざりもここで見つかる）
    """
    events, errors = [], []
    for i, line in enumerate(stdout.splitlines(), 1):
        m = LOG_RE.match(line)
        if not m:
            errors.append(f"{i}行目の形式が不正: {line!r}")
            continue
        events.append((int(m.group(1)), int(m.group(2)), m.group(3)))
    return events, errors


def compile_starts(events, coder):
    return [ts for ts, c, msg in events if c == coder and msg == "is compiling"]


# ---------------------------------------------------------------------------
# チェック関数（問題があればメッセージのリストを返す。空なら OK）
# ---------------------------------------------------------------------------

def check_format(p, events, errors):
    """FR-11: 形式・コーダー番号の範囲・タイムスタンプの非減少"""
    problems = list(errors)
    prev = 0
    for ts, c, msg in events:
        if not 1 <= c <= p.n:
            problems.append(f"存在しないコーダー番号: {ts} {c} {msg}")
        if ts < prev:
            problems.append(f"タイムスタンプが逆行: {prev} → {ts}（{c} {msg}）")
        prev = ts
    return problems


def check_after_burnout(p, events):
    """FR-12: burned out は最大1回、その後にログが出ない"""
    problems = []
    burns = [i for i, e in enumerate(events) if e[2] == "burned out"]
    if len(burns) > 1:
        problems.append(f"burned out が {len(burns)} 回出ている")
    if burns and burns[0] != len(events) - 1:
        after = events[burns[0] + 1:]
        problems.append(f"burned out の後にログが {len(after)} 行ある（最初: {after[0]}）")
    return problems


def check_state_order(p, events):
    """
    FR-05/FR-11: 各コーダーの状態が
      taken×2 → compiling → debugging → refactoring → taken×2 → ...
    の順になっているか。1人モードは taken×1 のみ
    """
    problems = []
    expected_next = {}   # coder -> 次に来てよいメッセージ
    taken = {}           # coder -> 直近の compiling 以降に取ったドングル数
    for ts, c, msg in events:
        if msg == "burned out":
            continue
        if msg == "has taken a dongle":
            taken[c] = taken.get(c, 0) + 1
            if taken[c] > 2:
                problems.append(f"{ts} {c}: compiling の前に taken が3回以上")
            continue
        exp = expected_next.get(c, "is compiling")
        if msg != exp:
            problems.append(f"{ts} {c}: '{msg}' が来たが、期待は '{exp}'")
        if msg == "is compiling" and taken.get(c, 0) != 2:
            problems.append(f"{ts} {c}: compiling の直前の taken が {taken.get(c, 0)} 回（2回のはず）")
        if msg == "is compiling":
            taken[c] = 0
        expected_next[c] = {"is compiling": "is debugging",
                            "is debugging": "is refactoring",
                            "is refactoring": "is compiling"}[msg]
        if len(problems) > 5:
            break
    return problems


def check_exclusive(p, events):
    """
    FR-06: 隣同士（＝ドングルを共有する2人）が同時にコンパイルしていないか。
    コンパイル区間 = [is compiling の時刻, 解放時刻)
    解放時刻は min(次の is debugging の時刻, compiling + time_to_compile) とする。
    is debugging のログは解放の「後」に log_mutex を待ってから出るので、
    人数が多いと数 ms 遅れる。それを解放時刻とみなすと誤検出になるため、
    「どんなに早くても compile 時間ぶん寝た後には解放している」という下限を使う
    """
    if p.n < 2:
        return []
    end_ts = float("inf")   # まだ debugging が来ていない＝ドングルを握ったまま
    intervals = {c: [] for c in range(1, p.n + 1)}
    open_start = {}
    for ts, c, msg in events:
        if msg == "is compiling":
            open_start[c] = ts
        elif msg == "is debugging" and c in open_start:
            start = open_start.pop(c)
            intervals[c].append((start, min(ts, start + p.compile)))
    for c, s in open_start.items():
        intervals[c].append((s, end_ts))

    problems = []
    for a in range(1, p.n + 1):
        b = a % p.n + 1          # 右隣
        if a == b:
            continue
        for s1, e1 in intervals[a]:
            for s2, e2 in intervals[b]:
                if s1 < e2 and s2 < e1:
                    problems.append(f"隣同士 {a} と {b} が同時にコンパイル: "
                                    f"[{s1},{e1}) と [{s2},{e2})")
                    return problems
    return problems


def check_cooldown(p, events):
    """
    FR-07: ドングルが解放されてから dongle_cooldown ms 経つ前に、
    そのドングルを使う誰か（両隣）がコンパイルを始めていないか。
    解放時刻は check_exclusive と同じく min(is debugging, compiling + time_to_compile)
    """
    if p.cooldown == 0 or p.n < 2:
        return []
    problems = []
    release = {}   # dongle -> 最後に解放された時刻
    last_start = {}  # coder -> 直近の is compiling の時刻
    for ts, c, msg in events:
        left, right = p.dongles_of(c)
        if msg == "is compiling":
            last_start[c] = ts
            for d in (left, right):
                if d in release and ts < release[d] + p.cooldown - COOLDOWN_TOLERANCE_MS:
                    problems.append(f"{ts} {c}: ドングル{d} は {release[d]} に解放され "
                                    f"{release[d] + p.cooldown} までクールダウン中のはず")
        elif msg == "is debugging":
            freed = min(ts, last_start.get(c, ts) + p.compile)
            release[left] = freed
            release[right] = freed
        if len(problems) > 3:
            break
    return problems


def check_burnout_timing(p, events):
    """
    NFR-01 / D-07: burned out の時刻が「最後のコンパイル開始 + time_to_burnout」から
    0〜10ms 以内か。早すぎる（まだ燃え尽きていない）のも NG
    """
    burns = [e for e in events if e[2] == "burned out"]
    if not burns:
        return []
    ts, c, _ = burns[0]
    starts = compile_starts(events, c)
    last = starts[-1] if starts else 0
    deadline = last + p.burnout
    diff = ts - deadline
    if diff < -1:
        return [f"{c} が早すぎる burned out: 期限 {deadline} より {-diff}ms 前（{ts}）"]
    if diff > BURNOUT_TOLERANCE_MS:
        return [f"{c} の burned out が遅い: 期限 {deadline} から {diff}ms 後（{ts}）"]
    return []


def check_missed_burnout(p, events):
    """
    FR-10: 期限を 10ms 以上過ぎたのに burned out が出ていないコーダーがいないか
    （モニターの見逃し）。規定回数を達成したコーダーは対象外（D-06）
    """
    burn_ts = next((ts for ts, c, m in events if m == "burned out"), None)
    end = burn_ts if burn_ts is not None else (events[-1][0] if events else 0)
    problems = []
    for c in range(1, p.n + 1):
        starts = compile_starts(events, c)
        checkpoints = [0] + starts[:p.required]   # 規定回数達成後は見ない
        for prev, nxt in zip(checkpoints, checkpoints[1:] + [None]):
            if nxt is None:
                if len(starts) >= p.required:
                    break   # 達成済み
                nxt = end
            # 期限 + 10ms を過ぎた時点で、まだ誰の burned out も出ていなければ見逃し
            overdue = prev + p.burnout + BURNOUT_TOLERANCE_MS
            if nxt > overdue and (burn_ts is None or burn_ts > overdue):
                problems.append(f"{c}: {prev} の次のコンパイル開始が {nxt}"
                                f"（{nxt - prev}ms 空いた）のに burned out が出ていない")
                break
    return problems


def survived(p, events):
    """全員が規定回数以上コンパイルし、誰も燃え尽きていないか"""
    if any(m == "burned out" for _, _, m in events):
        burn = next(e for e in events if e[2] == "burned out")
        return [f"バーンアウトした: {burn[0]} {burn[1]} burned out"]
    short = [c for c in range(1, p.n + 1)
             if len(compile_starts(events, c)) < p.required]
    if short:
        return [f"規定回数 {p.required} 回に届かないまま終了したコーダー: {short}"]
    return []


def common_checks(p, events, errors):
    """どのシミュレーションでも必ず成り立つべきこと"""
    return (check_format(p, events, errors)
            + check_after_burnout(p, events)
            + check_state_order(p, events)
            + check_exclusive(p, events)
            + check_cooldown(p, events)
            + check_burnout_timing(p, events)
            + check_missed_burnout(p, events))


def exit_speed_warning(events, res):
    """U-01: 最後のログからプロセス終了までが長すぎないか（FAIL ではなく WARN）"""
    if not events:
        return None
    gap = res.elapsed_ms - events[-1][0]
    if gap > EXIT_WARN_MS:
        return f"最後のログから終了まで約 {gap:.0f}ms かかった（停止の反応が遅い？）"
    return None


# ---------------------------------------------------------------------------
# テストケース定義
#   kind:
#     "error"   … 引数エラーになるべき
#     "survive" … 誰も燃え尽きず、全員規定回数で止まるべき
#     "burn"    … 誰かが燃え尽きるべき（who を指定するとそのコーダー、at で目安時刻）
#     "lone"    … コーダー1人
#     "finish"  … 規定回数 0 などで即終了すべき
# ---------------------------------------------------------------------------

TESTS = [
    # --- 引数エラー（AT-01〜03, FR-02） ---------------------------------
    dict(id="AT-01", name="args: 引数なし", kind="error", args=[]),
    dict(id="AT-01", name="args: 7個", kind="error", args="4 800 200 200 200 5 0".split()),
    dict(id="AT-01", name="args: 9個", kind="error", args="4 800 200 200 200 5 0 fifo x".split()),
    dict(id="AT-02", name="args: 負の数", kind="error", args="-5 800 200 200 200 5 0 fifo".split()),
    dict(id="AT-02", name="args: 英字", kind="error", args="abc 800 200 200 200 5 0 fifo".split()),
    dict(id="AT-02", name="args: 数字+英字", kind="error", args="4 800a 200 200 200 5 0 fifo".split()),
    dict(id="AT-02", name="args: 空文字", kind="error", args=["", "800", "200", "200", "200", "5", "0", "fifo"]),
    dict(id="AT-02", name="args: 小数", kind="error", args="4 800 3.5 200 200 5 0 fifo".split()),
    dict(id="AT-02", name="args: +符号", kind="error", args="+4 800 200 200 200 5 0 fifo".split()),
    dict(id="AT-02", name="args: INT_MAX超え", kind="error", args="4 2147483648 200 200 200 5 0 fifo".split()),
    dict(id="AT-02", name="args: 巨大な数", kind="error", args="4 99999999999999999999 200 200 200 5 0 fifo".split()),
    dict(id="AT-02", name="args: cooldown が負", kind="error", args="4 800 200 200 200 5 -1 fifo".split()),
    dict(id="AT-03", name="args: scheduler 大文字", kind="error", args="4 800 200 200 200 5 0 FIFO".split()),
    dict(id="AT-03", name="args: scheduler 不正", kind="error", args="4 800 200 200 200 5 0 rr".split()),
    dict(id="AT-03", name="args: scheduler 末尾空白", kind="error", args=["4", "800", "200", "200", "200", "5", "0", "edf "]),
    dict(id="D-01", name="args: コーダー0人", kind="error", args="0 800 200 200 200 5 0 fifo".split()),

    # --- 1人（AT-04, D-05） ---------------------------------------------
    dict(id="AT-04", name="lone: 1人はコンパイルできず燃え尽きる", kind="lone",
         args="1 800 200 200 200 5 0 {s}".split()),

    # --- 規定回数0（D-02） ----------------------------------------------
    dict(id="D-02", name="finish: compiles_required=0 で即終了", kind="finish",
         args="4 800 200 200 200 0 0 {s}".split()),

    # --- 生き残るべきケース（AT-05, AT-06, FR-09） -----------------------
    dict(id="AT-05", name="survive: 5人 余裕あり", kind="survive",
         args="5 800 200 200 200 7 0 {s}".split()),
    dict(id="AT-05", name="survive: 2人", kind="survive",
         args="2 410 200 100 100 5 0 {s}".split()),
    dict(id="AT-06", name="survive: 4人 ギリギリ(余裕10ms)", kind="survive",
         args="4 410 200 100 100 10 0 {s}".split()),
    dict(id="AT-06", name="survive: 3人 直列でギリギリ", kind="survive",
         args="3 610 200 100 100 5 0 {s}".split()),
    dict(id="AT-06", name="survive: 5人 奇数でギリギリ", kind="survive",
         args="5 610 200 200 200 5 0 {s}".split()),
    dict(id="AT-05", name="survive: 10人", kind="survive",
         args="10 800 200 200 200 5 0 {s}".split()),
    dict(id="AT-05", name="survive: 極小時間 (1ms)", kind="survive",
         args="3 50 1 1 1 3 1 {s}".split()),
    dict(id="AT-05", name="survive: 時間0", kind="survive",
         args="4 100 0 0 0 20 0 {s}".split()),

    # --- クールダウン（AT-08, FR-07） ------------------------------------
    dict(id="AT-08", name="cooldown: 500ms のクールダウンが守られる", kind="survive",
         args="3 2000 100 50 50 4 500 {s}".split()),
    dict(id="AT-08", name="cooldown: 4人 + 50ms", kind="survive",
         args="4 600 100 100 100 5 50 {s}".split()),
    dict(id="AT-08", name="burn: クールダウンのせいで間に合わない", kind="burn",
         args="2 300 100 50 50 5 300 {s}".split()),

    # --- 燃え尽きるべきケース（AT-07, NFR-01） ---------------------------
    dict(id="AT-07", name="burn: 4人 周期400 > 310", kind="burn",
         args="4 310 200 100 100 5 0 {s}".split()),
    dict(id="AT-07", name="burn: compile が burnout より長い", kind="burn",
         args="2 100 200 50 50 5 0 {s}".split()),
    dict(id="AT-07", name="burn: 5人 300ms", kind="burn",
         args="5 300 200 200 200 5 0 {s}".split()),
    dict(id="AT-07", name="burn: burnout 0", kind="burn",
         args="3 0 100 100 100 5 0 {s}".split()),

    # --- 負荷（AT-09, NFR-05） -------------------------------------------
    dict(id="AT-09", name="stress: 200人", kind="survive", timeout=40,
         args="200 3000 100 100 100 3 0 {s}".split()),
    dict(id="AT-09", name="stress: 50人 長め", kind="survive", timeout=60,
         args="50 1000 100 100 100 30 0 {s}".split()),
]

SCHEDULERS = ["fifo", "edf"]


def expand(tests):
    """{s} を fifo / edf に展開する"""
    out = []
    for t in tests:
        if "{s}" in t["args"]:
            for s in SCHEDULERS:
                t2 = dict(t)
                t2["args"] = [a.replace("{s}", s) for a in t["args"]]
                t2["name"] = f"{t['name']} [{s}]"
                out.append(t2)
        else:
            out.append(t)
    return out


def default_timeout(args):
    """終わるはずの時間から、ハング判定の秒数をざっくり決める"""
    try:
        p = Params(args)
    except (ValueError, IndexError):
        return 3
    cycle = p.compile + p.debug + p.refactor + p.cooldown
    estimate_ms = (p.required + 2) * max(cycle, 1) * max(p.n, 2) / 2 + p.burnout
    return min(60, max(5, estimate_ms / 1000 * 3))


# ---------------------------------------------------------------------------
# 判定
# ---------------------------------------------------------------------------

def judge(test, res):
    """(問題のリスト, 警告のリスト) を返す"""
    kind = test["kind"]
    if res.timed_out and kind == "error":
        return ["エラーにならず、シミュレーションが始まってしまった（3秒で打ち切り）"], []
    if res.timed_out:
        return [f"ハング: {res.elapsed_ms / 1000:.1f}秒で終わらなかった"], []
    if res.code is not None and res.code < 0:
        return [f"シグナルで異常終了（{-res.code}）。セグフォ等の可能性"], []

    if kind == "error":
        problems = []
        if res.code == 0:
            problems.append("終了コードが 0（エラーなら非0のはず）")
        if res.stdout.strip():
            problems.append(f"エラーなのに標準出力にログが出た: {res.stdout.splitlines()[0]!r}")
        if not res.stderr.strip():
            problems.append("標準エラー出力にメッセージがない")
        return problems, []

    p = Params(res.args)
    events, errors = parse_log(res.stdout)
    problems = common_checks(p, events, errors)
    warns = []
    w = exit_speed_warning(events, res)
    if w:
        warns.append(w)
    if res.code != 0:
        problems.append(f"終了コードが {res.code}（正常停止なら 0 のはず）")

    if kind == "survive":
        problems += survived(p, events)
    elif kind == "burn":
        if not any(m == "burned out" for _, _, m in events):
            problems.append("burned out が出ていない（燃え尽きるはずのパラメータ）")
    elif kind == "lone":
        taken = [e for e in events if e[2] == "has taken a dongle"]
        if len(taken) != 1:
            problems.append(f"1人なのに taken が {len(taken)} 回（1回のはず）")
        if any(m == "is compiling" for _, _, m in events):
            problems.append("ドングル1つでコンパイルしている")
        if not events or events[-1][2] != "burned out":
            problems.append("最後が burned out になっていない")
    elif kind == "finish":
        if any(m == "burned out" for _, _, m in events):
            problems.append("即終了のはずが burned out が出た")
        if res.elapsed_ms > 1000:
            warns.append(f"即終了のはずが {res.elapsed_ms:.0f}ms かかった")
    return problems, warns


def save_log(test, res, attempt):
    os.makedirs(LOG_DIR, exist_ok=True)
    safe = re.sub(r"[^\w\-]+", "_", f"{test['id']}_{test['name']}")[:80]
    path = os.path.join(LOG_DIR, f"{safe}_{attempt}.log")
    with open(path, "w") as f:
        f.write(f"$ ./codexion {' '.join(res.args)}\n")
        f.write(f"# exit={res.code} elapsed={res.elapsed_ms:.0f}ms timed_out={res.timed_out}\n")
        f.write("# ---- stdout ----\n" + res.stdout)
        f.write("\n# ---- stderr ----\n" + res.stderr)
    return os.path.relpath(path, ROOT)


def check_leaks(binary, args, timeout_s):
    """macOS の leaks --atExit で確認。0=リークなし, 1=リークあり"""
    cmd = ["leaks", "--atExit", "--", binary] + args
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout_s)
    except subprocess.TimeoutExpired:
        return None, "leaks の実行がタイムアウト"
    except FileNotFoundError:
        return None, "leaks コマンドがない"
    m = re.search(r"(\d+) leaks? for (\d+) total leaked bytes", p.stdout)
    summary = m.group(0) if m else "結果を読み取れなかった"
    return p.returncode == 0, summary


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description="Codexion テストランナー")
    ap.add_argument("--bin", default=os.path.join(ROOT, "codexion"), help="テストするバイナリ")
    ap.add_argument("-n", "--repeat", type=int, default=3, help="シミュレーション系を何回ずつ回すか")
    ap.add_argument("--quick", action="store_true", help="各ケース1回だけ")
    ap.add_argument("-k", "--filter", default="", help="名前か ID にこの文字列を含むテストだけ")
    ap.add_argument("--leaks", action="store_true", help="leaks でメモリリークも確認する")
    ap.add_argument("--list", action="store_true", help="テスト一覧を表示して終了")
    opt = ap.parse_args()

    tests = [t for t in expand(TESTS)
             if opt.filter.lower() in (t["name"] + t["id"]).lower()]
    if opt.list:
        for t in tests:
            print(f"{t['id']:6} {t['kind']:8} {t['name']:45} {' '.join(t['args'])}")
        return 0
    if not os.path.exists(opt.bin):
        print(f"{RED}バイナリがない: {opt.bin}（先に make してください）{RESET}")
        return 2

    repeat = 1 if opt.quick else opt.repeat
    passed = failed = warned = 0
    failures = []

    for t in tests:
        n = 1 if t["kind"] == "error" else repeat
        if t["kind"] == "error":
            timeout_s = 3   # エラーなら即終了するはず。通ってしまうと走り続けるので短く切る
        else:
            timeout_s = t.get("timeout") or default_timeout(t["args"])
        problems_all, warns_all, logs = [], [], []
        for attempt in range(1, n + 1):
            res = run(opt.bin, t["args"], timeout_s)
            problems, warns = judge(t, res)
            if problems:
                problems_all.append((attempt, problems))
                logs.append(save_log(t, res, attempt))
            warns_all += warns

        label = f"{t['id']:6} {t['name']}"
        if problems_all:
            failed += 1
            print(f"{RED}FAIL{RESET} {label}  {GRAY}({len(problems_all)}/{n} 回失敗){RESET}")
            attempt, problems = problems_all[0]
            for msg in problems[:5]:
                print(f"       - {msg}")
            print(f"       {GRAY}再現: ./codexion {' '.join(repr(a) if a == '' or ' ' in a else a for a in t['args'])}{RESET}")
            print(f"       {GRAY}ログ: {logs[0]}{RESET}")
            failures.append(label)
        else:
            passed += 1
            mark = f"{YELLOW}PASS{RESET}" if warns_all else f"{GREEN}PASS{RESET}"
            print(f"{mark} {label}  {GRAY}({n}回){RESET}")
        if warns_all:
            warned += 1
            print(f"       {YELLOW}WARN: {warns_all[0]}{RESET}")

    if opt.leaks:
        print(f"\n{BOLD}--- メモリリーク（leaks --atExit） ---{RESET}")
        for args in (["4", "800", "200", "200", "200", "3", "0", "fifo"],   # 正常終了
                     ["4", "310", "200", "100", "100", "5", "0", "edf"],    # バーンアウト
                     ["1", "300", "100", "100", "100", "5", "0", "fifo"],   # 1人
                     ["abc", "800", "200", "200", "200", "5", "0", "fifo"]):  # 引数エラー
            ok, summary = check_leaks(opt.bin, args, 30)
            mark = f"{GREEN}OK  {RESET}" if ok else (f"{RED}LEAK{RESET}" if ok is False else f"{YELLOW}??? {RESET}")
            print(f"{mark} ./codexion {' '.join(args):40} {GRAY}{summary}{RESET}")

    print(f"\n{BOLD}結果: {GREEN}{passed} passed{RESET}{BOLD}, "
          f"{RED if failed else ''}{failed} failed{RESET}{BOLD}, "
          f"{YELLOW if warned else ''}{warned} warned{RESET}")
    if failures:
        print(f"{GRAY}失敗したテスト:{RESET}")
        for f in failures:
            print(f"  - {f}")
    print(f"{GRAY}※ helgrind（データ競合）は macOS では使えないので、Linux / Docker で別途確認すること{RESET}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
