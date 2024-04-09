// ConsoleX
#include "cx_console.h"
// STL::C++
#include <string>  // string, append, c_str, stoi
// STL::C
#include <stdio.h> // popen

namespace cx // ConsoleX main namespace
{
    // 사용할 함수와 클래스를 명확히 특정해 using 사용.
    // namespace 전체를 using 하는 것은 추천하지 않음.
    using std::string;
    using std::stoi;

    // local scope 변수 할당을 위한 이름 없는 namespace
    namespace
    {
        // 해당 변수는 로컬에서만 사용가능
        constexpr size_t CMD_PKT_RECV_MAX_LEN = 512;
    }

    // source(.cpp) 파일에서 static 함수 정의 시 해당 파일 내부에서만 사용 가능
    // 쉘에 명령어를 입력한 뒤 출력 결과값을 std::string으로 읽어오는 함수
    static string _GetStringResultFromCommand( const string&& cmd ) noexcept
    {
        // popen : Executes the given cmd as a shell and returns fd after connecting the pipe.
        FILE* stream = popen( cmd.c_str(), "r" );

        // 만약 popen() 실패 시, 빈 문자열 반환
        if( !stream )
            return string {};

        string result {};
        result.reserve( CMD_PKT_RECV_MAX_LEN );

        char buffer[CMD_PKT_RECV_MAX_LEN] = { '\0', };
        while( fgets( buffer, CMD_PKT_RECV_MAX_LEN, stream ) != NULL )
            result.append( buffer );

        pclose( stream );
        return result;
    }

    size_t GetConsoleW( void )
    {
        try          { return stoi( _GetStringResultFromCommand( R"(stty size | awk '{print $2}')" ) ); }
        catch( ... ) { return 0; }
    }

    size_t GetConsoleH( void )
    {
        try          { return stoi( _GetStringResultFromCommand( R"(stty size | awk '{print $1}')" ) ); }
        catch( ... ) { return 0; }
    }

    std::ostream& console( void ) noexcept
    {
        cx::ResetColor();
        return std::cout;
    }

    std::ostream& ResetColor( std::ostream& _os ) noexcept
    {
        cx::ResetColor();
        return _os;
    }

} // nsp::cx