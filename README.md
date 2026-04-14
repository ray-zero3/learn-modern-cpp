# モダンC++ 入門 — TypeScript 開発者のためのC++20ハンズオン

**TypeScript で大規模開発の経験がある方**を対象に、C++20 とモダンなツールチェインを実践的に学ぶハンズオン教材です。

OpenCV を使ったコンピュータビジョンアプリケーションを章ごとに育てながら、C++ 固有の概念を TypeScript との対比で体系的に理解していきます。

---

## 対象読者

| 項目 | 前提 |
|---|---|
| メイン言語 | TypeScript（大規模アプリ開発経験あり） |
| C++経験 | 初級（簡単なプログラムのみ） |
| プログラミング基礎 | あり（型システム・非同期・所有権の概念は TS 以外の文脈で理解可能） |

`for` ループや `if` 文の説明はしません。型・非同期・所有権などの概念は **TypeScript とのアナロジー** で解説し、C++ 固有の罠（所有権・ムーブ・RAII・テンプレート・未定義動作）を重点的に深掘りします。

---

## 最終成果物

Webカメラ映像をリアルタイムに解析し、OSC プロトコルでネットワーク送受信できる **CLI + GUI アプリケーション**。

```
カメラ入力 → 色検出 / オプティカルフロー → OSC 送信 → 外部アプリで可視化
                                              ↑
                                         OSC 受信でパラメータ調整 (Ch.9)
```

---

## 技術スタック

| カテゴリ | 採用 |
|---|---|
| 言語規格 | **C++20** (`concepts`, `ranges`, `coroutines`, `std::span`, `std::jthread`) |
| コンパイラ | **Homebrew LLVM clang++** (Apple Clang より C++20 サポートが充実) |
| ビルドシステム | **CMake 3.25+** (`FetchContent` / `find_package` を両方使用) |
| 画像処理 | **OpenCV 4.x** (Homebrew 導入済みを想定) |
| OSC | 自前最小実装 `minimal_osc.hpp` (oscpack は Apple Silicon 非互換) |
| GUI | **Dear ImGui** + GLFW + OpenGL3 |
| テスト | **doctest** (ヘッダオンリー) |
| フォーマッタ | **clang-format** |
| 静的解析 | **clang-tidy** |

---

## 章立て

| 章 | テーマ | C++ の学び |
|---|---|---|
| **Ch.0** | C++プロジェクトの 0→1 | CMake 基礎、ヘッダ/ソース分離、ODR、FetchContent |
| **Ch.1** | カメラ映像をウィンドウに表示 | RAII、`std::optional`、`= delete` / `= default` |
| **Ch.2** | HSV 色域トラッカー | `std::span`、`std::ranges`、ムーブセマンティクス |
| **Ch.3** | OSC 送信の統合 | `std::variant` + visitor パターン、UDP ソケット |
| **Ch.4** | Detector 抽象化 | `concepts` / `requires`、静的 vs 動的ディスパッチ |
| **Ch.5** | オプティカルフロー検出器 | `std::array` vs `std::vector`、キャッシュ局所性 |
| **Ch.6** | 非同期パイプライン | `std::jthread`、`std::atomic`、lock-free queue |
| **Ch.7** | 設定ファイル (TOML) | `std::expected`、エラー伝搬の型安全な設計 |
| **Ch.8** | ポーズ推定 (optional) | ONNXRuntime、C API 相互運用、PIMPL パターン |
| **Ch.9** | OSC 受信 + ImGui 完成形 | Dear ImGui 統合、observer パターン、リソース解放順序 |

各章は **`reference/`（完成コード）と `workspace/`（作業用）** のペアで構成されています。

---

## セットアップ

### 前提条件

```bash
# Homebrew LLVM
brew install llvm

# CMake
brew install cmake

# OpenCV (Ch.1 以降で使用)
brew install opencv

# Ninja (任意、ビルド高速化)
brew install ninja
```

### ビルド

```bash
git clone https://github.com/ray-zero3/learn-modern-cpp.git
cd learn-modern-cpp

cmake -S . -B build \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -GNinja

cmake --build build
ctest --test-dir build --output-on-failure
```

初回は doctest / ImGui / toml++ が `FetchContent` で自動取得されます（数十秒）。

### 特定の章だけビルドする

```bash
cmake --build build --target chapter02_ref_app
```

---

## ディレクトリ構成

```
modern-cpp/
├── CMakeLists.txt
├── cmake/
│   ├── Dependencies.cmake    # FetchContent で取得する外部ライブラリ
│   └── Warnings.cmake        # コンパイラ警告の共通設定
├── docs/
│   ├── index.html            # 学習ガイド（全章 + 深掘り記事）
│   ├── setup.md
│   └── deepdive/             # 深掘り記事 Markdown
├── chapter00_hello/
│   ├── reference/            # 完成コード（読む用）
│   └── workspace/            # 作業場所（前章の完了物がそのまま入る）
├── chapter01_camera_view/ ...
...
└── chapter09_osc_receive/
```

---

## 学習の進め方

1. `docs/index.html` をブラウザで開いて概要を把握する
2. 各章の `reference/README.md` を読んでゴールと TypeScript との差分を確認する
3. `workspace/` で TDD サイクル（RED → GREEN → REFACTOR）を回す
4. `docs/deepdive/` の深掘り記事で背景・設計思想を理解する
5. 達成チェックリストを埋めて次章へ

---

## ライセンス

MIT
