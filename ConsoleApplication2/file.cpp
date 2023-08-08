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
int File::fprintf(const char* pstr, ...)
{
    va_list ap;
    va_start(ap, pstr);

    // 1、计算得到长度
    //---------------------------------------------------
    // 返回 成功写入的字符个数
    int count_write = snprintf(NULL, 0, pstr, ap);
    va_end(ap);

    // 长度为空
    if (0 >= count_write)
        return -1;

    count_write++;

    // 2、构造字符串再输出
    //---------------------------------------------------
    va_start(ap, pstr);

    char* pbuf_out = NULL;
    pbuf_out = (char*)malloc(count_write);
    if (NULL == pbuf_out)
    {
        va_end(ap);
        return -1;
    }

    // 构造输出
    vsnprintf(pbuf_out, count_write, pstr, ap);
    // 释放空间
    va_end(ap);

    // 输出结果
    std::cout << "str = " << pbuf_out << "\n";
    std::fprintf(file, pbuf_out);

    // 释放内存空间
    free(pbuf_out);
    pbuf_out = NULL;
}



FILE* File::GetFile()
{
	return file;
}