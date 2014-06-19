#include<cstdio>
#include "Tester.hpp"
#include "CommStream/CommStream.hpp"
#include "CommStream/Cipher.hpp"
#include "Global/Thread.hpp"
#include "HTTPd/HTTPd.hpp"
#include "Global/Error.hpp"
#include <random>
#include <cstdio>

using namespace GlobalMUD;
int main(){
    printf("%s\n\n",ERROR::ToString(Error::ListenFailure).c_str());

    #ifdef _WIN32
    WSADATA globalWSAData;
    WSAStartup( MAKEWORD(2, 2), &globalWSAData );
    #endif
    Test<GlobalMUD::Thread>();
    Test<GlobalMUD::Ciphers::Cipher>();
    Test<GlobalMUD::Ciphers::XOR>();
    Test<GlobalMUD::CommStream>();
    Test<GlobalMUD::HTTPd>();

}
