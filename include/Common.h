// Common.h
#ifndef COMMON_H
#define COMMON_H
#pragma once
#include <wx/wx.h>

#define wxT_UTF8(str) wxString::FromUTF8(str)

#endif // COMMON_H

// 转换成小写(符合腾讯证券)
static std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}


// 用于动态字符串处理的内存块结构
struct MemoryBlock {
    char* data;
    size_t size;

    MemoryBlock() {
        data = (char*)malloc(1);
        size = 0;
    }

    ~MemoryBlock() {
        if (data) {
            free(data);
        }
    }
};

// 用于写入接收到的数据的回调函数
static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    MemoryBlock* mem = (MemoryBlock*)userp;

    char* ptr = (char*)realloc(mem->data, mem->size + total_size + 1);
    if (!ptr) {
        throw std::runtime_error(_("Failed to allocate memory"));
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, total_size);
    mem->size += total_size;
    mem->data[mem->size] = 0;

    return total_size;
}
