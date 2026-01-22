#ifndef _CONSOLE_X_SCREEN_HPP_
#define _CONSOLE_X_SCREEN_HPP_

/** ------------------------------------------------------------------------------------
 *  ConsoleX Screen & Geometry Module
 *  ------------------------------------------------------------------------------------
 *  콘솔 화면의 좌표계 정의, 터미널 크기 조회, 커서 이동,
 *  색상 출력 제어 및 화면 청소 기능을 담당합니다.
 *  이 모듈은 입력(Input) 처리와 무관하게 출력(Output) 및 메타데이터만 관리합니다.
 *  ------------------------------------------------------------------------------------ */

#include <string>
#include <iostream>

#include "cx_color.hpp" // 색상 정의 사용

namespace cx
{
    // =========================================================================
    // Geometry Structures
    // =========================================================================

    /**
     * @brief 2D 좌표 구조체 ( 1-based Index, Left-top based coordinate system )
     * @note  ANSI 터미널 표준에 따라 좌상단은 (1, 1)입니다.
     */
    struct Coord
    {
        int x = 0; // Column (가로)
        int y = 0; // Row    (세로)

        // --- Constructors ---
        constexpr Coord( void ) noexcept : x( 0 ), y( 0 ) {}
        constexpr Coord( const int _x, const int _y ) noexcept : x( _x ), y( _y ) {}

        // --- Utilities ---

        /// @brief 좌표가 유효한지 확인 (0 이상이어야 함)
        constexpr bool IsValid( void ) const noexcept { return x >= 0 && y >= 0; }

        /// @brief 디버깅용 문자열 반환 "(x, y)"
        std::string ToString( void ) const;

        // --- Operators ---
        constexpr bool  operator==( const Coord& other ) const { return x == other.x && y == other.y; }
        constexpr bool  operator!=( const Coord& other ) const { return !(*this == other); }
        constexpr Coord operator+ ( const Coord& other ) const { return { x + other.x, y + other.y }; }
        constexpr Coord operator- ( const Coord& other ) const { return { x - other.x, y - other.y }; }
        constexpr Coord operator* ( const int    s )     const { return { x * s, y * s }; }
        constexpr Coord operator/ ( const int    s )     const { return { x / s, y / s }; }

        friend std::ostream& operator<<( std::ostream& os, const Coord& c );

        // --- Constants ---
        static const Coord Zero;    // (0, 0) - Invalid/Unset
        static const Coord Origin;  // (1, 1) - Top-Left
    };

    /**
     * @brief 터미널 화면 크기 구조체
     */
    struct TermSize
    {
        int cols = 0; // 너비 (Width)
        int rows = 0; // 높이 (Height)
    };

    // =========================================================================
    // Screen Control Class
    // =========================================================================

    /**
     * @brief 콘솔 화면 제어 정적 클래스
     * @details 커서 이동, 화면 지우기, 터미널 크기 조회 등 출력 관련 기능을 제공합니다.
     */
    class Screen
    {
    public:
        /**
         * @brief  현재 터미널 창의 크기(Col, Row)를 조회합니다.
         * @return TermSize ( 실패 시 0,0 반환 )
         *
         * @details
         *   ioctl 시스템 콜을 사용하여 즉시 커널에서 정보를 가져옵니다.
         *   입력 스트림과 무관하므로 다른 스레드와 충돌 없이 호출 가능합니다.
         */
        static TermSize GetSize( void );

        /**
         * @brief  콘솔 커서를 절대 좌표로 이동시킵니다.
         * @param  pos 이동할 목표 좌표 (1-based)
         * @return 이동 성공 시 true, 유효하지 않은 좌표 등의 이유로 이동 실패 시 false
         *
         * @details
         *   이동 전 현재 터미널 크기를 확인하여, 화면 밖으로 나가지 않도록
         *   자동으로 좌표를 보정(Clamping)합니다.
         */
        static bool MoveCursor( const Coord& pos );

        /**
         * @brief 콘솔 커서를 현재 위치 기준으로 상대 이동시킵니다.
         * @param dx 가로 이동량 (+: 오른쪽, -: 왼쪽)
         * @param dy 세로 이동량 (+: 아래쪽, -: 위쪽)
         */
        static void MoveCursorRelative( const int dx, const int dy );

        /**
         * @brief   화면 전체를 지우고 커서를 (1,1)로 초기화합니다.
         * @details ANSI Escape Code "\033[2J\033[1;1H" 사용
         */
        static void Clear( void );

        /**
         * @brief  전경색(글자색)을 설정합니다.
         * @param  color 설정할 색상 객체 (RGB, Hex 등)
         * @return 설정이 성공했다면 true, 실패했다면 false
         */
        static bool SetColor( const Color& color );

        /**
         * @brief  배경색을 설정합니다.
         * @param  color 설정할 색상 객체
         * @return 설정이 성공했다면 true, 실패했다면 false
         */
        static bool SetBackColor( const Color& color );

        /**
         * @brief 글자색과 배경색을 모두 터미널 기본값으로 초기화합니다.
         */
        static void ResetColor( void );

    private:
        // Static Class: 인스턴스 생성 금지
        Screen()  = delete;
        ~Screen() = delete;
    };

} // namespace cx

#endif // _CONSOLE_X_SCREEN_HPP_