# 深掘り 00: なぜヘッダが必要なのか — コンパイル単位とリンカ

## TL;DR

- C++ のコンパイラは **1ファイル (= Translation Unit)** だけを見る。他のファイルの中身は**見えない**。
- なのに「関数を呼ぶ」には **その関数のシグネチャ**（戻り値・引数）を知っている必要がある。
- この「シグネチャだけ先に教えるファイル」が **ヘッダ (`.hpp` / `.h`)**。
- 実体（関数本体）は別の `.cpp` に書いて、コンパイラが生成した `.o` をリンカが繋ぐ。
- TS と決定的に違うのはここ: **tsc は全部舐めるが、C++ は 1 ファイルずつ独立で見る**。

## モデル図

```
greeting.cpp ──┐
               ├── clang++ ──► greeting.o ──┐
greeting.hpp ──┘                             ├── ld ──► chapter00_ref_app
                                             │
main.cpp ────┐                               │
             ├── clang++ ──► main.o ─────────┘
greeting.hpp ─┘
```

`greeting.hpp` は **両方のコンパイル単位** に貼り込まれる。両方の `.o` に「`greet::greet(std::string_view)` が存在する」という前提が埋め込まれ、リンク時に本体と結合される。

## TS との対比

```ts
// utils.ts
export function greet(name: string): string { return `Hello, ${name}`; }
```

```ts
// main.ts
import { greet } from "./utils";
greet("World");
```

TS（tsc）は両方のファイルを同時に解析する。`greet` の実装が `utils.ts` にあることは tsc が知っている。なので型宣言を別に書く必要がない（`.d.ts` は外部 JS 消費用など特殊ケース）。

C++ では:

```cpp
// greeting.hpp
namespace greet { std::string greet(std::string_view); }  // ← シグネチャだけ
```

```cpp
// greeting.cpp
#include "greet/greeting.hpp"
namespace greet { std::string greet(std::string_view n) { ... } }  // ← 実体
```

```cpp
// main.cpp
#include "greet/greeting.hpp"
int main() { greet::greet("World"); }
```

main.cpp をコンパイルする clang++ は **greeting.cpp の中身を知らない**。
ヘッダ経由で「そういう関数が**ある**」と教えるだけ。実体解決はリンカの仕事。

## `#include` は「コピペ」

```cpp
#include "greet/greeting.hpp"
```

これは**プリプロセッサ**が `.hpp` のテキストをそっくりその場に貼り付ける命令です。
つまり main.cpp は展開後には:

```cpp
#pragma once
#include <string>
#include <string_view>
namespace greet { /* ... */ }

// ... この下に main.cpp の本体
int main() { ... }
```

という 1 つの巨大なテキストになってからコンパイルされます。
これが「ヘッダは軽く保つべき」の本質的理由。重いヘッダを include すると、**インクルードした全ファイルがその分重くコンパイルされる**。

## `#pragma once` が守っていること

同じヘッダが同じコンパイル単位に **2 回以上展開されること** を防ぎます。

```cpp
// a.hpp   (pragma once なし)
struct Foo { int x; };

// b.hpp
#include "a.hpp"   // Foo が展開される

// main.cpp
#include "a.hpp"   // Foo がもう一度展開される
#include "b.hpp"   // a.hpp 経由でさらに展開
// -> Foo の定義が3回並ぶ → 「redefinition of Foo」エラー
```

`#pragma once` が付いていると、プリプロセッサが「このファイルは既に展開済み」と記録し、2 回目以降の `#include` を無視します。伝統的には:

```cpp
#ifndef GREET_GREETING_HPP
#define GREET_GREETING_HPP
// ...
#endif
```

というインクルードガードで同じことをしていました。`#pragma once` はほぼ全コンパイラでサポートされた非標準機能ですが、主要コンパイラで問題ないので本プロジェクトでは `#pragma once` を採用。

## ODR — One Definition Rule

これは C++ の**最も重要なルールの1つ**です。

> 非 inline な関数・変数の**定義**は、プログラム全体で**ちょうど1回**でなければならない。**宣言**は何回あってもよい。

これを破ると、**リンクエラー**か、**もっと悪い場合は "未定義動作"** になります（動くこともあるが、結果は保証されない）。

典型的な違反:

```cpp
// greeting.hpp （ダメな例）
std::string greet(std::string_view n) { return "Hello"; }  // 定義をヘッダに書いている
```

これを2つの .cpp から `#include` すると、両方の `.o` に `greet` の実体が入り、リンカが「二重定義」と怒る。

正しい形:

```cpp
// greeting.hpp
std::string greet(std::string_view n);  // 宣言のみ
```

```cpp
// greeting.cpp
std::string greet(std::string_view n) { return "Hello"; }  // 定義はここだけ
```

### ヘッダに書いても良いもの

- `inline` 関数（ODR の例外: 各 TU に定義があっても OK と規格が保証）
- `constexpr` 関数（implicit inline）
- テンプレート（実体化のため定義が見える必要がある）
- クラス定義（`class Foo { ... };` は宣言扱い。メンバ関数の実装は `.cpp` に書くのが定石）
- `inline` 変数（C++17〜）

逆に言うと、**非 inline な「普通の関数」の本体はヘッダに書かない**。これが守れれば ODR 違反の 9 割は避けられます。

## 再コンパイルコストの観点

`.hpp` を 1 行変えると、それを `#include` した**全ての .cpp が再コンパイル**されます。
`.cpp` を 1 行変えても、その .cpp だけ再コンパイルすれば OK。

だから大規模プロジェクトでは「**ヘッダは最小**・実装詳細は `.cpp` に押し込む」が鉄則です。Pimpl イディオムやフォワード宣言もこの最適化の文脈で出てきます（詳細は後の章で）。

## まとめ

| 疑問 | 答え |
|---|---|
| なぜヘッダがある？ | C++ のコンパイル単位は1ファイル。シグネチャを事前に教える仕組みが必要 |
| `#include` は何？ | プリプロセッサによるテキストコピペ |
| `#pragma once` は何を防ぐ？ | 同一 TU 内の多重展開 → 多重定義エラー |
| ODR とは？ | 非 inline の「定義」はプログラム全体で 1 回 |
| ヘッダに書いていいのは？ | 宣言、inline/constexpr/テンプレート/クラス定義 |
| TS とどう違う？ | tsc は全ファイル舐める／C++ は 1 ファイルずつ独立 + リンカ結合 |
