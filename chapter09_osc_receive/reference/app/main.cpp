// ============================================================
// app/main.cpp
// ------------------------------------------------------------
// Chapter 09: OSC 受信 + ImGui の完成形アプリ。
//
// ImGui が使用可能な場合: GLFW + ImGui ウィンドウ付き
// 使用不可な場合: コンソールのみのフォールバック
//
// 機能:
//   - カメラプレビュー (OpenCV imshow)
//   - ImGui スライダーで HSV 閾値をリアルタイム調整
//   - OSC 受信ログ表示
//   - FPS 表示
// ============================================================
#include "media/camera_capture.hpp"
#include "media/color_tracker.hpp"
#include "media/config.hpp"
#include "media/osc_sender.hpp"
#include "media/osc_receiver.hpp"
#include "media/osc_value.hpp"
#include "media/ui_state.hpp"

#include <chrono>
#include <iostream>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// ImGui が使えるかどうかで条件コンパイル
#ifdef MODERN_CPP_WITH_IMGUI
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#endif

// ============================================================
// ImGui ウィジェット関数（責務分離）
// ============================================================
#ifdef MODERN_CPP_WITH_IMGUI

// HSV スライダーウィジェット
static void widget_hsv_sliders(media::UiState& state) {
    ImGui::Begin("HSV 設定");

    auto range = state.get_hsv_range();

    // lower の各チャンネルをスライダーで調整
    float lower[3] = {
        static_cast<float>(range.lower[0]),
        static_cast<float>(range.lower[1]),
        static_cast<float>(range.lower[2])
    };
    float upper[3] = {
        static_cast<float>(range.upper[0]),
        static_cast<float>(range.upper[1]),
        static_cast<float>(range.upper[2])
    };

    bool changed = false;
    ImGui::Text("下限 (H/S/V)");
    changed |= ImGui::SliderFloat("H (lower)", &lower[0], 0, 179);
    changed |= ImGui::SliderFloat("S (lower)", &lower[1], 0, 255);
    changed |= ImGui::SliderFloat("V (lower)", &lower[2], 0, 255);

    ImGui::Separator();
    ImGui::Text("上限 (H/S/V)");
    changed |= ImGui::SliderFloat("H (upper)", &upper[0], 0, 179);
    changed |= ImGui::SliderFloat("S (upper)", &upper[1], 0, 255);
    changed |= ImGui::SliderFloat("V (upper)", &upper[2], 0, 255);

    if (changed) {
        state.set_hsv_range(media::HsvRange{
            cv::Scalar(lower[0], lower[1], lower[2]),
            cv::Scalar(upper[0], upper[1], upper[2])
        });
    }

    ImGui::End();
}

// OSC ログウィジェット
static void widget_osc_log(media::UiState& state) {
    ImGui::Begin("OSC ログ");

    if (ImGui::Button("クリア")) {
        state.clear_logs();
    }

    ImGui::BeginChild("ログリスト");
    auto logs = state.get_logs();
    for (const auto& log : logs) {
        ImGui::TextUnformatted(log.c_str());
    }
    // 自動スクロール
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0F);
    }
    ImGui::EndChild();

    ImGui::End();
}

// FPS / ステータスウィジェット
static void widget_status(media::UiState& state) {
    ImGui::Begin("ステータス");
    ImGui::Text("FPS: %.1f", static_cast<double>(state.get_fps()));
    ImGui::End();
}

#endif // MODERN_CPP_WITH_IMGUI

// ============================================================
// main
// ============================================================
int main(int argc, char** argv) {
    // 設定ファイルの読み込み
    const std::string config_path = (argc > 1) ? argv[1] : "config.toml";
    auto config_result = media::load_config(config_path);
    media::Config config = config_result.value_or(media::make_default_config());

    // 共有状態
    media::UiState ui_state;
    ui_state.set_hsv_range(config.hsv);

    // OSC 送信
    media::OscSender osc_sender(config.osc_host, config.osc_port);

    // OSC 受信（Max から HSV 設定を受け取る）
    media::OscReceiver osc_receiver(9001); // 受信ポートは別ポート
    osc_receiver.on_message([&ui_state](std::string_view addr,
                                         const std::vector<media::OscValue>& args) {
        const std::string log = std::string(addr) + " (" +
                                std::to_string(args.size()) + " args)";
        ui_state.add_log(log);
    });
    osc_receiver.start();

    // カメラ
    media::CameraCapture cam;
    if (!cam.open(config.camera_id)) {
        std::cerr << "[エラー] カメラ " << config.camera_id << " を開けませんでした。\n";
        return 1;
    }

    media::ColorTracker tracker{config.hsv};

    // HSV 変更時にトラッカーを更新
    ui_state.on_change([&tracker, &ui_state]() {
        tracker.set_range(ui_state.get_hsv_range());
    });

#ifdef MODERN_CPP_WITH_IMGUI
    // -------------------------------------------------------
    // ImGui 初期化
    // -------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "[エラー] GLFW 初期化失敗\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Chapter 09 - OSC + ImGui", nullptr, nullptr);
    if (!window) {
        std::cerr << "[エラー] GLFW ウィンドウ作成失敗\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-sync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::cout << "[INFO] ImGui ウィンドウ起動。ウィンドウを閉じると終了。\n";

    auto last_time = std::chrono::steady_clock::now();
    int frame_count = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // カメラフレーム取得
        auto frame = cam.read_frame();
        if (frame.has_value()) {
            cv::Mat display = frame->clone();
            auto result = tracker.process(*frame);

            if (result.has_value()) {
                if (const auto* tr = std::get_if<media::TrackResult>(&result.value())) {
                    cv::circle(display, tr->centroid, 20, cv::Scalar(0, 255, 0), 3);

                    std::vector<media::OscValue> args{
                        tr->centroid.x, tr->centroid.y,
                        static_cast<float>(tr->area),
                    };
                    [[maybe_unused]] bool _ = osc_sender.send("/tracker/centroid", args);
                }
            }

            cv::imshow("Camera", display);
        }

        // FPS 計算
        ++frame_count;
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - last_time).count();
        if (elapsed >= 1.0F) {
            ui_state.set_fps(static_cast<float>(frame_count) / elapsed);
            frame_count = 0;
            last_time = now;
        }

        // ImGui フレーム
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        widget_hsv_sliders(ui_state);
        widget_osc_log(ui_state);
        widget_status(ui_state);

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        // OpenCV ウィンドウのキー入力処理
        const int key = cv::waitKey(1) & 0xFF;
        if (key == 'q' || key == 27) break;
    }

    // リソース解放（順序が重要）
    // 1. ImGui バックエンドを先にシャットダウン
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // 2. GLFW を終了
    glfwDestroyWindow(window);
    glfwTerminate();
    // 3. OpenCV ウィンドウを閉じる
    cv::destroyAllWindows();

#else
    // ImGui なしのフォールバック: コンソールのみ
    std::cout << "[INFO] ImGui が無効です。コンソールモードで動作。'q' で終了。\n";
    std::cout << "[INFO] brew install glfw && cmake で ImGui を有効化できます。\n";

    while (true) {
        auto frame = cam.read_frame();
        if (!frame.has_value()) break;

        cv::Mat display = frame->clone();
        auto result = tracker.process(*frame);

        if (result.has_value()) {
            if (const auto* tr = std::get_if<media::TrackResult>(&result.value())) {
                cv::circle(display, tr->centroid, 20, cv::Scalar(0, 255, 0), 3);
                std::vector<media::OscValue> args{
                    tr->centroid.x, tr->centroid.y,
                    static_cast<float>(tr->area),
                };
                [[maybe_unused]] bool _ = osc_sender.send("/tracker/centroid", args);
            }
        }

        cv::imshow("Chapter 09 - Console Mode", display);
        const int key = cv::waitKey(30) & 0xFF;
        if (key == 'q' || key == 27) break;
    }

    cv::destroyAllWindows();
#endif // MODERN_CPP_WITH_IMGUI

    osc_receiver.stop();
    return 0;
}
