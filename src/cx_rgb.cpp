// ConsoleX
#include "cx_rgb.h"
// STL::C
#include <stdio.h>  // printf
#include <stdlib.h> // strtol
// STL::C++
#include <regex>    // regex, regex_match
#include <sstream>  // stringstream
#include <iomanip>

namespace cx // ConsoleX main namespace
{
    // 사용할 함수와 클래스를 명확히 특정해 using 사용.
    // namespace 전체를 using 하는 것은 추천하지 않음.
    using std::string;

    // local scope 변수 할당을 위한 이름 없는 namespace
    namespace
    {
        // const value :: Error & Reset color hex string
        constexpr const char* INVALLID_RGB_HEX = "INVALID_RGB";
        constexpr const char* RESET_COLOR_TYPE = "RESET_COLOR";
    }

    bool IsResetColorType( const cx::Color& color ) noexcept
    {
        return ( string { RESET_COLOR_TYPE } == color.hex() );
    }

    void ResetColor( void ) noexcept
    {
        printf("\033[0m");
    }

    // cx::RGB enum value -> Hex Code std::string
    static string _GetHexStringFromHexColor( const cx::RGB clr )
    {
        switch ( clr ) {
        case RGB::BLACK:   return string { "#000000" };
        case RGB::GRAY:    return string { "#808080" };
        case RGB::WHITE:   return string { "#FFFFFF" };
        case RGB::SILVER:  return string { "#C0C0C0" };
        case RGB::RED:     return string { "#FF0000" };
        case RGB::MAROON:  return string { "#800000" };
        case RGB::YELLOW:  return string { "#FFFF00" };
        case RGB::OLIVE:   return string { "#808000" };
        case RGB::LIME:    return string { "#00FF00" };
        case RGB::GREEN:   return string { "#008000" };
        case RGB::CYAN:    return string { "#00FFFF" };
        case RGB::TEAL:    return string { "#008080" };
        case RGB::BLUE:    return string { "#0000FF" };
        case RGB::NAVY:    return string { "#000080" };
        case RGB::MAGENTA: return string { "#FF00FF" };
        case RGB::PURPLE:  return string { "#800080" };
        case RGB::RESET:   return string { RESET_COLOR_TYPE };
        default:           return string { INVALLID_RGB_HEX };
        }
    }

    // 주어진 문자열이 HexCode 타입인지 체크
    static inline bool _IsValidColorHexString( const string& hex )
    {
        // 정규표현식을 이용한 HexCode Check
        if( std::regex_match( hex, std::regex("^#([0-9a-fA-F]{6})$") ) )
            return true;
        // ResetCode type 또한 return true
        else if( hex == string{ RESET_COLOR_TYPE } )
            return true;
        return false;
    }

    // 주어진 cx::rgb_set 인자들이 유효값인지 체크
    static inline bool _IsValidRgbRange( const cx::rgb_set& rgb )
    {
        return
            (rgb.r >= 0 && rgb.r <= 255) &&
            (rgb.g >= 0 && rgb.g <= 255) &&
            (rgb.b >= 0 && rgb.b <= 255);
    }

    // 주어진 문자열을 숫자로 변환
    static inline rgb_value _GetRgbValueFromString( const char* hex )
    {
        return (rgb_value)strtol( hex, NULL, 16 );
    }

    // 숫자값을 16진수로 변환
    static inline string _UintToHex( const unsigned int value )
    {
        std::stringstream ss;
        ss << std::hex << std::uppercase << value;
        return ss.str();
    }

    // cx::rgb_set 값을 HexCode 문자열로 변환
    static inline string _GetStrFromRGB( const cx::rgb_set rgb )
    {
        string res = "#";
        res += cx::_UintToHex( rgb.r );
        res += cx::_UintToHex( rgb.g );
        res += cx::_UintToHex( rgb.b );
        return res;
    }

    // HexCode 문자열을 개별 r,g,b 값으로 분해
    static inline cx::rgb_set _GetRgbValueFromHexString( const string& hex )
    {
        char r_word[3] = { hex[1], hex[2], '\0' };
        char g_word[3] = { hex[3], hex[4], '\0' };
        char b_word[3] = { hex[5], hex[6], '\0' };

        return cx::rgb_set {
            cx::_GetRgbValueFromString( r_word ),
            cx::_GetRgbValueFromString( g_word ),
            cx::_GetRgbValueFromString( b_word ),
        };
    }

    cx::Color::Color( const string& hex, const bool is_fg )
        : _is_fg    ( is_fg )
        , _is_valid ( cx::_IsValidColorHexString( hex ) )
    {
        if( _is_valid ) {
            _hex = hex;
            _rgb = cx::_GetRgbValueFromHexString( hex );
        }
        else {
            _hex = INVALLID_RGB_HEX;
        }
    }

    cx::Color::Color( const cx::rgb_set& rgb, const bool is_fg )
        : _is_fg    ( is_fg )
        , _is_valid ( _IsValidRgbRange( rgb ) )
    {
        if( _is_valid ) {
            _hex = cx::_GetStrFromRGB( rgb );
            _rgb = rgb;
        }
        else {
            _hex = INVALLID_RGB_HEX;
        }
    }

    cx::Color::Color( const cx::RGB rgb, const bool is_fg )
        : _is_fg    ( is_fg )
        , _is_valid ( cx::_IsValidColorHexString( cx::_GetHexStringFromHexColor( rgb ) ) )
    {
        if( _is_valid ) {
            _hex = cx::_GetHexStringFromHexColor( rgb );
            _rgb = cx::_GetRgbValueFromHexString( _hex );
        }
        else {
            _hex = INVALLID_RGB_HEX;
        }
    }

    cx::Color::Color( const cx::Color& color, const bool is_fg )
        : _is_fg    ( is_fg )
        , _is_valid ( ( color ) ? true : false )
        , _hex      ( color.hex() )
        , _rgb      ( color.rgb() )
    {}

    cx::rgb_set cx::Color::rgb( void ) const noexcept
    {
        return this->_rgb;
    }

    string cx::Color::hex( void ) const noexcept
    {
        return this->_hex;
    }

    bool cx::Color::operator==( const cx::Color& other ) const
    {
        return ( this->hex() == other.hex() );
    }

    bool cx::Color::operator!=( const cx::Color& other ) const
    {
        return ( this->hex() != other.hex() );
    }

    cx::Color& cx::Color::operator=( const cx::Color& other )
    {
        if( this != &other ) {
            this->_is_fg    = other._is_fg;
            this->_is_valid = other._is_valid;
            this->_hex      = other._hex;
            this->_rgb      = other._rgb;
        }
        return *this;
    }

} // nsp::cx