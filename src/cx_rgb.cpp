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
    bool IsResetColorType( const cx::Color& color ) noexcept
    {
        return ( std::string { RESET_COLOR_TYPE } == color.hex() );
    }

    bool SetBackColor( const cx::RGB clr ) noexcept
    {
        return SetBackColor( cx::Color { clr } );
    }

    bool SetBackColor( const std::string& hex ) noexcept
    {
        return SetBackColor( cx::Color { hex } );
    }

    bool SetBackColor( const cx::Color& clr ) noexcept
    {
        if( clr == false )
            return false;

        if( clr.hex() == std::string { RESET_COLOR_TYPE } ) {
            cx::ResetColor();
            return true;
        }

        printf("\033[48;2;%d;%d;%dm", clr.rgb().r, clr.rgb().g, clr.rgb().b );
        return true;
    }

    void ResetColor( void ) noexcept
    {
        printf("\033[0m");
    }

    std::ostream& ResetColor( std::ostream& _os ) noexcept
    {
        cx::ResetColor();
        return _os;
    }

    static std::string _GetHexStringFromHexColor( const cx::RGB clr )
    {
        switch ( clr ) {
        case RGB::BLACK:   return std::string { "#000000" };
        case RGB::GRAY:    return std::string { "#808080" };
        case RGB::WHITE:   return std::string { "#FFFFFF" };
        case RGB::SILVER:  return std::string { "#C0C0C0" };
        case RGB::RED:     return std::string { "#FF0000" };
        case RGB::MAROON:  return std::string { "#800000" };
        case RGB::YELLOW:  return std::string { "#FFFF00" };
        case RGB::OLIVE:   return std::string { "#808000" };
        case RGB::LIME:    return std::string { "#00FF00" };
        case RGB::GREEN:   return std::string { "#008000" };
        case RGB::CYAN:    return std::string { "#00FFFF" };
        case RGB::TEAL:    return std::string { "#008080" };
        case RGB::BLUE:    return std::string { "#0000FF" };
        case RGB::NAVY:    return std::string { "#000080" };
        case RGB::MAGENTA: return std::string { "#FF00FF" };
        case RGB::PURPLE:  return std::string { "#800080" };
        case RGB::RESET:   return std::string { RESET_COLOR_TYPE };
        default:           return std::string { INVALLID_RGB_HEX };
        }
    }

    static inline bool _IsValidColorHexString( const std::string& hex )
    {
        if( std::regex_match( hex, std::regex("^#([0-9a-fA-F]{6})$") ) )
            return true;
        else if( hex == std::string{ RESET_COLOR_TYPE } )
            return true;
        return false;
    }

    static inline bool _IsValidRgbRange( const cx::rgb_value& rgb )
    {
        return (rgb.r <= 255) && (rgb.g <= 255) && (rgb.b <= 255);
    }

    static inline unsigned int _GetIntFromHex( const char* hex )
    {
        return (unsigned int)strtol( hex, NULL, 16 );
    }

    static inline std::string _UintToHex( const unsigned int value )
    {
        std::stringstream ss;
        ss << std::hex << std::uppercase << value;
        return ss.str();
    }

    static inline std::string _GetStrFromRGB( const cx::rgb_value rgb )
    {
        std::string res = "#";
        res += cx::_UintToHex( rgb.r );
        res += cx::_UintToHex( rgb.g );
        res += cx::_UintToHex( rgb.b );
        return res;
    }

    static inline cx::rgb_value _GetRgbValueFromHexString( const std::string& hex )
    {
        char r_word[3] = { hex[1], hex[2], '\0' };
        char g_word[3] = { hex[3], hex[4], '\0' };
        char b_word[3] = { hex[5], hex[6], '\0' };

        return cx::rgb_value {
            cx::_GetIntFromHex( r_word ),
            cx::_GetIntFromHex( g_word ),
            cx::_GetIntFromHex( b_word ),
        };
    }

    cx::Color::Color( const std::string& hex )
        : _is_fg    ( true )
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

    cx::Color::Color( const cx::rgb_value& rgb )
        : _is_fg    ( true )
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

    cx::Color::Color( const cx::RGB rgb )
        : _is_fg    ( true )
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

    cx::rgb_value cx::Color::rgb( void ) const noexcept
    {
        return this->_rgb;
    }

    std::string cx::Color::hex( void ) const noexcept
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