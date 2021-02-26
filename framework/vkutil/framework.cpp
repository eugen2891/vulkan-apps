#include "framework.h"

#include <stdio.h>

#include <sys/stat.h>

char* vkutil::LoadFileAsString(const char* pFileName, size_t* pSize)
{
    char* pRetVal = nullptr;
    FILE* pFile = fopen(pFileName, "rb");
    if (pFile)
    {
        struct stat fileStat;
        int fileNo = _fileno(pFile);
        if (fstat(fileNo, &fileStat) == 0)
        {
            char* pBuffer = new(std::nothrow) char[fileStat.st_size + 1];
            if (fread(pBuffer, fileStat.st_size, 1, pFile) == 1)
            {
                pBuffer[fileStat.st_size] = 0;
                pRetVal = pBuffer;
            }
            else
                delete[] pBuffer;            
        }
        fclose(pFile);
    }
    return pRetVal;
}
