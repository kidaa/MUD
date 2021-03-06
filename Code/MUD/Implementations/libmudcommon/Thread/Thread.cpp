#include "Thread/Thread.hpp"
#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
    #include <unistd.h>
#endif
#include "Thread/Mutex.hpp"
#include "Error/Error.hpp"
#include <cstdio>

namespace MUD{
    DLL_EXPORT Thread::~Thread(){
        Run();
        //To prevent unforeseen issues, join with the thread unless
        //it was explicitly detached.
        if( !detached )
            Join();
    }

    THREADRETURN THREADCallFunction Thread::ThreadFunc(void*d){
        Arguments* args = (Arguments*) d;
        #ifndef _WIN32
        //This is to simulate a suspended thread on POSIX systems
        args->Lock.Wait();
        #endif

        args->f();
        delete args;
        return 0;
    }
    DLL_EXPORT Thread::Thread( std::function<void()> f ){
            #ifdef DISABLETHREADS
            //For debugging purposes. Make the thread call the function instead of making a thread.
            f();
            #else
            detached = false;
            Arguments* args = new Arguments;
            args->f = f;
            //This is to simulate a suspended thread on POSIX systems
            args->Lock.Lock();
            Lock = args->Lock;
            //Spawn the thread
            #ifdef _WIN32
            ThreadHandle = CreateThread( nullptr, 8048, MUD::Thread::ThreadFunc, args, CREATE_SUSPENDED, &ThreadID );
            valid = ThreadHandle != nullptr;
            #else
            pthread_attr_t attr;
            pthread_attr_init( &attr );
            valid = 0 == pthread_create(&ThreadHandle, nullptr, MUD::Thread::ThreadFunc, args);

            pthread_attr_destroy( &attr );
            #endif
            #endif
        }

    #ifdef _WIN32
    DLL_EXPORT void Thread::Run(){
        Lock.Unlock();
        ResumeThread( ThreadHandle );
    }

    DLL_EXPORT void Thread::Join(){
        if( !detached ){
        WaitForSingleObject( ThreadHandle, INFINITE );
        if( ThreadHandle != nullptr )
            CloseHandle( ThreadHandle );
        ThreadHandle = nullptr;
        }
    }

    DLL_EXPORT void Thread::Suspend(){
        SuspendThread( ThreadHandle );
    }

    DLL_EXPORT void Thread::Kill(){
        TerminateThread( ThreadHandle, 0 );
        if( ThreadHandle != nullptr )
            CloseHandle( ThreadHandle );
        ThreadHandle = nullptr;
    }

    DLL_EXPORT void Thread::Detach(){
        detached = true;
        if( ThreadHandle != nullptr )
            CloseHandle( ThreadHandle );
        ThreadHandle = nullptr;
    }

    DLL_EXPORT void Thread::Sleep(unsigned long msec){
        ::Sleep(msec);
    }

    #else
    DLL_EXPORT void Thread::Run(){
        Lock.Unlock();
    }

    DLL_EXPORT void Thread::Join(){
        if( valid )
            pthread_join( ThreadHandle, nullptr );
        valid = !valid;
    }

    DLL_EXPORT void Thread::Suspend(){
    //No-op with pthreads
    }

    DLL_EXPORT void Thread::Kill(){
        if( valid )
            pthread_cancel( ThreadHandle );
    }
    DLL_EXPORT void Thread::Detach(){
        detached = true;
        pthread_detach( ThreadHandle );
        pthread_detach( ThreadHandle );
        pthread_detach( ThreadHandle );
        pthread_detach( ThreadHandle );
    }

    DLL_EXPORT void Thread::Sleep(unsigned long msec){
        ::usleep(msec*1000);
    }

    #endif

    #ifdef RunUnitTests
    //A list of functions meant to demonstrate how each feature works.
    //There are so many just in case there's a corner case that starts to fail.
    struct functions{
        static int accumulator;
        int accumulator2;
        static void testA(){
            for( int i = 0; i < 100000; ++i ){
                accumulator++;
            }
        }
        static void testB_1(){
            for( int i = 0; i < 100000; ++i ){
                accumulator--;
            }
        }
        static void testB_2(){
            for( int i = 0; i < 100000; ++i ){
                accumulator*=2;
            }
        }
        static void testC_1(Mutex m){
            accumulator = 1;
            m.Lock();
            for( int i = 0; i < 24; ++i ){
                accumulator*=2;
            }
            m.Unlock();
        }
        static void testC_2(Mutex m){
            m.Lock();
            for( int i = 0; i < 100000; ++i ){
                accumulator-=1;
            }
            m.Unlock();
        }
        static void testD_1(){
            accumulator = 0;
        }
        static void testD_2(int a){
            accumulator += a;
        }
        static void testD_3(int a, int b ){
            accumulator += a + b;
        }
        static void testD_4(int a, int b, int c ){
            accumulator += a + b + c;
        }
        static void testD_5(int a, int b, int c, int d ){
            accumulator += a + b + c + d;
        }
        static void testD_6(int a, int b, int c, int d, int e ){
            accumulator += a + b + c + d + e;
        }
        void operator()(int a){
            accumulator = a;
        }
        void testF(int a, int b){
            accumulator2 = a + b;
        }
        static void testG(int m, int n, int b, int v, int c, int x, int z, int l, int k, int j, int h, int g, int f, int d, int s, int a, int p, int o, int i, int u, int y, int t, int r, int e, int w, int q ){
            accumulator = q+w+e+r+t+y+u+i+o+p+a+s+d+f+g+h+j+k+l+z+x+c+v+b+n+m;
        }
        static void testG2(int m, int n, int b, int v, int c, int x, int z, int l, int k, int j, int h, int g, int f, int d, int s, int a, int p, int o, int i, int u, int y, int t, int r, int e, int w, int q, int m1, int n1, int b1, int v1, int c1, int x1, int z1, int l1, int k1, int j1, int h1, int g1, int f1, int d1, int s1, int a1, int p1, int o1, int i1, int u1, int y1, int t1, int r1, int e1, int w1, int q1 ){
            accumulator = q+w+e+r+t+y+u+i+o+p+a+s+d+f+g+h+j+k+l+z+x+c+v+b+n+m+q1+w1+e1+r1+t1+y1+u1+i1+o1+p1+a1+s1+d1+f1+g1+h1+j1+k1+l1+z1+x1+c1+v1+b1+n1+m1;
        }


    };
    int functions::accumulator;
    DLL_EXPORT bool Thread::RunTests(){
        TEST("MUD::Thread");

        Thread testA(functions::testA);
        testA.Run();
        testA.Join();
        ASSERT(functions::accumulator == 100000);


        Thread testB_1(functions::testB_1);
        Thread testB_2(functions::testB_2);
        testB_1.Run();
        testB_1.Join();
        testB_2.Run();
        testB_2.Join();
        ASSERT( functions::accumulator == 0 );

        Mutex testCm;
        testCm.Lock();
        Thread testC_1( std::bind(functions::testC_1, testCm) );
        Thread testC_2( std::bind(functions::testC_2, testCm) );
        testC_1.Run();
        testCm.Unlock();
        Thread::Sleep(100);
        testCm.Lock();
        testC_2.Run();
        testCm.Unlock();
        testC_1.Join();
        testC_2.Join();
        ASSERT( functions::accumulator == (16777216-100000) );

        Thread testD_1( std::bind(functions::testD_1) );
        Thread testD_2( std::bind(functions::testD_2,1) );
        Thread testD_3( std::bind(functions::testD_3,2,3) );
        Thread testD_4( std::bind(functions::testD_4,4,5,6) );
        Thread testD_5( std::bind(functions::testD_5,7,8,9,10) );
        Thread testD_6( std::bind(functions::testD_6,11,12,13,14,15) );
        testD_1.Run(); testD_1.Join();
        testD_2.Run(); testD_2.Join();
        testD_3.Run(); testD_3.Join();
        testD_4.Run(); testD_4.Join();
        testD_5.Run(); testD_5.Join();
        testD_6.Run(); testD_6.Join();
        ASSERT( functions::accumulator == (1+2+3+4+5+6+7+8+9+10+11+12+13+14+15) );

        functions testEf;
        testEf.accumulator2 = 0;
        Thread testE( std::bind(testEf,1337) );
        testE.Run();
        testE.Join();
        ASSERT( functions::accumulator == 1337 );

        functions testFf;
        testFf.accumulator2 = 0;
        Thread testF( std::bind(&functions::testF, &testFf, 100, 101 ) );
        testF.Run();
        testF.Join();
        ASSERT( testFf.accumulator2 == 201 );

        Thread testG( std::bind(functions::testG,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26) );
        testG.Run();
        testG.Join();
        ASSERT( functions::accumulator == 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16+17+18+19+20+21+22+23+24+25+26 );

        Thread testG2( std::bind(functions::testG2,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26) );
        testG2.Run();
        testG2.Join();
        ASSERT( functions::accumulator == 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16+17+18+19+20+21+22+23+24+25+26+1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16+17+18+19+20+21+22+23+24+25+26 );





        return true;
    }
    #endif
}
