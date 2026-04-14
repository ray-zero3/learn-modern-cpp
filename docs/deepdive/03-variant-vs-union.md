# 深掘り 03: std::variant vs discriminated union — タグ付き共用体の実装

## TL;DR

- C の `union` は「複数の型を同じメモリに重ねる」構造だが、どの型が入っているか追跡しない（危険）
- `std::variant<T1, T2, ...>` は「どの型が入っているか」をタグで管理するタイプセーフな union
- TS の `discriminated union` に相当するが、コンパイラが型の網羅性を確認できる
- `std::visit` は variant の実際の型に応じてラムダ/関数を選択して呼び出す

---

## TypeScript の discriminated union との対比

```typescript
// TS: discriminated union
type OscValue =
    | { kind: 'int'; value: number }
    | { kind: 'float'; value: number }
    | { kind: 'string'; value: string }
    | { kind: 'bool'; value: boolean };

function formatArg(val: OscValue): string {
    switch (val.kind) {
        case 'int':    return `i:${val.value}`;
        case 'float':  return `f:${val.value}`;
        case 'string': return `s:${val.value}`;
        case 'bool':   return `b:${val.value}`;
    }
}
```

```cpp
// C++: std::variant
using OscValue = std::variant<int, float, std::string, bool>;

std::string format_arg(const OscValue& val) {
    return std::visit([](const auto& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, int>)         return "i:" + std::to_string(v);
        if constexpr (std::is_same_v<T, float>)       return "f:" + std::to_string(v);
        if constexpr (std::is_same_v<T, std::string>) return "s:" + v;
        if constexpr (std::is_same_v<T, bool>)        return std::string("b:") + (v ? "true" : "false");
    }, val);
}
```

TS と C++ の主な違い:

| | TypeScript | C++ std::variant |
|---|---|---|
| 型の宣言 | `kind` フィールドで識別 | variant の型リストで識別 |
| 型の確認 | `val.kind === 'int'` | `std::holds_alternative<int>(val)` |
| 値の取り出し | `val.value` | `std::get<int>(val)` (失敗で例外) |
| 網羅性チェック | 型システムで確認 | `std::visit` + `if constexpr` で確認 |
| 型安全 | ○ | ○ (std::get の失敗で例外) |

---

## C の union との違い

```c
// C: 危険な union（どの型が入っているか追跡しない）
union Value {
    int   i;
    float f;
    char* s;
};

// 「どの型が入っているか」は自分で覚えておく必要がある
Value v;
v.i = 42;
printf("%f\n", v.f); // UB! i を float として読んでいる
```

```cpp
// C++: 安全な std::variant
std::variant<int, float, std::string> v = 42;

// 間違った型でアクセスしようとすると例外
try {
    float f = std::get<float>(v); // throws std::bad_variant_access
} catch (const std::bad_variant_access&) {
    // 安全に捕捉できる
}

// 型チェックしてからアクセス
if (std::holds_alternative<int>(v)) {
    int i = std::get<int>(v); // 安全
}
```

---

## std::visit の仕組み

`std::visit` は「dispatch table（関数ポインタの表）」に基づいて実際の型に対応する関数を呼ぶ。

```cpp
// overload パターン（複数のラムダを1つの visitor にまとめる慣用句）
template<typename... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};

// C++17 以降: CTAD で推論
auto visitor = overload{
    [](int v)         { std::cout << "int: " << v; },
    [](float v)       { std::cout << "float: " << v; },
    [](std::string v) { std::cout << "string: " << v; },
    [](bool v)        { std::cout << "bool: " << v; },
};

std::variant<int, float, std::string, bool> val = 3.14F;
std::visit(visitor, val); // → "float: 3.14" が出力される
```

`if constexpr` を使う方法（この章で採用）:

```cpp
std::visit([](const auto& v) {
    using T = std::decay_t<decltype(v)>;
    if constexpr (std::is_same_v<T, int>)    { /* int の処理 */ }
    else if constexpr (std::is_same_v<T, float>) { /* float の処理 */ }
    // ...
}, val);
```

`if constexpr` はコンパイル時の条件分岐。`if` と違い、コンパイルされない分岐はインスタンス化されない。

---

## std::variant の内部構造（概要）

```
variant<int, float, string, bool>:
  +----------+----------+
  | index(2b)| storage  |
  +----------+----------+
  | 0 = int  | 4 bytes  |
  | 1 = float| 4 bytes  |
  | 2 = string| std::string::sizeof() |
  | 3 = bool | 1 byte   |
  +----------+----------+
```

- `index_` : 現在入っている型のインデックス（0-3）
- `storage_` : 最大の型に合わせたアライン済みバッファ（`std::aligned_storage` 相当）

合計サイズ: 最大メンバ + アラインメント調整 + インデックス分。
`sizeof(std::variant<int, float, std::string, bool>)` ≒ 32 バイト程度（実装依存）。

---

## この章での自前 OSC 実装について

oscpack ライブラリが arm64 (Apple Silicon) の `long` サイズ問題で使えないため、
OSC 1.0 の最小実装を `media/minimal_osc.hpp` に記述した。

OSC 1.0 パケット構造:
```
/address\0[pad]   ← null 終端 + 4 バイトアライン
,iff\0[pad]       ← type tag (カンマ始まり)
[4 bytes int]     ← big-endian int32
[4 bytes float]   ← big-endian float32
[4 bytes float]   ← big-endian float32
```

実装のポイント:
- `std::endian::native` で実行時にエンディアンを判定（C++20 機能）
- `__builtin_bswap32` でバイトスワップ（コンパイラ組み込み）
- BSD ソケット API で UDP 送信（macOS/Linux 共通）

---

## 関連

- `chapter03_osc_sender/reference/include/media/osc_value.hpp` — OscValue の定義
- `chapter03_osc_sender/reference/include/media/minimal_osc.hpp` — 自前 OSC 実装
- `chapter03_osc_sender/reference/src/osc_sender.cpp` — std::visit の実用例
