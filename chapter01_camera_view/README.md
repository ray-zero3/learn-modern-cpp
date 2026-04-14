# Chapter 1 — カメラ映像をウィンドウに表示

## TL;DR

この章のゴールは **内蔵 Webcam の映像を 30fps でウィンドウに表示する** こと。
OpenCV の `cv::VideoCapture` を **RAII ラッパクラス** で包み、リソース管理を安全にします。

---

## この章で学ぶこと

| C++ 概念 | 説明 |
|---|---|
| **RAII** | コンストラクタでリソース取得、デストラクタで解放。`finally` 不要の理由 |
| **コピー禁止** (`= delete`) | 物理デバイスを2つのオブジェクトが「所有」する問題を型で防ぐ |
| **ムーブセマンティクス** (`= default`) | 所有権の移転。コピーせず「持ち主を変える」 |
| **`std::optional<T>`** | 「値があるかもしれない型」。nullptr チェックを型安全に書く |
| **`[[nodiscard]]`** | 戻り値を無視したらコンパイル警告。TS の lint ルールに相当 |
| **`find_package(OpenCV)`** | システムにインストール済みのライブラリを CMake で参照 |

---

## TypeScript との対比

| | TypeScript | Chapter 1 の C++ |
|---|---|---|
| リソース解放 | `try-finally` / `Symbol.dispose` | デストラクタ（自動、例外があっても確実） |
| 「値がないかも」 | `T \| null \| undefined` | `std::optional<T>` |
| 「使えない関数」の型表現 | TypeScript にはない | `= delete` でコンパイルエラー |
| 外部ライブラリ参照 | `npm install + import` | `find_package + target_link_libraries` |

---

## ディレクトリ構成

```
chapter01_camera_view/
├── CMakeLists.txt
├── README.md
├── reference/              # お手本（完成コード）
│   ├── CMakeLists.txt
│   ├── include/media/
│   │   └── camera_capture.hpp   # CameraCapture クラスの宣言
│   ├── src/
│   │   └── camera_capture.cpp   # CameraCapture の実装
│   ├── app/
│   │   └── main.cpp             # カメラ表示アプリ
│   └── tests/
│       └── test_camera_capture.cpp  # doctest
└── workspace/              # 作業場所（前章 reference をコピーして開始）
```

---

## Step 1: ビルドと実行

```bash
# ルートから全体ビルド
cmake -S . -B build \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# テスト実行
ctest --test-dir build --output-on-failure

# カメラアプリ起動（macOS のカメラ許可が必要）
./build/chapter01_camera_view/reference/chapter01_ref_app
```

macOS では初回起動時にカメラ使用許可ダイアログが出ます。
システム設定 → プライバシーとセキュリティ → カメラ で Terminal.app に許可が必要です。

---

## Step 2: CameraCapture クラスを理解する

### なぜ RAII ラッパが必要か

`cv::VideoCapture` を直接使った場合:

```cpp
cv::VideoCapture cap;
cap.open(0);
// ... 何か処理 ...
// 例外が飛んだら release() が呼ばれない！
cap.release();
```

`CameraCapture` を使った場合:

```cpp
{
    media::CameraCapture cam;
    cam.open(0);
    // ... 何か処理 ...
    // 例外が飛んでも、スコープを出ると ~CameraCapture() が呼ばれる
} // ← ここで cap_.release() が自動実行
```

### `std::optional` の使い方

```cpp
auto frame = cam.read_frame(); // std::optional<cv::Mat>

// 値があるか確認してからアクセス
if (frame.has_value()) {
    cv::imshow("window", *frame); // * で中の値を取り出す
}

// TS の optional chaining 的な書き方
if (frame) {
    cv::imshow("window", *frame);
}
```

---

## Step 3: workspace で実装する

`workspace/` に以下を作成してください:

1. `CMakeLists.txt` (reference 参照)
2. `include/media/camera_capture.hpp` — クラス宣言
3. `src/camera_capture.cpp` — 実装
4. `tests/test_camera_capture.cpp` — テスト（先に書く！TDD）
5. `app/main.cpp` — カメラ表示アプリ

**TDD の流れ**:
1. テストを書く → ビルドエラー（RED）
2. ヘッダと空実装を作る → テスト失敗（RED）
3. 実装を埋める → テスト PASS（GREEN）

---

## Step 4: 深掘り

- [docs/deepdive/01-raii.md](../docs/deepdive/01-raii.md) — なぜ `finally` が不要か、5 つの特殊メンバ関数、`= delete` / `= default`

---

## 達成チェックリスト

- [ ] `ctest --test-dir build --output-on-failure` で Chapter 1 reference の全テスト PASS
- [ ] RAII が「コンストラクタで取得、デストラクタで解放」だと説明できる
- [ ] `= delete` で何が防げるか言える（コピー禁止の理由）
- [ ] `= default` なムーブコンストラクタの動作を説明できる
- [ ] `std::optional` が `T | null` より型安全な理由を言える
- [ ] `[[nodiscard]]` を外すと何が困るか説明できる
- [ ] `find_package` と `FetchContent` の使い分けを説明できる

全部埋まったら → Chapter 2（HSV色域トラッカー）へ
