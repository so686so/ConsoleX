#include "cx_screen.hpp"

// System Headers
#include <sys/ioctl.h> // ioctl, TIOCGWINSZ
#include <unistd.h>    // STDOUT_FILENO
#include <algorithm>   // std::clamp
#include <iostream>

namespace cx
{
    // =========================================================================
    // Coord Struct Implementation
    // =========================================================================

    // Static Constants Definition
    const Coord Coord::Zero   = { 0, 0 };
    const Coord Coord::Origin = { 0, 0 };

    std::string Coord::ToString( void ) const
    {
        return "( " + std::to_string( x ) + ", " + std::to_string( y ) + " )";
    }

    std::ostream& operator<<( std::ostream& os, const Coord& c )
    {
        os << c.ToString();
        return os;
    }

    // =========================================================================
    // Screen Class Implementation
    // =========================================================================

    TermSize Screen::GetSize( void )
    {
        struct winsize ws;

        // STDOUT_FILENO(표준 출력)의 윈도우 사이즈 정보를 가져옵니다.
        // 실패 시 0,0을 반환하여 호출 측에서 예외 처리를 할 수 있게 합니다.
        if ( ioctl( STDOUT_FILENO, TIOCGWINSZ, &ws ) == -1 ) {
            return { 0, 0 };
        }
        return { ws.ws_col, ws.ws_row };
    }

    // [Internal Helper] 좌표를 현재 화면 크기 내로 제한(Clamping)합니다.
    static Coord ClampToTerminal( const Coord& pos )
    {
        // Screen 클래스 자신의 GetSize 함수를 호출 (독립적 동작)
        auto size = Screen::GetSize();

        // 터미널 크기를 못 가져왔거나 0일 경우에 대한 방어 코드
        // (최소 1x1 보장 혹은 매우 큰 값으로 설정하여 로직 오류 방지)
        int max_w = ( size.cols > 0 ) ? size.cols : 999;
        int max_h = ( size.rows > 0 ) ? size.rows : 999;

        // 0 부터 (길이 - 1) 까지로 제한
        return Coord {
            std::clamp( pos.x, 0, max_w - 1 ),
            std::clamp( pos.y, 0, max_h - 1 )
        };
    }

    bool Screen::MoveCursor( const Coord& pos )
    {
        // 유효하지 않은 좌표라면 실패 반환
        if( !pos.IsValid() )
            return false;

        // 화면 밖으로 커서가 나가는 것을 방지하기 위해 좌표 보정
        Coord safe_pos = ClampToTerminal( pos );

        // 핵심: User(0-based) -> ANSI(1-based) 변환
        // \033[<Row>;<Col>H
        std::cout << "\033[" << safe_pos.y + 1 << ";" << safe_pos.x + 1 << "H" << std::flush;

        return true;
    }

    void Screen::MoveCursorRelative( const int dx, const int dy )
    {
        if( dx == 0 && dy == 0 ) return;

        // 상대 이동 명령은 터미널 에뮬레이터가 알아서 화면 끝 처리를 하므로
        // 별도의 Clamp 로직 없이 명령만 전송합니다.

        if( dy < 0 ) std::cout << "\033[" << -dy << "A"; // Up
        if( dy > 0 ) std::cout << "\033[" <<  dy << "B"; // Down
        if( dx > 0 ) std::cout << "\033[" <<  dx << "C"; // Right
        if( dx < 0 ) std::cout << "\033[" << -dx << "D"; // Left

        std::cout << std::flush;
    }

    void Screen::Clear( void )
    {
        // \033[2J: 화면 전체 지우기
        // \033[1;1H: 커서를 좌상단(1,1)으로 이동
        std::cout << "\033[2J\033[1;1H" << std::flush;
    }

    bool Screen::SetColor( const Color& color )
    {
        if( !color.IsValid() )
            return false;

        // Color 객체로부터 전경색 ANSI 코드를 받아 출력
        std::cout << color.ToAnsiForeground() << std::flush;

        return true;
    }

    bool Screen::SetBackColor( const Color& color )
    {
        if( !color.IsValid() )
            return false;

        // Color 객체로부터 배경색 ANSI 코드를 받아 출력
        std::cout << color.ToAnsiBackground() << std::flush;

        return true;
    }

    void Screen::ResetColor( void )
    {
        // Color::Reset 객체의 코드를 출력 (\033[0m)
        std::cout << Color::Reset.ToAnsiForeground() << std::flush;
    }

} // namespace cx