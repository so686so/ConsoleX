#include "cx_color.hpp"

#include <iomanip>
#include <sstream>
#include <algorithm> // std::clamp if needed
#include <regex>

namespace cx
{
    // =========================================================================
    // Static Presets Definition
    // =========================================================================
    const Color Color::Black   { 0,   0,   0   };
    const Color Color::White   { 255, 255, 255 };
    const Color Color::Red     { 255, 0,   0   };
    const Color Color::Green   { 0,   255, 0   };
    const Color Color::Blue    { 0,   0,   255 };
    const Color Color::Yellow  { 255, 255, 0   };
    const Color Color::Cyan    { 0,   255, 255 };
    const Color Color::Magenta { 255, 0,   255 };
    const Color Color::Gray    { 128, 128, 128 };
    const Color Color::Reset   { Color::Type::RESET };

    // =========================================================================
    // Implementation
    // =========================================================================

    Color::Color()
        : type_( Type::NONE )
        , rgb_ { 0, 0, 0 }
    {
        //
    }

    Color::Color( uint8_t r, uint8_t g, uint8_t b )
        : type_( Type::RGB )
        , rgb_ { r, g, b }
    {
        //
    }

    Color::Color( Type type )
        : type_( type )
        , rgb_ { 0, 0, 0 }
    {
        //
    }

    Color::Color( const std::string& hex_code )
        : type_( Type::NONE )
        , rgb_ { 0, 0, 0 }
    {
        // 1. 입력 검증 및 전처리 (# 제거)
        std::string hex = hex_code;
        if( hex.empty()   ) return;
        if( hex[0] == '#' ) hex.erase(0, 1);

        // 반드시 6자리여야 함
        if(hex.length() != 6)
            return;

        // 2. 정규식 검사 (0-9, A-F)
        static const std::regex hex_regex("^[0-9a-fA-F]{6}$");
        if( !std::regex_match(hex, hex_regex) )
            return;

        // 3. 파싱 (std::stoi - base 16)
        try {
            int r = std::stoi( hex.substr(0, 2), nullptr, 16 );
            int g = std::stoi( hex.substr(2, 2), nullptr, 16 );
            int b = std::stoi( hex.substr(4, 2), nullptr, 16 );

            this->rgb_ = {
                static_cast<uint8_t>(r),
                static_cast<uint8_t>(g),
                static_cast<uint8_t>(b)
            };
            this->type_ = Type::RGB;
        }
        catch (...) {
            // 파싱 에러 시 NONE 유지
        }
    }

    std::string Color::ToAnsiForeground() const
    {
        if( type_ == Type::RESET ) return "\033[0m"; // Reset All
        if( type_ != Type::RGB   ) return "";        // NONE

        // \033[38;2;R;G;Bm
        return "\033[38;2;" + std::to_string( rgb_.r ) + ";" +
                              std::to_string( rgb_.g ) + ";" +
                              std::to_string( rgb_.b ) + "m";
    }

    std::string Color::ToAnsiBackground() const
    {
        if( type_ == Type::RESET ) return "\033[0m"; // Reset All
        if( type_ != Type::RGB   ) return "";        // NONE

        // \033[48;2;R;G;Bm
        return "\033[48;2;" + std::to_string( rgb_.r ) + ";" +
                              std::to_string( rgb_.g ) + ";" +
                              std::to_string( rgb_.b ) + "m";
    }

    std::string Color::ToHex() const
    {
        if( type_ != Type::RGB )
            return "";

        std::stringstream ss;
        ss << "#"
           << std::hex << std::uppercase << std::setfill('0')
           << std::setw(2) << (int)rgb_.r
           << std::setw(2) << (int)rgb_.g
           << std::setw(2) << (int)rgb_.b;
        return ss.str();
    }

    bool Color::operator==( const Color& other ) const
    {
        if( type_ != other.type_ )
            return false;

        if( type_ == Type::RGB )
            return rgb_ == other.rgb_;

        return true; // 둘 다 RESET이거나 NONE이면 같음
    }

    bool Color::operator!=( const Color& other ) const
    {
        return !(*this == other);
    }

} // namespace cx