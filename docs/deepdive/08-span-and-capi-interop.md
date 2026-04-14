# 08: std::span と C API の相互運用

## TL;DR

`std::span<T>` は「メモリの所有権を持たない、連続したデータへのビュー」。TypeScript の `ArrayBufferView` や `Uint8Array` に相当する。ONNX Runtime などの C API を安全に扱うためのブリッジとして活用できる。

## TypeScript との対比

```typescript
// TS: Uint8Array は ArrayBuffer の "ビュー"
function processBytes(data: Uint8Array): void {
    // data を所有していない、外から借りているだけ
    for (const byte of data) { /* ... */ }
}

const buf = new ArrayBuffer(1024);
const view = new Uint8Array(buf);     // バッファを所有せず参照
processBytes(view);
```

```cpp
// C++: std::span は連続データへの non-owning view
#include <span>

void process_bytes(std::span<const uint8_t> data) {
    // data は所有しない — ポインタ+長さのペアに過ぎない
    for (uint8_t byte : data) { /* ... */ }
}

std::vector<uint8_t> buf(1024);
process_bytes(buf);                        // vector から暗黙変換
process_bytes({buf.data(), buf.size()});   // 明示的に構築
```

## `std::span` の特性

```cpp
std::span<T>         // 可変ビュー（書き込み可）
std::span<const T>   // 読み取り専用ビュー

// 静的サイズ（コンパイル時に確定）
std::span<float, 17> keypoints;   // 17要素固定

// 動的サイズ
std::span<float> scores;          // サイズは実行時に決まる

// 部分ビュー
auto first5 = span.first(5);
auto last3  = span.last(3);
auto mid    = span.subspan(2, 4);  // offset=2, count=4
```

## ONNX Runtime との相互運用 (Chapter 8)

ONNX Runtime は C API を持つ。`std::span` を使うと境界が明確になる：

```cpp
// C API の典型的なパターン:
// void* と size_t でデータを渡す
OrtApi::CreateTensorWithDataAsOrtValue(
    info,
    data_ptr,    // void*
    data_len,    // size_t (バイト数)
    shape,       // int64_t*
    shape_len,   // size_t
    ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
    &tensor
);

// std::span を経由すると型と長さが一緒に来るので安全
void fill_input_tensor(std::span<const float> input_data, OrtValue* tensor) {
    // input_data.data() → void* に変換
    // input_data.size_bytes() → バイト数
    memcpy(ort_buffer, input_data.data(), input_data.size_bytes());
}
```

## `reinterpret_cast` の安全な使い方

ONNX や OpenCV の C API では型変換が避けられない場面がある：

```cpp
// NG: 任意の型への reinterpret_cast は UB になりやすい
float* bad = reinterpret_cast<float*>(void_ptr);

// OK: char/uint8_t は "バイトビュー" として特別に許可されている
const uint8_t* bytes = reinterpret_cast<const uint8_t*>(float_ptr);

// OK: std::bit_cast (C++20) — 型安全なビット変換
float f = 1.0f;
uint32_t bits = std::bit_cast<uint32_t>(f);  // IEEE 754 ビット列として解釈
```

## Chapter 8 でのスタブ設計

Chapter 8 はONNX Runtime をオプション（`MODERN_CPP_ENABLE_ONNX OFF`）にしている。ONNX なしでもインターフェースは同じ：

```cpp
// include/media/pose_estimator.hpp — インターフェースは常にある
struct KeyPoint { cv::Point2f position; float score; };
using PoseResult = std::array<KeyPoint, 17>;  // MoveNet の出力: 17関節

class PoseEstimator {
public:
    [[nodiscard]] bool load(std::string_view model_path);
    [[nodiscard]] bool is_loaded() const;
    [[nodiscard]] std::optional<PoseResult> estimate(const cv::Mat& frame);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;  // PIMPL: ONNX 依存をヘッダから隠す
};
```

### PIMPL パターンのメリット

```cpp
// ヘッダに #include <onnxruntime_cxx_api.h> が不要
// → ONNX Runtime がない環境でもコンパイルできる
// → ヘッダの変更がソースファイルのみに局所化される（ビルド時間短縮）

// src/pose_estimator.cpp にだけ ONNX の詳細が入る
#ifdef MODERN_CPP_WITH_ONNX
#include <onnxruntime_cxx_api.h>
struct PoseEstimator::Impl {
    Ort::Session session_{nullptr};
    // ...
};
#else
struct PoseEstimator::Impl {};  // スタブ
#endif
```

## まとめ

- `std::span<const T>` = TS の `ReadonlyArray<T>` + ポインタ（非所有）。
- C API とのインターフェースで「長さとポインタを別々に渡す」パターンを型安全にまとめられる。
- PIMPL パターンで外部ライブラリへの依存をヘッダから隠し、コンパイル時間を短縮できる。
- `std::bit_cast` (C++20) を使うと `reinterpret_cast` より安全な型変換ができる。
