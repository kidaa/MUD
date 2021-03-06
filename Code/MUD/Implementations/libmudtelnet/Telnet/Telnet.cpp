#include<map>
#include<string>
#include "libmudcommon/Error/Error.hpp"
#include "libmudcommon/CommStream/CommStream.hpp"
#include "libmudcommon/Thread/Thread.hpp"
#include "libmudtelnet/Telnet/Telnet.hpp"
#include <cctype>
#include <string.h>

#include<functional>

#include "libmudcommon/Strings/Strings.hpp"

namespace MUD{
    void Telnet::ConnectionHandler( CommStream cs, std::function<void(TelnetSession)> callback ){
        //Run the user's specified callback function
        TelnetSession ts( cs, *this );
        callback( ts );
    }

    DLL_EXPORT Telnet::Telnet() : stream( CommStream::BINARY ) {

    }

    DLL_EXPORT Error Telnet::Listen( int port, std::function<void(TelnetSession)> callback ){
        //Start a CommStream daemon that will fire off new TelnetSession's on connect
        return stream.Listen( port, std::bind( &Telnet::ConnectionHandler, this, std::placeholders::_1, callback ) );
    }

    DLL_EXPORT Error Telnet::Disconnect(){
        //Kill any daemon running on the CommStream if there is one running.
        return stream.Disconnect(true);
    }

    DLL_EXPORT Error Telnet::Disable( Telnet::Feature f ){
        for( size_t i = 0; i < Features.size(); ++i ){
            if( Features[i] == f ){
                Features.erase( Features.begin() + i );
            }
        }
        return Error::None;
    }

    DLL_EXPORT Error Telnet::Enable( Telnet::Feature f ){
        for( size_t i = 0; i < Features.size(); ++i ){
            if( Features[i] == f ){
                return Error::AlreadyEnabled;
            }
        }
        Features.push_back( f );
        return Error::None;
    }

    DLL_EXPORT Error Telnet::ReadTerms( std::string fname ){
        FILE* f = fopen( fname.c_str(), "r" );
        if( f == nullptr )
            return Error::FileNotFound;
        Terminal temp;
        bool readingterm = false; //Indicates whether or not we're reading a terminal name or parameter
        while( true ){
            char buffer[1000];

            fgets( buffer, 500, f );
            if( feof(f) )
                break;
            int strend = strlen( buffer );
            while(strend >= 0 && ( buffer[strend] == '\0' || isspace(buffer[strend]) ) ){
                buffer[strend--] = 0;
            }

            //If we're on a blank line...
            if( strlen( buffer ) == 0 ){
                //...And we were reading a definition...
                if( readingterm ){
                    //Then store it.
                    readingterm = false;
                    SupportedTerms[temp.Name] = temp;

                }
            }
            //Otherwise if we're not on a blank line...
            else{
                //And if we aren't reading a definition...
                if( !readingterm ){
                    //Then this is the name of the next terminal. Prepare to read parameters.
                    char* b = buffer;
                    while( *b != '\0' && isspace( *b ) )
                            b++;
                    strupr( b );
                    temp = Terminal();
                    temp.Name = b;
                    readingterm = true;

                }
                //But if we are...
                else{
                    //Find a colon in the buffer
                    char *iter = buffer;
                    while( *iter != ':' && *iter != '\0' ){
                        iter++;
                    }
                    //If a colon indeed exists...
                    if( *iter != '\0' ){
                        *iter = '\0';
                        //Set Key to the string between buffer and iter
                        char* Key = buffer;
                        //Set Value to the string following iter
                        char* Value = iter+1;

                        //Trim the whitespace off the start of Key and Value.
                        while( *Key != '\0' && isspace( *Key ) )
                            Key++;
                        while( *Value != '\0' && isspace( *Value ) )
                            Value++;

                        //If we actually have a parameter value...
                        if( strlen(Value) != 0 ){
                            //Make it all uppercase
                            strupr( Key );
                            strupr( Value );
                            bool affirmative = Value[0] == 'Y';

                            switch( HashString( Key ) ){
                                case HashString( "COLOR" ):         temp.Color = affirmative;           break;
                                case HashString( "WRAPS" ):         temp.Wraps = affirmative;           break;
                                case HashString( "ANSI-ESCAPE" ):   temp.ANSIEscape = affirmative;      break;
                                case HashString( "PREFERENCE" ):    temp.Preference = atol( Value );    break;
                                case HashString( "INHERIT" ): {
                                    std::string n = temp.Name;
                                    temp = SupportedTerms[Value];
                                    temp.Name = n;
                                }                                                                       break;
                                default: break;
                            }
                        }
                    }
                }
            }
        } //End of while

        //If we hit EOF before writing a terminal to the supported terminal map, write it.
        if( readingterm ){
            readingterm = false;
            SupportedTerms[temp.Name] = temp;
        }
        return Error::None;
    }
}

