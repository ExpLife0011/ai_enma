
#pragma once

#include <stdio.h>
#include <tchar.h>
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

#include "..\ai_enma\ai_enma.h"
#ifndef _M_X64
  #ifdef _DEBUG
    #pragma comment(lib,"..\\Debug\\ai_enma.lib")
  #else
	#pragma comment(lib,"..\\Release\\ai_enma.lib")
  #endif
#else
  #ifdef _DEBUG
	#pragma comment(lib,"..\\x64\\Debug\\ai_enma.lib")
  #else
	#pragma comment(lib,"..\\x64\\Release\\ai_enma.lib")
  #endif
#endif