# 09: リソース解放順序とライフタイム管理

## TL;DR

C++ では**オブジェクトの破棄順序が言語仕様で保証**されている。ImGui + GLFW + OpenCV のような複数ライブラリを組み合わせるとき、この順序を意識しないとクラッシュやリソースリークが起きる。

## TypeScript との対比

TypeScript (Node.js) では GC がオブジェクトを回収するため、デストラクタの順序を気にすることは少ない。ただし「後に作ったものを先に閉じる」という依存関係は同様に存在する：

```typescript
// TS: 逆順クリーンアップを手動で書く
const db = await openDatabase();
const cache = new Cache(db);  // db に依存
const server = new Server(cache, db);  // cache と db に依存

// 閉じるときは逆順
await server.close();   // 先に server
await cache.close();    // 次に cache
await db.close();       // 最後に db
```

C++ では RAII + スコープで「逆順破棄」が自動化される：

```cpp
{
    Database db;           // 1番目に生成
    Cache cache{db};      // 2番目に生成（db に依存）
    Server server{cache}; // 3番目に生成（cache に依存）

    // スコープを出ると逆順に破棄される:
    // 1. server のデストラクタ
    // 2. cache のデストラクタ
    // 3. db のデストラクタ
}
// ← ここで全て解放済み
```

## Chapter 9 での解放順序

Chapter 9 では ImGui (OpenGL3) / GLFW / OpenCV の 3 つのサブシステムを使っている。

```
依存関係:
  ImGui (OpenGL3 backend) → OpenGL Context → GLFW Window → GLFW
  OpenCV imshow → 独立（GLFWとは別）
```

```cpp
// app/main.cpp の解放順序（コメントで明記）
// 1. ImGui バックエンドを先にシャットダウン
//    （OpenGL コンテキストがまだ有効な間に呼ぶ必要がある）
ImGui_ImplOpenGL3_Shutdown();
ImGui_ImplGlfw_Shutdown();
ImGui::DestroyContext();

// 2. GLFW を終了
//    （ImGui の後でないと OpenGL コールが UB になる）
glfwDestroyWindow(window);
glfwTerminate();

// 3. OpenCV ウィンドウを閉じる
//    （GLFW と独立しているのでいつでもよいが最後にまとめる）
cv::destroyAllWindows();
```

### 間違えるとどうなるか

```cpp
// NG: GLFW を先に終了すると ImGui のシャットダウンが UB
glfwTerminate();          // OpenGL コンテキスト消滅
ImGui_ImplOpenGL3_Shutdown();  // ← OpenGL API を呼ぶが UB! クラッシュ
```

## `std::jthread` のライフタイム

Chapter 6 と 9 では `std::jthread` でバックグラウンドスレッドを動かしている。`jthread` は RAII スレッドで、デストラクタが `request_stop()` + `join()` を自動で呼ぶ。

```cpp
class OscReceiver {
    std::jthread recv_thread_;  // メンバ変数として持つ
public:
    void start() {
        recv_thread_ = std::jthread([this](std::stop_token st) {
            recv_loop(std::move(st));
        });
    }
    void stop() {
        recv_thread_.request_stop();  // 停止フラグ
        recv_thread_.join();          // スレッド終了を待つ
    }
    ~OscReceiver() {
        // stop() を呼び忘れても jthread のデストラクタが
        // request_stop() + join() を自動で呼ぶ
    }
};
```

TS の比較：

```typescript
// TS: Worker には自動クリーンアップがない — 手動 terminate() が必要
class OscReceiver {
    private worker: Worker;
    close() { this.worker.terminate(); }
    // デストラクタがないので close() を呼び忘れると Worker がリーク
}
```

## `UiState` のスレッド安全なライフタイム

`UiState` は複数スレッド（OSC受信スレッド + GUIスレッド）から同時にアクセスされる。

```cpp
class UiState {
    mutable std::mutex mutex_;    // mutable = const メソッドでもロックできる
    std::atomic<float> fps_;      // アトミック: mutex 不要な単純な読み書き
    std::vector<std::string> logs_;  // mutex で保護
    // ...

    // on_change コールバックの実行中は mutex を解放しない
    // → デッドロックに注意（コールバック内で再び UiState を触るな）
    void notify_change() {
        if (on_change_) {
            on_change_();  // mutex ロック中に呼ぶ場合は設計に注意
        }
    }
};
```

## Observer パターンとライフタイム

Chapter 9 では `UiState::on_change()` でコールバックを登録する Observer パターンを使っている。

```cpp
// app/main.cpp
media::ColorTracker tracker{config.hsv};

// キャプチャ: UiState の変更を tracker に伝える
// 注意: このラムダは tracker と ui_state の参照を捕捉する
ui_state.on_change([&tracker, &ui_state]() {
    tracker.set_range(ui_state.get_hsv_range());
});
```

ライフタイムの罠：

```cpp
// NG: ローカル変数のポインタをコールバックに渡す → ダングリング参照
{
    media::ColorTracker temp_tracker{config.hsv};
    ui_state.on_change([&temp_tracker]() {  // temp_tracker がスコープを出たら...
        temp_tracker.set_range(...);          // ← ダングリング参照! UB
    });
}  // ← temp_tracker 破棄 → コールバックが呼ばれると UB
```

## まとめ

- C++ の RAII + スコープは**逆順破棄を自動化**する。TypeScript の手動クリーンアップより安全。
- 複数ライブラリを組み合わせるときは**依存関係の逆順**で解放する。
- `std::jthread` はデストラクタで自動的に `request_stop() + join()` を呼ぶ（TS の `Worker.terminate()` とは異なりリークしない）。
- Observer パターンのコールバックが捕捉する参照のライフタイムに注意。コールバックより長く生きるオブジェクトのみを参照すること。
