#ifndef _CONSOLE_X_RGB_H_
#define _CONSOLE_X_RGB_H_

// STL::C++
#include <string>      // std::string
#include <iostream>    // for 'std::cout <<' overloading
#include <type_traits> // is_same

namespace cx // ConsoleX main namespace
{
    // 표준 RGB 컬러에 대한 enum class
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

    // 특정한 boolean 값에 대한 이름 붙이기
    constexpr bool FG = true;  // foreground(font)
    constexpr bool BG = false; // background(back)

    // RGB 헥스 코드 값에서 개별 R, G, B에 대한 16진수 값 구조체
    using rgb_value = int;
    typedef struct _rgb_s_ {
        rgb_value r = -1;
        rgb_value g = -1;
        rgb_value b = -1;
    } rgb_set;

    // 함수 선언을 위한 cx::Color 전방 선언
    class Color;

    /**
     * @brief
     *    폰트 색상 세팅용 cx::Color 클래스를 반환하는 함수.
     *
     * @param value
     *    cx::Color 값으로 변환 가능한 템플릿 클래스.
     *
     * @return
     *    유효한 value 값 입력 시,
     *    폰트 색상 세팅 cx::Color 객체 반환.
     */
    template <typename T>
    cx::Color FontColor( const T& value ) noexcept;

    /**
     * @brief
     *    배경 색상 세팅용 cx::Color 클래스를 반환하는 함수.
     *
     * @param value
     *    cx::Color 값으로 변환 가능한 템플릿 클래스.
     *
     * @return
     *    유효한 value 값 입력 시,
     *    배경 색상 세팅 cx::Color 객체 반환.
     */
    template <typename T>
    cx::Color BackColor( const T& value ) noexcept;

    /**
     * @brief
     *    폰트 색상을 바꾸는 템플릿 함수.
     *    해당 함수는 std::cout과 독립적으로 사용.
     *
     * @param value
     *    cx::Color 값으로 변환 가능한 템플릿 클래스.
     *
     * @return
     *    폰트 색상 변환 성공 시 return true
     *
     * @example
     *    cx::SetFontColor("#00FF00");
     *    std::cout << "Hello world" << std::endl;
     */
    template <typename T>
    inline bool SetFontColor( const T& value ) noexcept;

    /**
     * @brief
     *    배경 색상을 바꾸는 템플릿 함수.
     *    해당 함수는 std::cout과 독립적으로 사용.
     *
     * @param value
     *    cx::Color 값으로 변환 가능한 템플릿 클래스.
     *
     * @return
     *    배경 색상 변환 성공 시 return true
     *
     * @example
     *    cx::SetBackColor("#00FF00");
     *    std::cout << "Hello world" << std::endl;
     */
    template <typename T>
    inline bool SetBackColor( const T& value ) noexcept;

    /**
     * @brief
     *    콘솔 폰트 및 배경 색상을 초기화하는 함수.
     *    해당 함수는 std::cout과 독립적으로 사용.
     *
     * @example
     *    cx::ResetColor();
     *    std::cout << "Hello world" << std::endl;
     */
    void ResetColor( void ) noexcept;

    /**
     * @brief
     *    해당 cx::Color 객체가 색상 초기화 값인지 판단하는 함수.
     *
     * @return
     *    해당 값을 cx::SetColor() 했을 때 색상 초기화가
     *    되는 값이라면 return true
    */
    bool IsResetColorType( const cx::Color& color ) noexcept;

    /**
     * @brief
     *    ConsoleX 색상 클래스
     *
     * @note
     *    ConsoleX 라이브러리의 모든 색상 처리는
     *    반드시 해당 클래스를 이용.
     */
    class Color {
    public:
        // 기본 생성자 삭제 - cx::Color 객체는 반드시 유효한 실제 색상을 가지고 있어야 함
        Color( void ) = delete;

        Color( const std::string& hex, const bool is_fg = true );
        Color( const cx::rgb_set& rgb, const bool is_fg = true );
        Color( const cx::Color& color, const bool is_fg = true );
        Color( const cx::RGB rgb = cx::RGB::RESET, const bool is_fg = true );

        // 소멸자: default
        ~Color() = default;

        /**
         * @brief
         *    해당 객체의 색상 속성 값을 cx::rgb_set 형태로 반환
         *
         * @return
         *    만약 유효한 색상 값이라면,
         *    각 r g b 값이 0 ~ 255 범위로 할당된 cx::rgb_set 구조체 반환
         *    만약 유효하지 않은 객체라면,
         *    모든 인자값이 -1 인 cx::rgb_set 구조체 반환
         */
        cx::rgb_set rgb( void ) const noexcept;

        /**
         * @brief
         *    해당 객체의 색상 속성 값을 std::string 형태로 반환
         *
         * @return
         *    만약 유효한 색상 값이라면,
         *    헥스 코드 형태의 문자열( 예시 : "#FF00AB") std::string 값 반환.
         *    만약 유효하지 않은 객체라면, "INVALID_RGB" std::string 값 반환.
         */
        std::string hex( void ) const noexcept;

        /**
         * @brief
         *    해당 객체의 색상 속성 값을 std::string 형태로 반환
         *
         * @return
         *    만약 유효한 색상 값이라면,
         *    헥스 코드 형태의 문자열( 예시 : "#FF00AB") std::string 값 반환.
         *    만약 유효하지 않은 객체라면, "INVALID_RGB" std::string 값 반환.
         */
        std::string str( void ) const noexcept { return this->hex(); };

        // 해당 객체의 색상 값이 폰트 색상 값인지 확인하는 함수.
        // cx::Color 객체의 기본 색상 세팅 값은 폰트 색상.
        bool is_font_color( void ) const noexcept { return  (this->_is_fg); };

        // 해당 객체의 색상 값이 배경 색상 값인지 확인하는 함수.
        bool is_back_color( void ) const noexcept { return !(this->_is_fg); };

        /**
         * @brief
         *   현재 cx::Color 객체가 유효한 객체인지 판단하는 연산자
         *
         * @example
         *    if( color ) { ...; } 식으로 사용하면 됨
         */
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

    // nameless namespace for local scope variables
    namespace {
        typedef char HEX_RGB_CHAR[8]; // typedef for type check
    }

    template <typename T>
    cx::Color FontColor( const T& value ) noexcept
    {
        // value 의 타입이 cx::RGB 일 때
        if constexpr ( std::is_same<T, cx::RGB>::value )
        {
            return cx::Color { value, true };
        }
        // value 의 타입이 cx::rgb_set 일 때
        else if constexpr ( std::is_same<T, cx::rgb_set>::value )
        {
            return cx::Color { value, true };
        }
        // value 의 타입이 std::string 일 때
        else if constexpr ( std::is_same<T, std::string>::value )
        {
            return cx::Color { value, true };
        }
        // value 의 타입이 const char[8] 일 때
        else if constexpr ( std::is_same<T, HEX_RGB_CHAR>::value )
        {
            return cx::Color { std::string{ value }, true };
        }
        // value 의 타입이 cx::Color 일 때
        else if constexpr ( std::is_same<T, cx::Color>::value )
        {
            return cx::Color { value, true };
        }
        // 그 이외의 타입일 때
        return cx::Color { cx::RGB::NONE, true };
    }

    template <typename T>
    cx::Color BackColor( const T& value ) noexcept
    {
        // value 의 타입이 cx::RGB 일 때
        if constexpr ( std::is_same<T, cx::RGB>::value )
        {
            return cx::Color { value, false };
        }
        // value 의 타입이 cx::rgb_set 일 때
        else if constexpr ( std::is_same<T, cx::rgb_set>::value )
        {
            return cx::Color { value, true };
        }
        // value 의 타입이 std::string 일 때
        else if constexpr ( std::is_same<T, std::string>::value )
        {
            return cx::Color { value, false };
        }
        // value 의 타입이 const char[8] 일 때
        else if constexpr ( std::is_same<T, HEX_RGB_CHAR>::value )
        {
            return cx::Color { std::string{ value }, false };
        }
        // value 의 타입이 cx::Color 일 때
        else if constexpr ( std::is_same<T, cx::Color>::value )
        {
            return cx::Color { value, false };
        }
        // 그 이외의 타입일 때
        return cx::Color { cx::RGB::NONE, false };
    }

    // 색상을 설정하는 템플릿 기본 함수
    template <typename T>
    inline bool _SetColor( const T& value, const bool is_foregournd = true ) noexcept
    {
        // STEP 1 :
        //    cx::Color 객체가 아니지만 cx::Color 객체를 생성할 수 있는 값이
        //    들어온다면, 위의 네 경우 조건문을 통해 cx::Color 생성 과정 후
        //    else if ( std::is_same<T, cx::Color>::value ) 조건 부분으로 진입.
        if constexpr ( std::is_same<T, cx::RGB>::value )
        {
            return _SetColor( cx::Color { value }, is_foregournd );
        }
        else if constexpr ( std::is_same<T, cx::rgb_set>::value )
        {
            return _SetColor( cx::Color { value }, is_foregournd );
        }
        else if constexpr ( std::is_same<T, std::string>::value )
        {
            return _SetColor( cx::Color { value }, is_foregournd );
        }
        else if constexpr ( std::is_same<T, HEX_RGB_CHAR>::value )
        {
            return _SetColor( std::string { value }, is_foregournd );
        }

        // STEP 2 :
        //    value 값이 cx::Color 객체거나 `STEP 1` 과정을 통해 cx::Color 객체가
        //    생성된 뒤 해당 템플릿 특수화 함수 부분 진입.
        else if constexpr ( std::is_same<T, cx::Color>::value ) {
            return _SetColor( value, is_foregournd );
        }

        // Others
        return false;
    }

    // 색상을 설정하는 템플릿 특수화 함수
    template <>
    inline bool _SetColor<cx::Color>( const cx::Color& color, const bool is_foregournd ) noexcept
    {
        // 유효하지 않은 객체라면 false 반환 후 종료
        if( !color ) return false;

        // 초기화 값 색상이라면 모든 색상 세팅값 초기화
        if( cx::IsResetColorType( color ) ) {
            cx::ResetColor();
            return true;
        }
        // 실제 색상 세팅 부분
        if( is_foregournd )
            printf("\033[38;2;%d;%d;%dm", color.rgb().r, color.rgb().g, color.rgb().b );
        else
            printf("\033[48;2;%d;%d;%dm", color.rgb().r, color.rgb().g, color.rgb().b );

        return true;
    }

    template <typename T>
    inline bool SetFontColor( const T& value ) noexcept
    {
        return cx::_SetColor( value, true );
    }

    template <typename T>
    inline bool SetBackColor( const T& value ) noexcept
    {
        return cx::_SetColor( value, false );
    }

} // nsp::cx

#endif