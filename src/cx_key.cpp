// ConsoleX
#include "cx_key.h"

// STL :: C
#include <stdio.h>      //
#include <unistd.h>     // read
#include <memory.h>     // memset
#include <sys/select.h> // select

namespace cx // ConsoleX main namespace
{
    // 터미널에서 커서를 감수는 함수
    inline static void _CursorOff( void ) noexcept { printf("\33[?25l"); }

    // 터미널에서 커서를 다시 보이게 하는 함수
    inline static void _CursorOn ( void ) noexcept { printf("\33[?25h"); }

    // 터미널을 non-canonical mode로 바꾸는 함수
    static Termios _SetNonCanonicalMode( Termios* origin_attr ) noexcept
    {
        Termios save_attr = *origin_attr; // save attribute origin

        // Set new STDIN attribute to accept input immediately
        origin_attr->c_lflag     &= ~(ICANON | ECHO); // Non-canonical & hide echo
        origin_attr->c_cc[VMIN]  = 2;                 // Input buffer minimum number of characters returned
        origin_attr->c_cc[VTIME] = 1;                 // Wait maximum time ( 0.01 sec )

        // Apply
        tcsetattr( STDIN_FILENO, TCSANOW, origin_attr );

        // Return saved origin attribute
        return save_attr;
    }

    /**
     * 일반 ASCII 값과 달리 방향키의 ASCII 값은 '27 91 65' 와 같이
     * 연속된 세 개의 ASCII 값으로 읽혀짐. 이때 방향키의 두 번째
     * 키값은 91로 고정.
     */
    constexpr int ARROW_CHECK_VALUE = 91;
    constexpr int GENERAL_KEY_INDEX = 0;  // 방향키나 'ESC'키를 제외한 일반 키 값의 인덱스
    constexpr int ARROW_CHECK_INDEX = 1;  // 방향키인지 'ESC'키인지 확인하는 값의 인덱스
    constexpr int ARROW_VALUE_INDEX = 2;  // 입력값이 방향키일 때 실제 키값의 인덱스
    constexpr int FUNC_KEY_INDEX_01 = 2;
    constexpr int FUNC_KEY_INDEX_02 = 3;

    // FunctionKey(F키) 추출 함수
    // 해당 함수는 기종 기기별로 값이 다를 수 있음...
    inline static KeyBoard _FkeyCast_( const char v1, const char v2 )
    {
        if      ( v1 == 49 && v2 == 49 ) return KeyBoard::F1;
        else if ( v1 == 49 && v2 == 50 ) return KeyBoard::F2;
        else if ( v1 == 49 && v2 == 51 ) return KeyBoard::F3;
        else if ( v1 == 49 && v2 == 52 ) return KeyBoard::F4;
        else if ( v1 == 49 && v2 == 53 ) return KeyBoard::F5;
        else if ( v1 == 49 && v2 == 55 ) return KeyBoard::F6;
        else if ( v1 == 49 && v2 == 56 ) return KeyBoard::F7;
        else if ( v1 == 49 && v2 == 57 ) return KeyBoard::F8;
        else if ( v1 == 50 && v2 == 48 ) return KeyBoard::F9;
        else if ( v1 == 50 && v2 == 49 ) return KeyBoard::F10;
        else if ( v1 == 50 && v2 == 51 ) return KeyBoard::F11;
        else if ( v1 == 50 && v2 == 52 ) return KeyBoard::F12;
        return KeyBoard::NONE_INPUT;
    }

    // 키입력 받은 데이터 값을 cx::KeyBoard 값으로 변환하는 함수
    static KeyBoard _VerifyKeyInput( const char* const read_data, const ssize_t byte_len )
    {
        switch( byte_len )
        {
            // General Word keys
            case 1: return (KeyBoard)( read_data[GENERAL_KEY_INDEX] );
            case 2: return (KeyBoard)( read_data[GENERAL_KEY_INDEX] );

            // Arrow keys
            case 3: return (KeyBoard)( read_data[ARROW_VALUE_INDEX] + ARROW_ADD_VALUE );

            // Functional keys
            case 4: return (KeyBoard)( read_data[FUNC_KEY_INDEX_01] + FUNCIONAL_VALUE );
            case 5: return _FkeyCast_( read_data[FUNC_KEY_INDEX_01] , read_data[FUNC_KEY_INDEX_02] );
        }
        return KeyBoard::NONE_INPUT;
    }

    static KeyBoard _CapitalizeIfAlphabet( const KeyBoard target )
    {
        LowerKeyBoard n_target = static_cast<LowerKeyBoard>( target );
        if( n_target >= LowerKeyBoard::a && n_target <= LowerKeyBoard::z )
            return static_cast<KeyBoard>( n_target - CAPITALIZE_OFFSET );
        return target;
    }

    // select 이용해서 키 입력 이벤트 timeout_wait 하는 함수
    static int _OnEvent( const int timeout_milsec ) noexcept
    {
        const int us = ( timeout_milsec > 0 ) ? timeout_milsec * 1000 : 0;
        Timeval   tv = { 0L, static_cast<long>( us ) };
        fd_set    fds;

        FD_ZERO( &fds );
        FD_SET ( 0, &fds );

        return select( 1, &fds, NULL, NULL, &tv );
    }

    constexpr int IMAX_LEN = ( 5 + 1 );
    KeyBoard Key::GetKey( void ) noexcept
    {
        auto sptr_key = Key::GetKeyPtr();

        std::unique_lock<std::mutex> lck( sptr_key->mtx, std::defer_lock );
        if( lck.try_lock() == false )
            return KeyBoard::ALREADY_OCCUPIED;

        if( sptr_key->IsKeyStrokeDirect() == false )
            sptr_key->Resume();

        KeyBoard input_key  = KeyBoard::NONE_INPUT;
        ssize_t  read_bytes = 0;
        char     read_data[IMAX_LEN] = { 0x00, };

        sptr_key->await_force_stop_flg = false;
        while( _OnEvent( KeyDelay::FPS_10 ) <= 0 ) {
            if( sptr_key->await_force_stop_flg ) {
                sptr_key->await_force_stop_flg = false;
                return KeyBoard::FORCE_INTERRUPT;
            }
        }

        read_bytes = read( STDIN_FILENO, read_data, IMAX_LEN );
        if( read_bytes > 0 )
            input_key = _CapitalizeIfAlphabet( _VerifyKeyInput( read_data, read_bytes ) );
        return input_key;
    }

    KeyBoard Key::GetKeyTimeout( const int timeout_milsec ) noexcept
    {
        auto sptr_key = Key::GetKeyPtr();

        std::unique_lock<std::mutex> lck( sptr_key->mtx, std::defer_lock );
        if( lck.try_lock() == false )
            return KeyBoard::ALREADY_OCCUPIED;

        if( sptr_key->IsKeyStrokeDirect() == false )
            sptr_key->Resume();

        KeyBoard  input_key  = KeyBoard::NONE_INPUT;
        ssize_t   read_bytes = 0;
        char      read_data[IMAX_LEN]   = { 0x00, };
        const int wait_timeout_on_event = ( timeout_milsec < 0 ) ? sptr_key->await_timeout_milsec : timeout_milsec;

        if( _OnEvent( wait_timeout_on_event ) > 0 ) {
            read_bytes = read( STDIN_FILENO, read_data, IMAX_LEN );
            if( read_bytes ) input_key = _CapitalizeIfAlphabet( _VerifyKeyInput( read_data, read_bytes ) );
        }
        return input_key;
    }

    bool Key::SetReadKeyAwaitTimeout( const int timeout_milsec ) noexcept
    {
        auto sptr_key = Key::GetKeyPtr();

        std::unique_lock<std::mutex> lck( sptr_key->mtx, std::defer_lock );
        if( lck.try_lock() == false )
            return false;

        if( timeout_milsec > 0 ) {
            sptr_key->await_timeout_milsec = timeout_milsec;
            return true;
        }
        return false;
    }

    bool Key::TryPause( void ) noexcept
    {
        auto sptr_key = Key::GetKeyPtr();

        std::unique_lock<std::mutex> lck( sptr_key->mtx, std::defer_lock );
        if( lck.try_lock() == false )
            return false;

        _CursorOn();
        sptr_key->SetKeyStrokeWhenPressEnter();

        return true;
    }

    void Key::ForcePause( void ) noexcept
    {
        auto sptr_key = Key::GetKeyPtr();
        std::unique_lock<std::mutex> lck( sptr_key->mtx );

        _CursorOn();
        sptr_key->SetKeyStrokeWhenPressEnter();
    }

    void Key::Resume( void ) noexcept
    {
        _CursorOff();
        Key::GetKeyPtr()->SetKeyStrokeDirect();
    }

    void Key::Init( void ) noexcept
    {
        _CursorOff();
        this->SetKeyStrokeDirect();
    }

    void Key::Deinit( void ) noexcept
    {
        _CursorOn();
        this->SetKeyStrokeWhenPressEnter();
    }

    bool Key::SetKeyStrokeDirect() noexcept
    {
        if( this->is_key_stroke_direct == true )
            return true;

        Termios stdin_attr;
        memset( &stdin_attr, 0x00, sizeof(Termios) );

        // Execute function to get existing stdin options and assign new options
        tcgetattr( STDIN_FILENO, &stdin_attr );

        memset( &( this->save_attr ), 0x00, sizeof(Termios) );
        this->save_attr = _SetNonCanonicalMode( &stdin_attr );
        this->is_key_stroke_direct = true;

        return true;
    }

    bool Key::SetKeyStrokeWhenPressEnter() noexcept
    {
        if( this->is_key_stroke_direct == false )
            return true;

        // rollback to existing stdin options
        tcsetattr( STDIN_FILENO, TCSANOW, &( this->save_attr ) );
        this->is_key_stroke_direct = false;

        return true;
    }

    std::shared_ptr<cx::Key> Key::GetKeyPtr( void ) noexcept
    {
        static std::shared_ptr<cx::Key> singleton( new Key() );
        return singleton;
    }

}; // nsp: cx