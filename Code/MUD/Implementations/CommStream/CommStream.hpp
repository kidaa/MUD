#ifndef MUD_COMMSTREAM_COMMSTREAM_HPP
#define MUD_COMMSTREAM_COMMSTREAM_HPP

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include "Global/RefCounter.hpp"
#include <utility>
#ifdef _WIN32
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <termios.h>
    #include <pthread.h>
    #include <errno.h>
    #include <signal.h>
    #define SOCKET unsigned int
#endif

#include "Global/Error.hpp"
#include "Global/Mutex.hpp"
#include "CommStream/Cipher.hpp"
#include "Global/Thread.hpp"
#define NETBUFFERSIZE 1000


namespace GlobalMUD{
    class CommStreamInternal;

    class CommStream{
    friend CommStreamInternal;
    public:
        enum Encryption{
            NONE = 0,
            XOR = 1
        };
        enum ReceiveType{
            BINARY = 0,
            LINES = 1
        };
    private:
        void UseSocket(SOCKET sock);
        void Terminate();
        void PushData( char* data, size_t len );
        bool CanSend();
        CommStreamInternal* Get();
        SOCKET GetConnection();
        RefCounter<CommStreamInternal> Internal;
        bool transmitting;


    public:
        CommStream& operator=(CommStream other);
        CommStream( ReceiveType rectype = LINES );
        ~CommStream();
        Error Connect( std::string address, int port );
        bool Connected( );
        Error Disconnect( bool force = false );
        Error Listen( int port, void(*func)(CommStream stream, void* data), void* data = 0 );
        Error ListenOn( std::string address, int port, void(*func)(CommStream stream, void* data), void* data = 0 );
        Error Send( std::string message, bool important = false );
        Error Send( void* message, size_t len, bool important = false );
        Error SendFile( std::string path, bool important = false );
        Error Receive( std::string &message );
        Error Receive( char* message, size_t& len );
        Error Encrypt( Encryption type );
        int SendBufferSize();
        #ifdef RunUnitTests
        static bool RunTests();
        #endif
        static int ServiceSockets();


    };
    class CommStreamInternal{
        friend CommStream;
        struct Message{
            enum FLAGS{
                NONE = 0,
                IMPORTANT = 1
            };
            Message(std::string message, int flagss );
            Message(const char* message, size_t len, int flagss );

            std::shared_ptr<char> msg;
            size_t length;
            int flags;
        };
        CommStream::ReceiveType MyReceiveType;
        CommStream::Encryption MyEncryption;
        Ciphers::Cipher* MyCipher;
        SOCKET Connection;
        std::deque<Message> SendBuffer;
        int ImportantMessages;
        std::deque<Message> RecvBuffer;
        std::string RecvLinesBuffer;
        static Thread* MyThread;
        static void ServiceSocketsLoop();
        bool aborted;
        bool isconnected;
        unsigned int connecting;
        void Terminate();
        void PushData( char* data, size_t len );
        bool CanSend();
        Message GetSend();
        Mutex MyLock;
        bool disconnecting;

        public:
        CommStreamInternal( CommStream::ReceiveType rectype = CommStream::LINES );
        ~CommStreamInternal();
        Error Connect( std::string address, int port );
        bool Connected( );
        Error Disconnect( bool force = false );
        Error Listen( int port, void(*func)(CommStream stream, void* data), void* data = 0 );
        Error ListenOn( std::string address, int port, void(*func)(CommStream stream, void* data), void* data = 0 );
        Error Send( std::string message, bool important = false );
        Error Send( void* message, size_t len, bool important = false );
        Error Receive( std::string &message );
        Error Receive( char* message, size_t& len );
        Error Encrypt( CommStream::Encryption type );
        int SendBufferSize();

        static int ServiceSockets();
        static void PushStream( CommStream topush );
        static Mutex Lock;
        static std::vector<CommStream> CommStreams;
    };


}
#undef SOCKET

#endif
