# 深掘り 02: ムーブセマンティクス — コピーとムーブと参照の使い分け

## TL;DR

- **コピー**: 値を複製する。元のオブジェクトはそのまま。コストが高い場合がある。
- **ムーブ**: 所有権（内部リソース）を移転する。元は「空っぽ」になる。O(1) が多い。
- **参照 (`const T&`)**: 所有権も複製もしない。借りるだけ。
- `cv::Mat` は参照カウント付き。「コピー」しても実際はポインタのコピーのみ（浅いコピー）。

---

## C++ の値カテゴリ: lvalue と rvalue

```cpp
int x = 42;    // x は lvalue（名前があり、アドレスを取れる）
int y = x + 1; // x+1 は rvalue（一時的な値、アドレスがない）

std::string s = "hello";
std::string t = s;            // コピー: s は lvalue なのでコピーコンストラクタが呼ばれる
std::string u = std::move(s); // ムーブ: s は std::move で rvalue 参照にキャスト
// この時点で s は "有効だが空の状態"（s.empty() == true）
```

TS との比較:

```typescript
// TS: 代入は基本的に参照コピー（オブジェクトの場合）
const a = { x: 1 };
const b = a;         // b と a は同じオブジェクトを指す（浅いコピー）
b.x = 2;             // a.x も 2 になる！

// C++: 代入はデフォルトで「値コピー」
struct Point { int x; };
Point a{ 1 };
Point b = a;   // b は a とは独立したコピー
b.x = 2;       // a.x は変わらない（1 のまま）
```

---

## std::move の正体

`std::move` はムーブを**強制**するのではなく、型を「rvalue 参照」にキャストするだけ。

```cpp
// std::move の擬似実装（実際は <utility> の中に同等のものがある）
template<typename T>
decltype(auto) move(T&& v) {
    return static_cast<std::remove_reference_t<T>&&>(v);
}
```

呼んだ後に「リソースがなくなる」のは、ムーブコンストラクタ/代入演算子が
内部リソース（ポインタ、ハンドル）を移転して元を null/空にするから。

---

## cv::Mat の参照カウント

OpenCV の `cv::Mat` は Java の参照型や TS のオブジェクトに近い動作をする。

```cpp
cv::Mat a = cv::imread("photo.jpg"); // 実際の画像データをヒープに確保
cv::Mat b = a;    // ← シャローコピー。内部バッファは共有される
                  //   参照カウント +1 になるだけ（高速）
b.at<uchar>(0,0) = 255; // a のデータも変わる！（同じバッファ）

cv::Mat c = a.clone(); // ← ディープコピー。独立したバッファを作る
c.at<uchar>(0,0) = 0;  // a は変わらない
```

関数引数での渡し方:

```cpp
// const Mat& を渡す: コピーも参照カウントも増えない。推奨。
void process(const cv::Mat& frame) { /* ... */ }

// Mat を渡す: シャローコピー（O(1)）。独立操作したいなら .clone() を内部で呼ぶ。
void process(cv::Mat frame) { frame = frame.clone(); /* ... */ }

// Mat&& でムーブ受け取り: frame を所有して変更する関数向け。
void transform(cv::Mat&& frame) { /* ... */ }
```

---

## 関数引数での使い分けまとめ

| 引数の書き方 | 意味 | 典型ユースケース |
|---|---|---|
| `T val` | 値コピー（またはムーブ） | 小さな型 (int, float) |
| `const T& val` | 読み取り専用参照（コピーなし） | 大きな型の読み取り専用引数 |
| `T& val` | 可変参照（副作用あり） | 出力引数（非推奨傾向。返り値推奨） |
| `T&& val` | ムーブ参照（所有権を受け取る） | sink 引数（この関数がリソースを消費） |

`track(const cv::Mat& bgr, const HsvRange& range)` は:
- `bgr`: 大きな画像データを読み取るだけ → `const&` が最適
- `range`: 小さなスカラー構造体だが一貫性のため `const&`

---

## ムーブが重要な場面

```cpp
// パイプライン的な処理: 中間結果をムーブして不要なコピーを回避
std::optional<cv::Mat> CameraCapture::read_frame() {
    cv::Mat frame;
    cap_ >> frame;
    if (frame.empty()) return std::nullopt;
    return frame; // ← RVO または暗黙ムーブ。コンパイラが最適化
}
```

**RVO (Return Value Optimization)**: `return` の直後にローカル変数を渡すとき、
コンパイラはコピーもムーブもせず、直接呼び出し元のメモリに構築する最適化。

```cpp
// ムーブセマンティクスでコレクションへの追加が高速になる
std::vector<cv::Mat> frames;
frames.push_back(cam.read_frame().value()); // ← ムーブで追加
```

---

## 関連

- [01-raii.md](./01-raii.md) — デストラクタとリソース管理
- `chapter02_color_tracker/reference/include/media/color_tracker.hpp` — `const T&` の使用例
- `chapter02_color_tracker/reference/src/color_tracker.cpp` — `cv::Mat` の操作例
