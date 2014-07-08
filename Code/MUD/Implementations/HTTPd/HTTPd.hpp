#ifndef MUD_HTTPD_HTTPD_HPP
#define MUD_HTTPD_HTTPD_HPP


#include<map>
#include<string>
#include "Error/Error.hpp"
#include "CommStream/CommStream.hpp"
#include "Thread/Thread.hpp"
#include "Memory/Stream.hpp"

namespace GlobalMUD{
    class HTTPd{
    public:
        struct HTTPResponse{
            int status;
            std::map<std::string,std::string> headers;
            std::map<std::string,std::string> gets;
            std::map<std::string,std::string> posts;
            std::string request;
            std::string filepath;
            std::vector<char> content;
            void SetContent(std::string value);
        };
        struct MountPoint{
            enum Types{
                BAD = 0,
                FOLDER,
                FILE,
                FUNCTION
            } Type;
            std::string Path;
            std::string MountPath;
            HTTPResponse(*Func)(HTTPResponse response, HTTPd& parent);
        };
    private:
        std::map<std::string, MountPoint> MountPoints;
        std::string Address;
        int Port;
        CommStream *stream;
        Thread *MyThread;
        static void ConnectionHandler(CommStream stream, void* argument );
        static void ConnectionHandlerThread(CommStream stream, void* parent);
        static std::string ParseRequest(Stream &data, HTTPResponse &r);

    public:
        Error MountFolder(std::string mountpath, std::string folderpath);
        Error MountDirectory(std::string mountpath, std::string folderpath);
        Error MountFile(std::string mountpath, std::string filepath);
        Error MountFunction(std::string mountpath, HTTPResponse(*func)(HTTPResponse response, HTTPd& parent));
        HTTPd(std::string address, int port);
        ~HTTPd();
        Error Run();
        HTTPResponse PushPage( HTTPd::HTTPResponse r, std::string mount = "" );
        static std::string URLDecode(std::string url);

        static HTTPResponse DoError(HTTPResponse response, HTTPd& parent, int code);


        #ifdef RunUnitTests
        static bool RunTests();
        #endif

    };
}

#endif
