// ============================================================
// src/osc_sender.cpp
// ------------------------------------------------------------
// OscSender の実装。自前の minimal_osc.hpp を使う。
//
// oscpack は arm64 (Apple Silicon) で int32 型定義が壊れるため、
// 最小自前実装に切り替えた（詳細: include/media/minimal_osc.hpp）。
//
// std::variant を使った OscValue から type tag 文字列を生成し、
// PacketBuilder で OSC 1.0 準拠のバイト列を組み立て、
// UdpSender で UDP 送信する。
// ============================================================
#include "media/osc_sender.hpp"
#include "media/minimal_osc.hpp"

#include <iostream>
#include <memory>

namespace media {

// ============================================================
// Impl (実装詳細を隠蔽)
// ============================================================
struct OscSender::Impl {
    osc::UdpSender udp;

    explicit Impl(std::string_view host, int port)
        : udp(host, port) {}
};

// ============================================================
// コンストラクタ
// ============================================================
OscSender::OscSender(std::string_view host, int port)
    : host_(host), port_(port) {
    impl_ = std::make_unique<Impl>(host, port);
    connected_ = impl_->udp.is_connected();

    if (!connected_) {
        std::cerr << "[OscSender] ソケット作成失敗: " << host << ":" << port << '\n';
    }
}

// ============================================================
// デストラクタ（PIMPL: unique_ptr が Impl を解放）
// ============================================================
OscSender::~OscSender() = default;

// ============================================================
// send: OSC メッセージを送信
// ============================================================
bool OscSender::send(std::string_view address, std::span<const OscValue> args) {
    if (!connected_ || !impl_) {
        return false;
    }

    // -------------------------------------------------------
    // Type tag 文字列を variant の型から生成する
    // -------------------------------------------------------
    // OSC の type tag は各引数の型をコンパクトに表す 1 文字。
    //   i = int32, f = float32, s = string, T/F = bool (または i で代替)
    // "i" + "f" + "s" → type_tag = "ifs"
    std::string type_tag;
    for (const OscValue& val : args) {
        std::visit([&type_tag](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, int>)         type_tag += 'i';
            else if constexpr (std::is_same_v<T, float>)  type_tag += 'f';
            else if constexpr (std::is_same_v<T, std::string>) type_tag += 's';
            else if constexpr (std::is_same_v<T, bool>)   type_tag += 'i'; // bool → i で送信
        }, val);
    }

    // -------------------------------------------------------
    // OSC バイト列の組み立て
    // -------------------------------------------------------
    osc::PacketBuilder builder(address, type_tag);

    for (const OscValue& val : args) {
        std::visit([&builder](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, int>)              builder.add_int(v);
            else if constexpr (std::is_same_v<T, float>)       builder.add_float(v);
            else if constexpr (std::is_same_v<T, std::string>) builder.add_string(v);
            else if constexpr (std::is_same_v<T, bool>)        builder.add_bool(v);
        }, val);
    }

    // -------------------------------------------------------
    // UDP 送信
    // -------------------------------------------------------
    return impl_->udp.send(builder.data());
}

// ============================================================
// is_connected
// ============================================================
bool OscSender::is_connected() const {
    return connected_;
}

} // namespace media
