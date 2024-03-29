#pragma once

#include <node_api.h>

#include <string>

#include <v8.h>

// using v8::Array;
// using v8::CpuProfile;
// using v8::CpuProfileNode;
// using v8::Function;
// using v8::FunctionTemplate;
// using v8::Integer;
// using v8::Local;
// using v8::Number;
// using v8::Object;
// using v8::ObjectTemplate;
// using v8::String;
// using v8::Value;

using namespace std;

using namespace v8;




class StringConvert
{
private:
    /* data */
public:
    StringConvert(/* args */);
    static string convertToString(Local<String> str);
    ~StringConvert();
};

