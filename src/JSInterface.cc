#include "JSInterface.h"
// #include <sstream>
#include <random>
#include <codecvt>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>

#include "cpu_profiler/cpu_napi_profiler.h"
#include "cpu_profiler/cpu_profile.h"

using namespace std;

int generateType = 0;
bool recsamples = false;

int started_profiles_count_ = 0;
int profiles_since_last_cleanup_ = 0;
int profiles_clean_limit_ = 0;
v8::CpuProfiler* cpu_profiler_ = nullptr;
uint32_t sampling_interval_ = 0;


namespace napi {
	CPPParam *setGenerateType(napi_env, JSParam *argv, int argc)
	{
		if (argc < 1) {
			std::cout << __FUNCTION__ << ":" << "wrong number of parameters" << std::endl;
			return NULL;
		}
		// generateType
		generateType = argv[0].GetInt();
		return NULL;
	}

	CPPParam *startProfiling(napi_env, JSParam *argv, int argc)
	{
		if (argc < 1) {
			std::cout << __FUNCTION__ << ":" << "wrong number of parameters" << std::endl;
			return NULL;
		}

		std::string title = argv[0].GetString();
		recsamples = argv[1].GetBoolen();


		v8::Isolate* isolate = v8::Isolate::GetCurrent();
    	v8::HandleScope handle_scope(isolate);
		cpu_profiler_ = v8::CpuProfiler::New(isolate);

		v8::Local<v8::String> titleV8 = v8::String::NewFromUtf8(isolate, title.c_str(), v8::NewStringType::kNormal).ToLocalChecked();


		cpu_profiler_ -> StartProfiling(titleV8, recsamples);
		return NULL;
	}

	CPPParam *stopProfiling(napi_env, JSParam *argv, int argc)
	{
		if (argc < 1) {
			std::cout << __FUNCTION__ << ":" << "wrong number of parameters" << std::endl;
			return NULL;
		}
		string title = argv[0].GetString();
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
    	v8::HandleScope handle_scope(isolate);
		v8::Local<v8::String> titleV8 = v8::String::NewFromUtf8(isolate, title.c_str(), v8::NewStringType::kNormal).ToLocalChecked();

		
		// 结束 CPU 分析
		v8::CpuProfile* profile = cpu_profiler_->StopProfiling(titleV8);


		// 处理分析结果
		if (profile != nullptr) {
			// 输出分析结果
			// v8::String::Utf8Value profile_json(profile->GetTopDownRoot()->GetV8ProfilerWiseRoot()->GetCallUID());
			// printf("%s\n", *profile_json);
			
			// 释放内存
			profile->Delete();
		}
    

		// 	      v8::Local<v8::Object> object
		// object->SetPointerInInternalField(index, value);

		//    Local<Object> profile；
		//    profile；

    	// 释放 CpuProfiler
		profile->Delete();
  		--started_profiles_count_;
		if (!started_profiles_count_ && profiles_since_last_cleanup_ > profiles_clean_limit_) {
			// logger(this->isolate(), "clear cpuprofiler: %d\n", profiles_clean_limit_);
			cpu_profiler_->Dispose();
			cpu_profiler_ = nullptr;
			profiles_since_last_cleanup_ = 0;
		} else {
			// TODO 报错信息
			// logger(this->isolate(), "not clear cpuprofiler: %d, %d\n",
			// 	started_profiles_count_, profiles_clean_limit_);
		}

		return NULL;
	}
}



//napi_value Call_CPP_METHOD(napi_env env, napi_callback_info info);
// #include <napi.h>
#define DECLARE_NAPI_METHOD(method)                                    \
{                                                                      \
    #method, NULL, Call_CPP_METHOD, NULL, NULL, NULL, napi_enumerable, new ST_FUNCTION(napi::method,#method) \
}

napi_property_descriptor arrayMethods[] = {
    DECLARE_NAPI_METHOD(setGenerateType),
    DECLARE_NAPI_METHOD(startProfiling),
    DECLARE_NAPI_METHOD(stopProfiling)
};

// Module initialization.
static napi_value InitNapiMethod(napi_env env, napi_value exports)
{
	// Declare the above three bindings.
	// Attach them to the exports object.
	napi_status status = napi_define_properties(
		env, exports, sizeof(arrayMethods) / sizeof(*arrayMethods), arrayMethods);
	assert(status == napi_ok);
	// Return the newly adorned exports object.
	return exports;
}
// Mark this as a N-API module.
NAPI_MODULE(NODE_GYP_MODULE_NAME, InitNapiMethod)
