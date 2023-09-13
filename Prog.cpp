//https://habr.com/ru/companies/simbirsoft/articles/521018/

#include "stdafx.h"
#include <iostream>
using namespace std;

class Finalizer
{
    struct Data
    {
        int i = 0;
        char* c = nullptr;
        
        union U
        {
            long double d;
            
            int i[sizeof(d) / sizeof(int)]; //------------ sizeof(d) (где d — это long double) не обязательно кратен sizeof(int)
            
            char c [sizeof(i)];
        } u = {};
        
        time_t time;  //-------------------time_t — это макрос, в Visual Studio он может ссылаться на 32-битный (старый) или 64-битный (новый) целочисленный тип
    };
    
    struct DataNew;
    DataNew* data2 = nullptr;
    
    typedef DataNew* (*SpawnDataNewFunc)();
    SpawnDataNewFunc spawnDataNewFunc = nullptr;
    
    typedef Data* (*Func)();
    Func func = nullptr;
    
    Finalizer()
    {
        func = GetProcAddress(OTHER_LIB, "func") //------------ проблема с порядком инициализации статических объектов (SIOF): (объект OTHER_LIB в коде используется раньше, чем он был инициализирован)
        
        auto data = func(); //-------------------------- возвращенный ранее результат не проверяется перед использованием
      //пытаясь разыменовать его, вместо ожидаемого вызова функции мы получим ошибку нарушения прав доступа (access violation) или ошибку защиты памяти
        
        auto str = data->c;
        
        memset(str, 0, sizeof(str)); //----------------------------------- использование размера указателя на массив вместо размера самого массива 
        
        data->u.d = 123456.789;
        
        const int i0 = data->u.i[sizeof(long double) - 1U];
        
        spawnDataNewFunc = GetProcAddress(OTHER_LIB, "SpawnDataNewFunc")
        data2 = spawnDataNewFunc();
    }
    
    ~Finalizer()
    {
        auto data = func();
        
        delete[] data2;
    }
};

Finalizer FINALIZER;

HMODULE OTHER_LIB;
std::vector<int>* INTEGERS;

DWORD WINAPI Init(LPVOID lpParam)
{
    OleInitialize(nullptr);
    
    ExitThread(0U);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    static std::vector<std::thread::id> THREADS;
    
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            
            srand(time(nullptr));
            
            OTHER_LIB = LoadLibrary("B.dll");
            
            if (OTHER_LIB = nullptr)
                return FALSE;
            
            CreateThread(nullptr, 0U, &Init, nullptr, 0U, nullptr);
        break;
        
        case DLL_PROCESS_DETACH:
            CoUninitialize();
            
            OleUninitialize();
            {
                free(INTEGERS);
                
                const BOOL result = FreeLibrary(OTHER_LIB);
                
                if (!result)
                    throw new std::runtime_error("Required module was not loaded");
                
                return result;
            }
        break;
        
        case DLL_THREAD_ATTACH:
            THREADS.push_back(std::this_thread::get_id());
        break;
        
        case DLL_THREAD_DETACH:
            THREADS.pop_back();
        break;
    }
    return TRUE;
}

__declspec(dllexport) int Initialize(std::vector<int> integers, int& c) throw()
{
    for (int i : integers)
        i *= c;
    
    INTEGERS = new std::vector<int>(integers);
}

int Random()
{
    return rand() + rand();
}

__declspec(dllexport) long long int __cdecl _GetInt(int a)
{
    return 100 / a <= 0 ? a : a + 1 + Random();
}
