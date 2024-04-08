#ifndef _CONSOLE_X_RGB_H_
#define _CONSOLE_X_RGB_H_

// STL::C++
#include <string>      // std::string
#include <iostream>    // for 'std::cout <<' overloading
#include <type_traits> // is_same

namespace cx // ConsoleX main namespace
{
    // Enum class about default rgb hex color codes
    enum class RGB {
        BLACK,   // #000000
        GRAY,    // #808080
        SILVER,  // #C0C0C0
        WHITE,   // #FFFFFF
        RED,     // #FF0000
        MAROON,  // #800000
        YELLOW,  // #FFFF00
        OLIVE,   // #808000
        LIME,    // #00FF00
        GREEN,   // #008000
        CYAN,    // #00FFFF
        TEAL,    // #008080
        BLUE,    // #0000FF
        NAVY,    // #000080
        MAGENTA, // #FF00FF
        PURPLE,  // #800080
        RESET,   // Special RGB type for reset
        NONE,    // Invalid color type
    };

    constexpr bool FG = true;  // foreground(font)
    constexpr bool BG = false; // background(back)

    // R, G, B individual value expression of RGB hex value
    using rgb_value = unsigned short;
    typedef struct _rgb_s_ {
        rgb_value r = 0;
        rgb_value g = 0;
        rgb_value b = 0;
    } rgb_set;

    // Set foreground color - Used separately from std::cout
    template <typename T>
    bool SetFontColor( const T& value ) noexcept;

    // Set background color - Used separately from std::cout
    template <typename T>
    bool SetBackColor( const T& value ) noexcept;

    // Reset color - Used separately from std::cout
    void ResetColor( void ) noexcept;

    /**
     * @brief
     *    ConsoleX color class
     *
     * @note
     *    All console color inside ConsoleX
     *    must use the coressponding class
     */
    class Color {
    public:
        // Constructors
        Color( void ) = delete; // cx::Color class must have real color value
        Color( const std::string& hex, const bool is_fg = true );
        Color( const cx::rgb_set& rgb, const bool is_fg = true );
        Color( const cx::Color& color, const bool is_fg = true );
        Color( const cx::RGB rgb = cx::RGB::RESET, const bool is_fg = true );

        // Destructor
        virtual ~Color() = default;

        // Getter
        cx::rgb_set rgb( void ) const noexcept;
        std::string hex( void ) const noexcept;
        std::string str( void ) const noexcept { return this->hex(); };

        // is
        bool is_font_color( void ) const noexcept { return  this->_is_fg; };
        bool is_back_color( void ) const noexcept { return !this->_is_fg; };

        // check current cx::Color instance is valid
        operator bool() const { return this->_is_valid; };

        // Compare operator
        bool operator==( const cx::Color& other ) const;
        bool operator!=( const cx::Color& other ) const;

        // std::cout << overloading
        friend std::ostream& operator<< ( std::ostream& os, const cx::Color& color ) {
            if( color.is_font_color() )
                cx::SetFontColor( color );
            else
                cx::SetBackColor( color );
            return os;
        }

        // copy assignment
        cx::Color& operator=( const cx::Color& other );

    protected:
        bool _is_fg = true;    // is foreground(font) color

    private:
        bool        _is_valid; // is valid color instance
        std::string _hex;      // hex string
        cx::rgb_set _rgb;      // rgb values

    }; // cls::Color
    
    // typedef for type check
    typedef char HEX_RGB_CHAR[8];

    template <typename T>
    cx::Color FontColor( const T& value ) noexcept
    {
        if constexpr ( std::is_same<T, cx::RGB>::value ) {
            return cx::Color { value, true };
        }
        else if constexpr ( std::is_same<T, std::string>::value ) {
            return cx::Color { value, true };
        }
        else if constexpr ( std::is_same<T, HEX_RGB_CHAR>::value ) {
            return cx::Color { std::string{ value }, true };
        }
        else if constexpr ( std::is_same<T, cx::Color>::value ) {
            return cx::Color { value, true };
        }
        return cx::Color { cx::RGB::NONE, true };
    }

    template <typename T>
    cx::Color BackColor( const T& value ) noexcept
    {
        if constexpr ( std::is_same<T, cx::RGB>::value ) {
            return cx::Color { value, false };
        }
        else if constexpr ( std::is_same<T, std::string>::value ) {
            return cx::Color { value, false };
        }
        else if constexpr ( std::is_same<T, HEX_RGB_CHAR>::value ) {
            return cx::Color { std::string{ value }, false };
        }
        else if constexpr ( std::is_same<T, cx::Color>::value ) {
            return cx::Color { value, false };
        }
        return cx::Color { cx::RGB::NONE, false };
    }

    template <typename T>
    inline bool SetColor( const T& value, const bool is_foregournd = true ) noexcept
    {
        if constexpr ( std::is_same<T, cx::RGB>::value ) {
            return SetColor( cx::Color { value }, is_foregournd );
        }
        else if constexpr ( std::is_same<T, std::string>::value ) {
            return SetColor( cx::Color { value }, is_foregournd );
        }
        else if constexpr ( std::is_same<T, HEX_RGB_CHAR>::value ) {
            return SetColor( std::string { value }, is_foregournd );
        }
        else if constexpr ( std::is_same<T, cx::Color>::value ) {
            return SetColor( value, is_foregournd );
        }
        return false;
    }

    // Check Reset Color Type
    bool IsResetColorType( const cx::Color& color ) noexcept;

    template <>
    inline bool SetColor<cx::Color>( const cx::Color& color, const bool is_foregournd ) noexcept
    {
        if( !color ) return false;

        if( cx::IsResetColorType( color ) ) {
            cx::ResetColor();
            return true;
        }

        if( is_foregournd )
            printf("\033[38;2;%d;%d;%dm", color.rgb().r, color.rgb().g, color.rgb().b );
        else
            printf("\033[48;2;%d;%d;%dm", color.rgb().r, color.rgb().g, color.rgb().b );

        return true;
    }

    template <typename T>
    inline bool SetFontColor( const T& value ) noexcept
    {
        return cx::SetColor( value, true );
    }

    template <typename T>
    inline bool SetBackColor( const T& value ) noexcept
    {
        return cx::SetColor( value, false );
    }

} // nsp::cx

#endif