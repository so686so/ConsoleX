// ConsoleX
#include "cx_console.h"
// STL::C++
#include <string>  // string, append, c_str, stoi
// STL::C
#include <stdio.h> // popen

namespace cx // ConsoleX main namespace
{
    // Clearly specify which classes and functions to use
    using std::string;
    using std::stoi;

    namespace // nameless namespace for local scope variables
    {
        constexpr size_t CMD_PKT_RECV_MAX_LEN = 512;
    }

    static string _GetStringResultFromCommand( const string&& cmd ) noexcept
    {
        // popen : Executes the given cmd as a shell and returns fd after connecting the pipe.
        FILE* stream = popen( cmd.c_str(), "r" );

        // if popen failed, return empty string
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

} // nsp::cx