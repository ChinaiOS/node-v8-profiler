#include "string_convert.h"

StringConvert::StringConvert(/* args */)
{
}

StringConvert::~StringConvert()
{
}

string StringConvert::convertToString(Local<String> str) {
    Isolate* isolate = Isolate::GetCurrent();
    String::Utf8Value utf8(isolate, str);
    return string(*utf8);
}