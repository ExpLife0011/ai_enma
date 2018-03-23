
#pragma once
#define WIN32_LEAN_AND_MEAN  

#include <Windows.h>
#include <winternl.h>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <time.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iostream>





#ifndef _M_X64
  #ifdef _DEBUG
    #pragma comment(lib,"..\\..\\enma_pe\\Debug\\enma_pe.lib")
  #else
	#pragma comment(lib,"..\\..\\enma_pe\\Release\\enma_pe.lib")
  #endif
#else
  #ifdef _DEBUG
	#pragma comment(lib,"..\\..\\enma_pe\\x64\\Debug\\enma_pe.lib")
  #else
	#pragma comment(lib,"..\\..\\enma_pe\\x64\\Release\\enma_pe.lib")
  #endif
#endif


#define ALIGN_UP(a, b) (((a + b - 1) / b) * b)

#include "ai_enma.h"