#include "file.h"

File::File(const char* FileName)
{
	fopen_s(&file, FileName, "w+");
}

File::~File()
{
	if (file)
		fclose(file);
}

int File::fprintf(string str)
{
	return std::fprintf(file, str.c_str());
    
}

int File::fflush()
{
    return ::fflush(file);
}

//void show_str(const char* pstr, ...)
int File::fprintf(const char* format, ...)
{
    va_list args;

    va_start(args, format);
    int ret = vfprintf(file, format, args);
    va_end(args);

    return ret;

}



FILE* File::GetFile()
{
	return file;
}