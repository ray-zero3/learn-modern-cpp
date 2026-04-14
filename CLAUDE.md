# modern-cpp — TypeScript 開発者のためのモダンC++ 入門

このリポジトリは、**OpenCV を使ったコンピュータビジョンアプリケーション** を段階的に育てながら、**モダンC++ (C++20) と CMake 周辺ツールチェイン** を学ぶための学習プロジェクトです。

最終成果物は、Webカメラ映像をリアルタイムに解析してネットワーク経由でデータ送受信できる CLI + GUI アプリケーションです。

---

## 学習者のプロファイル（前提）

- **メイン言語**: TypeScript（大規模アプリ開発経験あり）
- **C++経験**: 初級（簡単なプログラムのみ）
- **プログラミング基礎**: あり（型システム、非同期、所有権の概念は TS 以外の文脈で理解可能）
この前提から、本教材は **一般的な C++ 入門とは異なり**、以下の方針で進めます：

- `for` ループや `if` 文の説明はしない
- 型・インターフェース・非同期などの概念は **TypeScript とのアナロジー** で説明する
- ただし **C++ 固有の概念（所有権、ムーブ、RAII、テンプレート、未定義動作）は深掘りする**
- メモリモデル、ABI、リンクなど「TSでは抽象化されている層」も必要に応じて説明する

---

## 学習方針

### 基本原則

1. **動かしながら学ぶ**: 各章は「動く成果物」で終わる。ビジュアルフィードバックのある題材なので、毎章何かが画面に映る / データが送られる。
2. **要所で深掘り**: 新機能が登場したら、その背景・設計思想・よくある落とし穴を `docs/deepdive/` に書く。
3. **テスト付き**: 各章で doctest を使ったテストを書く。TDD を基本とするが、ビジュアル系は後追いテストもOK。
4. **段階的拡張**: `chapter01/` から `chapter09/` まで、前章の成果物を次章で拡張する。各章は独立した CMake ターゲットとしてもビルド可能。
5. **モダンC++を使う**: C++20 を前提とし、必要に応じて C++23 の機能に触れる（コンパイラで使える範囲で）。`std::ranges`, `concepts`, `coroutines`, `std::span`, `std::expected`（またはその polyfill）を積極活用。

### コメント・ドキュメントの方針

- **コメントは日本語**。ただし識別子（変数名・関数名・クラス名）は英語。
- 概念を新規に導入した場所には、**TSとの対比コメント** を入れる（例: `// TSのinterfaceに相当するが、静的ディスパッチな点が違う`）。
- 各章の `README.md` に「この章で学ぶこと / TSとの差分 / 深掘りポイント」を明記する。

---

## 技術スタック

| カテゴリ | 採用 | 理由 |
|---|---|---|
| 言語規格 | **C++20** | `concepts`, `ranges`, `coroutines`, `std::span`, `std::jthread` が使える |
| コンパイラ | **Homebrew LLVM (clang++)** `/opt/homebrew/opt/llvm/bin/clang++` | Apple Clang より新しく、libc++ の ranges / coroutines 実装が揃っている |
| ビルドシステム | **CMake (3.25+)** | デファクト。`FetchContent` / `find_package` 両方学ぶ |
| 画像処理 | **OpenCV 4.x** (Homebrew導入済み) | カメラ入力、色変換、オプティカルフロー、検出器 |
| OSC | **自前最小実装** (`minimal_osc.hpp`) | oscpack は arm64 非互換のため UDP+big-endian のみ自前実装 |
| GUI | **Dear ImGui** + GLFW + OpenGL3 backend | パラメータ調整用の即席GUI。`FetchContent` で取得 |
| テスト | **doctest** | ヘッダオンリー、軽量、CMake統合が楽 |
| フォーマッタ | **clang-format** | LLVMスタイルベース + カスタマイズ |
| 静的解析 | **clang-tidy** | モダンC++のアンチパターン検出 |

---

## ディレクトリ構成（予定）

### workspace / reference 構造ルール

各章は `reference/`（お手本）と `workspace/`（作業用）を持つ:
- `reference/` = その章の完成形。ビルドが通ることを保証する
- `workspace/` = 前章の `reference/` のコピー（chapter00 のみ空）
- workspace に `// TODO` スタブは置かない。前章の実動コードがそのまま入る
- CMake は `if(EXISTS workspace/CMakeLists.txt)` で workspace が空でもビルドを通す

```
modern-cpp/
├── CLAUDE.md                  # このファイル
├── CMakeLists.txt             # ルート。各章をサブディレクトリとして追加
├── .clang-format
├── .clang-tidy
├── .gitignore
├── cmake/                     # 共通の CMake モジュール
│   ├── Dependencies.cmake     # FetchContent 定義
│   └── Warnings.cmake         # コンパイラ警告の共通設定
├── docs/
│   ├── index.html             # 学習ガイド（全章 + 深掘り記事を収録したシングルファイルHTML）
│   ├── setup.md               # 環境構築手順
│   └── deepdive/              # 深掘り記事 Markdown（00a,00b,01〜09。index.html にも埋め込み済み）
├── common/                    # 章を跨いで使うユーティリティ
│   ├── include/
│   └── src/
├── chapter00_hello/           # 第0章: 挨拶ライブラリ（CMake 0→1）
├── chapter01_camera_view/     # 第1章: カメラ映像をウィンドウに表示
├── chapter02_color_tracker/   # 第2章: HSV色域トラッカー
├── chapter03_osc_sender/      # 第3章: OSC送信の統合
├── chapter04_detector_abstraction/  # 第4章: Detector抽象化（concepts）
├── chapter05_optical_flow/    # 第5章: オプティカルフロー検出器追加
├── chapter06_async_pipeline/  # 第6章: コルーチン/ジョブシステム
├── chapter07_config/          # 第7章: 設定ファイル対応
├── chapter08_pose_estimation/ # 第8章: ONNXRuntime でポーズ推定（optional）
├── chapter09_osc_receive/     # 第9章: OSC受信で双方向化 + ImGui完成形
└── osc_examples/              # OSC 受信確認用サンプル（Python/Processing など）
```

---

## 章立てと学習目標

### Chapter 0: C++プロジェクトの 0→1
- **成果物**: 「挨拶ライブラリ + それを使う実行ファイル + doctestのテスト」が CMake で一発ビルドできる最小プロジェクト
- **C++の学び**: ヘッダ/ソース分離、CMakeの基本構造 (`add_library`, `add_executable`, `target_link_libraries`, `target_include_directories`, `target_compile_features`)、out-of-source build、`FetchContent`、Debug/Release、警告オプション
- **TS対比**: `package.json`/`tsconfig.json` → CMakeLists.txt、`npm install` → FetchContent、`.d.ts` vs ヘッダの違い
- **深掘り**: なぜヘッダが必要か（コンパイル単位とリンカ）、ODR（One Definition Rule）超入門

### Chapter 1: カメラ映像をウィンドウに表示
- **成果物**: 内蔵Webカメラの映像を 30fps で表示
- **C++の学び**: CMake超入門、RAII（`cv::VideoCapture`の寿命）、ヘッダ/ソース分割、名前空間
- **TS対比**: `#include` vs `import`、ヘッダの "宣言と定義の分離" がなぜあるか

### Chapter 2: HSV色域トラッカー
- **成果物**: 特定色の物体の重心と面積を検出して画面に描画
- **C++の学び**: `std::span`, `std::optional`, `std::ranges`, 値渡し/参照渡し/ムーブの使い分け
- **深掘り**: `cv::Mat` の参照カウント / コピーコスト / ムーブセマンティクス
- **TS対比**: 所有権、`const T&` vs TS の readonly

### Chapter 3: OSC送信の統合
- **成果物**: Chapter 2 の検出結果を OSC でネットワーク送信。受信アプリで可視化。
- **C++の学び**: `FetchContent` での外部ライブラリ取り込み、`std::variant` + visitor で OSC 型を表現
- **深掘り**: `std::variant` の内部実装、TS discriminated union との比較
- **TS対比**: ジェネリクス vs テンプレート（導入）

### Chapter 4: Detector抽象化（concepts）
- **成果物**: `Detector` concept を定義し、ColorTracker がそれを満たすように
- **C++の学び**: `concepts` / `requires` 節 / 仮想関数との比較 / CRTP
- **深掘り**: 静的ディスパッチ vs 動的ディスパッチのコスト、型消去 (`std::function` 的パターン)
- **TS対比**: TS の `interface` + structural typing と concepts の違い

### Chapter 5: オプティカルフロー検出器追加
- **成果物**: Farneback or Lucas-Kanade でフレーム間の動き量を検出し OSC 送信
- **C++の学び**: 新 Detector の追加（concepts の効果を体感）、循環バッファ、`std::array` vs `std::vector`
- **深掘り**: スタック vs ヒープ、メモリレイアウトと cache locality

### Chapter 6: 非同期パイプライン
- **成果物**: キャプチャ / 検出 / OSC送信 を別スレッドで並行処理
- **C++の学び**: `std::jthread`, `std::stop_token`, コルーチン (`std::coroutine_handle`, `co_yield`), lock-free queue
- **深掘り**: メモリモデル / `std::atomic` / happens-before
- **TS対比**: async/await は暗黙スケジューリング、C++ coroutines は明示的スケジューリング

### Chapter 7: 設定ファイル対応（TOML）
- **成果物**: HSV閾値、OSC送信先、カメラID等を TOML で指定
- **C++の学び**: `toml++` 取り込み、`std::expected`（or `tl::expected`）でエラー伝搬、結果型パターン
- **深掘り**: 例外 vs 結果型、C++標準の例外コストの実際

### Chapter 8: ポーズ推定（Optional）
- **成果物**: ONNXRuntime + MoveNet で身体の関節座標を取って OSC送信
- **C++の学び**: 外部バイナリ依存の CMake、C API との相互運用、`std::span<const float>` で tensor を扱う
- **スキップ可**: 重いのでここまで来たらご褒美章

### Chapter 9: OSC受信 + ImGui完成形
- **成果物**: OSC で外部からパラメータを受信して閾値をリアルタイム変更できる。ImGui でパラメータ調整 UI を実装。
- **C++の学び**: Dear ImGui 統合、コールバック設計、observer パターン、リソース開放順序（ImGui/GLFW/OpenCV のライフタイム）
- **仕上げ**: プリセット保存、FPS表示、パフォーマンス計測

---

## ビルド手順（初回セットアップ想定）

```bash
# 依存ライブラリ（未インストール分のみ）
brew install cmake glfw  # opencv は既にインストール済みのことを確認済み
# oscpack, imgui, doctest, toml++ は CMake の FetchContent で自動取得

# ビルド
cmake -S . -B build \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -GNinja   # ninja があれば。無ければ -G 省略

cmake --build build

# 特定章だけビルド
cmake --build build --target chapter02_color_tracker

# 実行
./build/chapter02_color_tracker/chapter02

# テスト
ctest --test-dir build --output-on-failure
```

macOS のカメラ使用許可プロンプトが初回起動時に出ます。

### 既知のビルド注意事項

- **oscpack は Apple Silicon (arm64) 非互換**: `int32` 型定義の問題があるため `MODERN_CPP_USE_OSCPACK=OFF`（デフォルト）で自前最小 OSC 実装を使用
- **doctest v2.4.11 + CMake 4.x**: `cmake_minimum_required` が古いため `CMAKE_POLICY_VERSION_MINIMUM 3.5` を `Dependencies.cmake` で設定済み
- **テストフレームワーク**: このプロジェクトでは **doctest** を使用（グローバル rules の GoogleTest 指定よりこちらを優先）

---

## コマンド早見表（Claude への指示用）

| やりたいこと | コマンド |
|---|---|
| ビルド | `cmake --build build` |
| クリーン | `rm -rf build && cmake -S . -B build ...` |
| テスト | `ctest --test-dir build --output-on-failure` |
| フォーマット | `find . -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \| xargs clang-format -i` |
| 静的解析 | `clang-tidy -p build path/to/file.cpp` |

### CMake ターゲット命名規則

- reference ターゲット: `chapter{NN}_ref_*`（例: `chapter03_ref_osc`, `chapter03_ref_tests`）
- workspace ターゲット: `chapter{NN}_ws_*`（例: `chapter03_ws_osc`, `chapter03_ws_tests`）
- 章番号は2桁ゼロ埋め（`00`〜`09`）

---

## Claude への指示（この教材を進める上でのルール）

1. **日本語で対話する**。コードコメントも原則日本語（TSとの対比メモを積極的に入れる）。
2. **章ごとに進める**。勝手に先の章に進まない。ユーザーが「次の章に進みたい」と言ったら次へ。
3. **1章の中でも段階を区切る**。以下のループを回す：
   1. その章のゴールとTSとの差分を説明
   2. 設計方針を示す（複数選択肢がある場合は比較表）
   3. テストから書く（doctest、TDD）
   4. 実装
   5. ビルド & 実行確認
   6. **要所の深掘り**（`docs/deepdive/` に記事を書く）
   7. ユーザーの理解確認と次章への接続
4. **先回りしすぎない**。TS経験者として自明な話（ループや条件分岐の構文）には時間を使わない。逆に **所有権・ムーブ・テンプレート・UB(未定義動作)・リンクなど C++ 固有の罠は必ず深掘りする**。
5. **簡潔な結論ファースト**で説明。長文の前に TL;DR を書く。
6. **破壊的な操作（`rm -rf`, `git push`, `git reset` 等）は実行前に必ず許可を取る**。
7. **新しい外部ライブラリを導入する時は代替案と比較して理由を添える**。
8. **深掘り記事は Markdown で `docs/deepdive/NN-topic.md` として保存**し、`docs/index.html` の該当章末尾にも `<details class="dive-details">` として追加する。
9. **ImGuiのUIは機能ごとに小さなウィジェット関数に分ける**（責務分離）。
10. **各章の完了時に "達成チェックリスト" を提示** して、ユーザーが理解度を自己チェックできるようにする。

---

## 深掘りトピック（執筆済み）

全章分を `docs/deepdive/` に Markdown として保存、`docs/index.html` の各章末尾にも折りたたみ形式で埋め込み済み。

- 00a: ヘッダとコンパイル単位 / 00b: CMake メンタルモデル
- 01: RAII とデストラクタ / 02: ムーブセマンティクス / 03: std::variant vs discriminated union
- 04: concepts vs virtual / 05: スタック vs ヒープ・キャッシュ局所性 / 06: コルーチン vs async/await
- 07: std::expected vs 例外 / 08: std::span と C API 相互運用 / 09: リソース解放順序

---

## メモ

- OpenCV は Homebrew 版がインストール済み（`brew list opencv` で確認）。C++20 で問題なく使える（OpenCV自身の最低要件は C++11 だが、消費側の言語規格は自由）。
- Homebrew LLVM の clang++ を使うのは、Apple Clang だと一部 C++20 機能（特に ranges の一部 / coroutines ライブラリ）が弱いため。
- OSC 疎通確認は Python の `python-osc` ライブラリや Processing などで行える（`osc_examples/` にサンプルを置く予定）。
