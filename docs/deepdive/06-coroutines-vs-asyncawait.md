# 06: C++20 コルーチン vs TypeScript async/await

## TL;DR

| 特徴 | TypeScript async/await | C++20 コルーチン |
|------|----------------------|----------------|
| スケジューラ | Node.js/ブラウザが自動管理 | **自分で用意する** |
| 切り替えコスト | イベントループ経由（μs） | 関数呼び出し並み（ns） |
| スタック | 別スタックは不要 | フレームをヒープに退避 |
| 主なユースケース | I/O 待ち、非同期API | ジェネレータ、パイプライン |

## TypeScript との対比

TypeScript では `async/await` を使うだけで非同期処理が書ける。

```typescript
// TS: 「どのスレッドで動かすか」を意識しない
async function processFrame(cap: VideoCapture): Promise<void> {
    const frame = await cap.readFrame();       // ここで制御を返す
    const result = await detect(frame);        // 検出が終わったら再開
    await sendOsc(result);                     // 送信が終わったら再開
}
```

C++20 のコルーチンは**同じキーワード**（`co_yield`, `co_return`, `co_await`）を持つが、根本的に異なる：

```cpp
// C++: コルーチンのフレームワーク(promise_type)を自分で書く必要がある
// または cppcoro などのライブラリを使う
Generator<cv::Mat> frame_producer(CameraCapture& cam) {
    while (true) {
        auto frame = cam.read_frame();
        if (!frame) co_return;
        co_yield *frame;          // 呼び出し元に制御を返す（サスペンド）
    }
}
```

## C++20 コルーチンの構造

### コルーチンの3要素

```
┌──────────────────────────────────────────────────┐
│ コルーチン関数 (coroutine function)                │
│   戻り値型が promise_type を持つ必要がある          │
│   co_yield / co_return / co_await を含む          │
└──────────────────────────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────────────┐
│ コルーチンフレーム (heap に確保)                   │
│   ローカル変数、再開ポイント、Promise を保持       │
└──────────────────────────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────────────┐
│ coroutine_handle<Promise>                         │
│   resume() / destroy() で操作                     │
└──────────────────────────────────────────────────┘
```

### 最小限の Generator の実装

```cpp
// Chapter 6 では使っていないが、C++20 コルーチンの "Hello World"
template<typename T>
struct Generator {
    struct promise_type {
        T current_value;

        Generator get_return_object() {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T value) noexcept {
            current_value = value;
            return {};           // caller に制御を戻す
        }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle_;

    // イテレータ経由でアクセス
    bool next() {
        handle_.resume();        // コルーチンを再開
        return !handle_.done();
    }
    T& value() { return handle_.promise().current_value; }
    ~Generator() { if (handle_) handle_.destroy(); }
};
```

## Chapter 6 での実装アプローチ

Chapter 6 では Generator より **スレッドベースのパイプライン** を採用した。理由：

| アプローチ | メリット | デメリット |
|----------|---------|----------|
| `std::jthread` + `SpscQueue` | シンプル、デバッグしやすい | OSスレッドのオーバーヘッド |
| コルーチン (Generator) | コンテキストスイッチが軽量 | `promise_type` 実装が複雑 |
| 非同期タスクライブラリ (cppcoro) | 高水準API | サードパーティ依存 |

```cpp
// Chapter 6 の実装: jthread + SpscQueue
// 各ステージを独立スレッドで動かす
class Pipeline {
    SpscQueue<cv::Mat, 8> capture_queue_;     // カメラ → 検出
    SpscQueue<DetectorOutput, 8> detect_queue_; // 検出 → OSC送信

    std::jthread capture_thread_;
    std::jthread detect_thread_;
    std::jthread sender_thread_;
    // ...
};
```

## `std::jthread` + `std::stop_token` の仕組み

```cpp
// jthread: RAII スレッド (デストラクタで自動 join)
// TS の "AbortController + AbortSignal" に相当
std::jthread worker([](std::stop_token stop_token) {
    while (!stop_token.stop_requested()) {  // 停止要求が来るまでループ
        // 処理...
    }
    // stop が要求されたら自然に関数を終了
    // デストラクタが join() を呼ぶ
});

// 外から停止を要求
worker.request_stop();   // stop_token.stop_requested() が true になる
// worker がスコープを出るとデストラクタが join() を呼ぶ
```

TypeScript の `AbortController` との対比：

```typescript
// TS の AbortController
const controller = new AbortController();
const signal = controller.signal;

async function worker(signal: AbortSignal) {
    while (!signal.aborted) {
        await doWork();
    }
}

controller.abort();  // signal.aborted が true になる
```

## メモリ順序と `std::atomic`

`SpscQueue` が使っている `std::memory_order`：

```cpp
// SpscQueue::try_push の核心
auto head = head_.load(std::memory_order_relaxed);   // 自分のカウンタ読み
auto next_head = (head + 1) % N;

if (next_head == tail_.load(std::memory_order_acquire)) {
    return false;  // 満杯
}

buffer_[head] = value;
head_.store(next_head, std::memory_order_release);    // tail スレッドに可視化
```

| memory_order | 意味 |
|-------------|------|
| `relaxed` | 順序保証なし（自分のカウンタ読みは他スレッドと関係ない） |
| `acquire` | これ以降の読み書きをこの操作より前に移動させない |
| `release` | これ以前の読み書きをこの操作より後に移動させない |

`acquire` + `release` のペアで「`release` 前の書き込みが `acquire` 後に可視になる」ことが保証される。

## まとめ

- C++ コルーチンは **スケジューラを自前で実装する** 必要があり、TS の `async/await` より低レベル。
- コンテキストスイッチが軽量（関数呼び出し並み）なので、大量の非同期タスクに向く。
- Chapter 6 では学習コストを下げるため `jthread` + `SpscQueue` を選んだ。
- `stop_token` = TS の `AbortSignal`、`jthread` = デストラクタで自動 `join` する スレッド。
