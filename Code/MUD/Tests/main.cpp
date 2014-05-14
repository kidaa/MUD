#include<cstdio>
#include "Tester.hpp"
#include "CommStream/CommStream.hpp"
#include "CommStream/Cipher.hpp"
#include <random>
#include <cstdio>

int main(){
    Test<GlobalMUD::Ciphers::Cipher>();
    Test<GlobalMUD::Ciphers::XOR>();
    Test<GlobalMUD::CommStream>();

}
