#ifndef _CONSOLE_X_RGB_H_
#define _CONSOLE_X_RGB_H_

// STL::C++
#include <string>      // std::string
#include <iostream>    // for 'std::cout <<' overloading
#include <type_traits> // is_same_v

namespace cx // ConsoleX main namespace
{
    // const value :: Error & Reset color hex string
    constexpr const char* INVALLID_RGB_HEX = "INVALID_RGB";
    constexpr const char* RESET_COLOR_TYPE = "RESET_COLOR";

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
    };

    // R, G, B individual value expression of RGB hex value
    typedef struct _rgb_v_ {
        unsigned int r = 0;
        unsigned int g = 0;
        unsigned int b = 0;
    } rgb_value;

    // Set foreground color - Used separately from std::cout
    template <typename T>
    bool SetFontColor( const T& value ) noexcept;

    // Set background color - Used separately from std::cout
    template <typename T>
    bool SetBackColor( const T& value ) noexcept;

    // Reset color - Used separately from std::cout
    void ResetColor( void ) noexcept;

    // Reset color - Use with std::cout
    std::ostream& ResetColor( std::ostream& _os ) noexcept;

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
        Color( void ) = delete; // color class must have color value
        Color( const std::string& hex );
        Color( const cx::rgb_value& rgb );
        Color( const cx::RGB rgb = cx::RGB::RESET );

        // Destructor
        ~Color() = default;

        // getter
        cx::rgb_value rgb( void ) const noexcept;
        std::string   hex( void ) const noexcept;
        std::string   str( void ) const noexcept { return this->hex(); };

        // is
        bool is_font_color( void ) const noexcept { return  this->_is_fg; };
        bool is_back_color( void ) const noexcept { return !this->_is_fg; };

        // check current cx::Color instance is valid
        operator bool() const { return this->_is_valid; };

        // compare operator
        bool operator==( const cx::Color& other ) const;
        bool operator!=( const cx::Color& other ) const;

        // std::cout << overloading
        friend std::ostream& operator<< ( std::ostream& os, const cx::Color& color ) {
            cx::SetFontColor( color.hex() );
            return os;
        }

        // copy assignment
        cx::Color& operator=( const cx::Color& other );

    protected:
        bool _is_fg = true; // is foreground(font) color

    private:
        bool          _is_valid; // is valid color instance
        std::string   _hex; // hex string
        cx::rgb_value _rgb; // rgb values
    };

    // Check Reset Color Type
    bool IsResetColorType( const cx::Color& color ) noexcept;

    template <typename T>
    inline bool SetColor( const T& value, const bool is_foregournd = true ) noexcept
    {
        if constexpr ( std::is_same_v<T, cx::RGB> || std::is_same_v<T, std::string> ) {
            return SetColor( cx::Color { value }, is_foregournd );
        }
        return false;
    }

    template <>
    inline bool SetColor<cx::Color>( const cx::Color& color, const bool is_foregournd ) noexcept
    {
        if( !color ) return false;

        if( IsResetColorType( color ) ) {
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


    // class FontColor final : public cx::Color
    // {

    // };

    // class BackColor final : public cx::Color
    // {

    // };

} // nsp::cx

#endif