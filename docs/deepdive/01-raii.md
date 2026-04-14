# 深掘り 01: RAII とデストラクタ — なぜ `finally` が要らないのか

## TL;DR

- C++ ではオブジェクトがスコープを出ると**必ずデストラクタが呼ばれる**。例外が飛んでも保証される。
- この仕組みを使って「コンストラクタでリソース取得、デストラクタで解放」を設計パターン化したのが **RAII**。
- TS の `finally` や Go の `defer` と似た目的を、言語のオブジェクトモデルで実現している。
- コピーを禁止 (`= delete`) してムーブのみ許可 (`= default`) すると、所有権を型で表現できる。

---

## なぜ `finally` が要らないのか

TypeScript でリソースを安全に解放するには:

```typescript
// TS: try-finally でリソース解放を保証
const cam = openCamera(0);
try {
    const frame = cam.readFrame();
    processFrame(frame);
} finally {
    cam.release(); // 例外が飛んでも必ず実行される
}
```

C++ では:

```cpp
// C++: スコープを出るとデストラクタが自動で呼ばれる
{
    media::CameraCapture cam;
    cam.open(0);
    auto frame = cam.read_frame();
    process_frame(frame);
} // ← ここで ~CameraCapture() が自動呼出。例外があっても同じ。
```

`finally` を書かなくていい理由は、**C++ のスタック巻き戻し (stack unwinding)** にある。
例外が飛ぶと、スタック上のすべてのオブジェクトに対してデストラクタが**逆順に**呼ばれる。
これが「例外安全性」の基盤。

> TS (Node 18+) の `Symbol.dispose` / `using` 構文は C++ の RAII を参考に設計された。

---

## RAII の 3 ステップ

```
1. コンストラクタ: リソースを取得（カメラ open、ファイル open、malloc など）
2. オブジェクトを使う: リソースを操作
3. デストラクタ: リソースを解放（release、close、free など）
```

リソースの寿命 = オブジェクトの寿命。これが RAII の本質。

---

## 5つの特殊メンバ関数

C++ のクラスには自動生成される「特殊メンバ関数」が 5 種類ある。

| 関数 | 生成条件 | 役割 |
|---|---|---|
| デフォルトコンストラクタ `T()` | 明示定義がない場合 | オブジェクト初期化 |
| コピーコンストラクタ `T(const T&)` | 明示定義がない場合 | 別オブジェクトからコピー |
| コピー代入 `T& operator=(const T&)` | 明示定義がない場合 | 代入でコピー |
| ムーブコンストラクタ `T(T&&)` | コピー系が定義されていない場合 | 別オブジェクトから所有権移転 |
| ムーブ代入 `T& operator=(T&&)` | コピー系が定義されていない場合 | 代入で所有権移転 |
| デストラクタ `~T()` | 常に存在 | オブジェクト破棄 |

### Rule of Five（5 の規則）

**「デストラクタを自前で書いたら、残り 4 つも全部定義しろ」**というルール。

`CameraCapture` では:

```cpp
class CameraCapture {
    ~CameraCapture();                             // ← 自前で書く
    CameraCapture(const CameraCapture&) = delete; // ← コピー禁止を明示
    CameraCapture& operator=(const CameraCapture&) = delete;
    CameraCapture(CameraCapture&&) = default;     // ← ムーブはOK
    CameraCapture& operator=(CameraCapture&&) = default;
};
```

### Rule of Zero（0 の規則）

5 つ全部コンパイラに生成させる（= 何も書かない）と、自動で合理的な動作になる。
メンバが `std::unique_ptr` など RAII オブジェクトだけなら、このルールで十分。

---

## `= delete` と `= default`

```cpp
// コンパイラに「生成するな」と指示
CameraCapture(const CameraCapture&) = delete;

// コンパイラに「デフォルト実装を生成しろ」と指示
CameraCapture(CameraCapture&&) = default;
```

`= delete` は「使おうとしたらコンパイルエラー」という型レベルの制約。
TS で型を使って「この引数は渡せない」を表現するのと同じ発想で、
`= delete` で「このオペレーションは禁止」を型で表現できる。

---

## コピー禁止にする理由

カメラは物理デバイス（ファイルハンドル、DBコネクションなど）の典型例。

```cpp
CameraCapture cam1;
cam1.open(0);

CameraCapture cam2 = cam1; // ← コピーを許可すると何が起きるか？
```

- `cam1` と `cam2` が**同じデバイスハンドル**を持つ
- `cam2` がスコープを出て `~CameraCapture()` を呼ぶ → `cap_.release()`
- その後 `cam1` のデストラクタが `cap_.release()` を呼ぶ → **二重解放**

これが `= delete` でコピーを禁止する理由。

---

## ムーブ後の「有効だが未定義ではない」状態

C++ のムーブ後オブジェクトは「有効だが内容は未定義」ではなく、
「有効で安全にアクセスできるが、値の内容は不定」と定義されている。

`CameraCapture` のムーブ後:

```cpp
CameraCapture cam_a;
cam_a.open(0); // cam_a がカメラを開く
CameraCapture cam_b = std::move(cam_a); // 所有権を cam_b に移転

// cam_a は is_opened() == false: 安全だがカメラを持っていない
// cam_b は is_opened() == true: カメラを所有している
```

`cv::VideoCapture` がムーブ後に `isOpened() == false` を返すことが
OpenCV の仕様として保証されているため、`= default` なムーブで十分。

---

## まとめ: RAII の恩恵

| 問題 | `finally` / `defer` | RAII |
|---|---|---|
| リソースリーク防止 | ○ (手動) | ○ (自動) |
| 例外安全 | ○ (try-finally) | ○ (stack unwinding) |
| 早期 return での安全 | △ (書き忘れリスク) | ○ (自動) |
| 所有権の型表現 | △ (慣習依存) | ○ (`= delete` / unique_ptr) |
| コード量 | 多い | 少ない |

RAII は「コードで書かなくていい」ことが多いほど強力なパターン。
デストラクタに信頼を置けると、早期 return が気軽に書けるようになる。

---

## 関連

- [02-move-semantics.md](./02-move-semantics.md) — ムーブとコピーの詳細
- `chapter01_camera_view/reference/include/media/camera_capture.hpp` — 実装例
