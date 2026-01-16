#ifndef _CONSOLE_X_COLOR_HPP_
#define _CONSOLE_X_COLOR_HPP_


/** ------------------------------------------------------------------------------------
 *  ConsoleX Color Module
 *  ------------------------------------------------------------------------------------
 *  RGB 색상 데이터를 관리하고 ANSI Escape Code 변환을 담당합니다.
 *  ------------------------------------------------------------------------------------ */

#include <string>
#include <cstdint> // uint8_t

namespace cx
{
    // RGB 값을 담는 단순 구조체
    struct Rgb
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        bool operator==( const Rgb& other ) const { return r == other.r && g == other.g && b == other.b; }
        bool operator!=( const Rgb& other ) const { return !(*this == other); }
    };

    // 색상 관리 클래스
    class Color
    {
    public:
        // 색상 타입 (일반 RGB, 터미널 기본값 복구, 없음)
        enum class Type { RGB, RESET, NONE };

    public:
        // 기본 생성자 (Type::NONE - 투명/무시)
        Color();

        // RGB 값으로 생성 (0~255)
        Color( uint8_t r, uint8_t g, uint8_t b );

        // Hex 문자열로 생성 (예: "#FF0000" or "FF0000")
        /// @details 파싱 실패 시 Type::NONE 상태가 됨
        Color( const std::string& hex_code );

        // 특수 타입 생성 (주로 Color::Reset() 내부 사용)
        Color( Type type );

        // 전경색(글자색)용 ANSI 시퀀스 반환 (예: "\033[38;2;R;G;Bm")
        std::string ToAnsiForeground() const;

        // 배경색용 ANSI 시퀀스 반환 (예: "\033[48;2;R;G;Bm")
        std::string ToAnsiBackground() const;

        // Hex 문자열 반환 (예: "#RRGGBB")
        std::string ToHex() const;

        // 유효한 색상인지 확인 (RESET 포함)
        bool IsValid() const { return type_ != Type::NONE; }

        // RGB 타입인지 확인
        bool IsRgb() const { return type_ == Type::RGB; }

        Rgb GetRgb() const { return rgb_; }

        bool operator==( const Color& other ) const;
        bool operator!=( const Color& other ) const;

        // --- Static Presets (자주 쓰는 색상) ---
        static const Color Black;
        static const Color White;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color Cyan;
        static const Color Magenta;
        static const Color Gray;

        // 터미널 색상을 초기화하는 특수 객체 (\033[0m)
        static const Color Reset;

    private:
        Type type_ = Type::NONE;
        Rgb  rgb_  = { 0, 0, 0 };
    };

} // namespace cx

#endif // _CONSOLE_X_COLOR_HPP_