# 深掘り 04: concepts — TS の interface と C++ concept の違い

## TL;DR

- C++ の `concept` は「型がある操作をサポートするか」をコンパイル時に検査する
- TS の `interface` と似ているが、継承不要・vtable なし・実行時コストゼロ
- 仮想関数は「動的ディスパッチ」（実行時に何の関数を呼ぶか決まる）
- concept を使ったテンプレートは「静的ディスパッチ」（コンパイル時に決まる）

---

## TypeScript の interface との比較

```typescript
// TS: interface による制約
interface Detector {
    process(frame: Mat): DetectorOutput | null;
}

class ColorTracker implements Detector {
    process(frame: Mat): DetectorOutput | null {
        // ...
    }
}

function runDetector(detector: Detector, frame: Mat): void {
    // 実行時: detector.process の実体を vtable で探す（動的ディスパッチ）
    const result = detector.process(frame);
}
```

```cpp
// C++: concept による制約
template <typename T>
concept Detector = requires(T t, const cv::Mat& frame) {
    { t.process(frame) } -> std::same_as<std::optional<DetectorOutput>>;
};

class ColorTracker {
public:
    // implements Detector を書かずに、process() を定義するだけで OK
    std::optional<DetectorOutput> process(const cv::Mat& frame);
};

template <Detector T>
void run_detector(T& detector, const cv::Mat& frame) {
    // コンパイル時: T の process() を直接呼び出す（静的ディスパッチ）
    auto result = detector.process(frame);
}
```

---

## 仮想関数（動的ディスパッチ）との比較

```cpp
// 仮想関数版（従来の多態性）
class DetectorBase {
public:
    virtual ~DetectorBase() = default;
    virtual std::optional<DetectorOutput> process(const cv::Mat&) = 0;
};

class ColorTrackerVirtual : public DetectorBase {
public:
    std::optional<DetectorOutput> process(const cv::Mat& frame) override;
};

// 実行時に vtable を経由して呼び出す（間接呼び出し）
std::unique_ptr<DetectorBase> det = std::make_unique<ColorTrackerVirtual>(...);
det->process(frame); // ← ポインタのデリファレンス + vtable ルックアップ
```

| | 仮想関数 | concept + template |
|---|---|---|
| ディスパッチ | 動的（実行時） | 静的（コンパイル時） |
| オーバーヘッド | vtable ルックアップ（〜数ns） | ゼロ（インライン化可能） |
| 実行時多態性 | ○ | × （型を1つに決める必要） |
| ポリモーフィックコンテナ | `vector<shared_ptr<Base>>` 可 | `vector<T>` は T 固定 |
| エラーメッセージ | 実行時例外 | コンパイル時エラー（より明確） |
| コードサイズ | バイナリが小さい | テンプレート実体化で増える |

---

## requires 節の構文

```cpp
template <typename T>
concept Detector = requires(T t, const cv::Mat& frame) {
    // 単純な呼び出し可能チェック
    t.process(frame);

    // 戻り値の型まで指定するチェック
    { t.process(frame) } -> std::same_as<std::optional<DetectorOutput>>;

    // 型メンバのチェック（例）
    // typename T::output_type;
};
```

`requires` の内側に書ける条件:
1. **単純要件**: `t.process(frame)` — その式がコンパイルできること
2. **型要件**: `typename T::output_type` — 型メンバが存在すること
3. **複合要件**: `{ expr } -> concept` — 式の戻り値型が concept を満たすこと
4. **ネスト要件**: `requires(...)` — 追加条件

---

## 静的ディスパッチのコスト実測（概念）

```cpp
// template<Detector T> はコンパイル時に特定の型に「インスタンス化」される
template <Detector T>
void run_detector(T& det, const cv::Mat& frame) {
    auto r = det.process(frame); // 直接呼び出し（inline 展開される）
}

// これは実際には以下と同等のコードが生成される
void run_detector_ColorTracker(ColorTracker& det, const cv::Mat& frame) {
    auto r = det.process(frame); // ColorTracker::process() を直接呼ぶ
}
```

vtable を経由しないため、CPU の分岐予測ミスが起きにくく、インライン化の恩恵を受けやすい。
リアルタイム音声/映像処理では数ns の差が体感できる場合がある。

---

## 型消去（Type Erasure）: 両方のいいとこ取り

「実行時多態性は欲しいが vtable の継承関係は嫌」な場合は型消去パターンを使う:

```cpp
// std::function は型消去の典型例
std::function<std::optional<DetectorOutput>(const cv::Mat&)> process_fn;

// ColorTracker の process を関数オブジェクトとして格納
ColorTracker tracker{...};
process_fn = [&tracker](const cv::Mat& f) {
    return tracker.process(f);
};

// 型を知らずに呼べる（実行時多態性）
auto result = process_fn(frame);
```

`std::function` の内部実装は小オブジェクト最適化 (SOO) + 関数ポインタ。
vtable ほどでないがゼロでもないオーバーヘッドがある。

---

## static_assert による concept 検証

```cpp
// コンパイル時に「この型は Detector を満たすか」を確認する
static_assert(Detector<ColorTracker>,
    "ColorTracker は Detector concept を満たさなければならない");

// リファクタリングで process() の戻り値を変えてしまった場合:
//   static_assert で即座にビルドエラーになる
//   → 「型安全なリファクタリング」を実現
```

TS の型チェック (`tsc --strict`) をビルド時に行うイメージ。
テストを書かなくても、静的な型の整合性はコンパイラが保証する。

---

## 関連

- `chapter04_detector_abstraction/reference/include/media/detector.hpp` — concept の定義
- `chapter04_detector_abstraction/reference/include/media/color_tracker.hpp` — ColorTracker の実装
- `chapter04_detector_abstraction/reference/tests/test_detector.cpp` — static_assert の使用例
