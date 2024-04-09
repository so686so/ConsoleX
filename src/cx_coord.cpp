// ConsoleX
#include "cx_coord.h"
#include "cx_console.h" // GetConsoleH, GetConsoleW
// STL::C++
#include <string>       // string, to_string
// STL::C
#include <stdio.h>      // fflush, printf
#include <fcntl.h>      // open
#include <unistd.h>     // read, write, close, open
#include <termio.h>     // tcgetattr, tcsetattr, ttyname, termios
#include <errno.h>      // errno

namespace cx // ConsoleX main namespace
{
    // 사용할 함수와 클래스를 명확히 특정해 using 사용.
    // namespace 전체를 using 하는 것은 추천하지 않음.
    using std::string;
    using std::to_string;

    cx::Coord::Coord( const int _x, const int _y )
        : x( _x )
        , y( _y )
        , v( true )
    {}

    cx::Coord::Coord( const bool valid )
        : x( 0 )
        , y( 0 )
        , v( valid )
    {}

    cx::Coord::Coord( const cx::Coord& coord )
        : x( coord.x )
        , y( coord.y )
        , v( coord ? true : false )
    {}

    string cx::Coord::str( void ) const noexcept
    {
        // 정상 Coord 객체일 때
        if( *this ) {
            string coord = "( " + std::to_string( this->x ) + ", " + std::to_string( this->y ) + " )";
            return coord;
        }
        // ErrorCoord 객체일 때
        return string { "( INV, INV )" };
    }

    bool cx::Coord::operator==( const cx::Coord& other ) const
    {
        return ( this->x == other.x ) && ( this->y == other.y );
    }

    bool cx::Coord::operator!=( const cx::Coord& other ) const
    {
        return !( *this == other );
    }

    cx::Coord cx::Coord::operator+( const cx::Coord& other ) const
    {
        return cx::Coord { this->x + other.x, this->y + other.y };
    }

    cx::Coord cx::Coord::operator-( const cx::Coord& other ) const
    {
        return cx::Coord { this->x - other.x, this->y - other.y };
    }

    cx::Coord cx::Coord::operator*( const int scalar ) const
    {
        return cx::Coord { this->x * scalar, this->y * scalar };
    }

    cx::Coord cx::Coord::operator/( const int scalar ) const
    {
        return cx::Coord { this->x / scalar, this->y / scalar };
    }

    cx::Coord& cx::Coord::operator=( const cx::Coord& other )
    {
        if( this != &other ) {
            this->x = other.x;
            this->y = other.y;
        }
        return *this;
    }

    // local scope 변수 할당을 위한 이름 없는 namespace
    namespace
    {
        // 해당 변수는 로컬에서만 사용가능
        constexpr int RD_EOF = -1;
        constexpr int RD_EIO = -2;
    }

    static inline int rd( const int fd  )
    {
        unsigned char buffer[4] = { 0x00, };
        ssize_t n = 0;

        while( true ) {
            n = read( fd, buffer, 1 );
            if      ( n  > (ssize_t) 0 ) return buffer[0];
            else if ( n == (ssize_t) 0 ) return RD_EOF;
            else if ( n != (ssize_t)-1 ) return RD_EIO;
            else if ( errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK ) return RD_EIO;
        }
    }

    static inline int wr( const int fd, const char* const data , const size_t bytes )
    {
        const char*       head = data;
        const char* const tail = data + bytes;
        ssize_t n = 0;

        while( head < tail ) {
            n = write( fd, head, (size_t)( tail - head ) );
            if       ( n  > (ssize_t) 0 ) head += n;
            else if  ( n != (ssize_t)-1 ) return EIO;
            else if  ( errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK ) return errno;
        }
        return 0;
    }

    /* Return a new file descriptor to the current tty */
    static inline int current_tty( void ) noexcept
    {
        // get tty name
        const char* dev = ttyname( STDIN_FILENO  );
        if ( !dev ) dev = ttyname( STDOUT_FILENO );
        if ( !dev ) dev = ttyname( STDERR_FILENO );
        if ( !dev ) {
            errno = ENOTTY;
            return -1;
        }

        int    fd;
        do   { fd = open( dev, O_RDWR | O_NOCTTY ); }
        while( fd == -1 && errno == EINTR );
        return fd;
    }

    static inline bool is_digit_ascii( const int ascii )
    {
        return ( ascii >= '0' && ascii <= '9' );
    }

    /**
     * @ref
     * https://www.linuxquestions.org/questions/programming-9/get-cursor-position-in-c-947833/
     */
    cx::Coord GetCoord( void ) noexcept
    {
        struct termios save_attr, new_attr;

        // flush last stdout buffer
        fflush( stdout );

        const int tty = current_tty();
        if( tty == -1 )
            return cx::ErrorCoord {};

        int cmd_res = 0;
        // save current terminal setting
        do    { cmd_res = tcgetattr( tty, &save_attr ); }
        while ( cmd_res == -1 && errno == EINTR );

        if( cmd_res == -1 )
            return cx::ErrorCoord {};

        new_attr = save_attr;

        // disable ICANON, ECHO, CREAD
        new_attr.c_lflag &= ~ICANON;
        new_attr.c_lflag &= ~ECHO;
        new_attr.c_lflag &= ~CREAD;

        int cx_rows = 0, cx_cols = 0;
        do {
            // set modified termios setting
            do    { cmd_res = tcsetattr( tty, TCSANOW, &new_attr); }
            while ( cmd_res == -1 && errno == EINTR );

            // set failed
            if( cmd_res == -1 ) break;

            // request cursor abs coord from the console
            if( wr( tty, "\033[6n", 4 ) != 0 ) break;

            // Return Example
            // --------------------------------
            //           ESC[35;40R
            // --------------------------------
            // it means,
            // start with left-top,
            // xpos(cols) = 40
            // ypos(rows) = 35

            // expect result : ESC(27)
            if( rd(tty) != 27  ) break;

            // expect result : '['
            if( rd(tty) != '[' ) break;

            // parse ypos(row)
            cmd_res = rd(tty);
            while( is_digit_ascii( cmd_res ) ) {
                cx_rows = ( 10 * cx_rows ) + ( cmd_res - '0' );
                cmd_res = rd(tty);
            }

            // expect result : ';'
            if( cmd_res != ';' ) break;

            // parse xpos(col)
            cmd_res = rd(tty);
            while( is_digit_ascii( cmd_res ) ) {
                cx_cols = ( 10 * cx_cols ) + ( cmd_res - '0' );
                cmd_res = rd(tty);
            }

            // expect result : 'R'
            if( cmd_res != 'R' ) break;

        } while( false );

        // restore saved termios setting
        do    { cmd_res = tcsetattr( tty, TCSANOW, &save_attr ); }
        while ( cmd_res == -1 && errno == EINTR );

        if( cx_cols == 0 || cx_rows == 0 )
            return cx::ErrorCoord {};

        // if success
        return cx::Coord{ cx_cols, cx_rows };
    }

    // curr 값을 min ~ max 값 사이로 강제하는 함수
    static inline int coord_clamp( const int min, const int curr, const int max ) noexcept
    {
        return ( curr < min ) ? min : ( ( curr > max ) ? max : curr );
    }

    bool SetCoord( const cx::Coord& coord ) noexcept
    {
        // check valid coord
        if( !coord )
            return false;

        // clamp
        const int xpos = coord_clamp( 1, coord.x, cx::GetConsoleW() );
        const int ypos = coord_clamp( 1, coord.y, cx::GetConsoleH() );

        // move
        printf( "\033[%d;%df", ypos, xpos );
        return true;
    }

} // nsp::cx