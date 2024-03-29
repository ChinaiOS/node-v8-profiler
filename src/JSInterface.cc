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
#include <vector>

// using namespace std;

#include "cpu_profiler/cpu_napi_profiler.h"
#include "cpu_profiler/cpu_profile.h"

#include "nlohmann/json.hpp"
#include "string_convert.h"

using v8::Array;
using v8::CpuProfile;
using v8::CpuProfileNode;
using v8::Function;
using v8::FunctionTemplate;
using v8::Integer;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;


int generateType = 0;
bool recsamples = false;

int started_profiles_count_ = 0;
int profiles_since_last_cleanup_ = 0;
int profiles_clean_limit_ = 0;
v8::CpuProfiler* cpu_profiler_ = nullptr;
uint32_t sampling_interval_ = 0;


void setNodes_(const v8::CpuProfileNode* node,
                            std::vector<Local<Object> >& list) {
	//   Local<Object> profile_node = Nan::New<Object>();
  	int32_t count = node->GetChildrenCount();

	std::vector<unsigned int> children;

	// 创建一个 nlohmann::json 对象
	nlohmann::json jsonArray;

	nlohmann::json profile_node;

	for (int32_t index = 0; index < count; index++) {
		jsonArray.push_back(node->GetChild(index)->GetNodeId());
	}

	 // 将 JSON 数组打印输出
    std::cout << jsonArray.dump() << std::endl;

	
	// Nan::Set(profile_node, Nan::New<String>("id").ToLocalChecked(),
	// 		Nan::New<Integer>(node->GetNodeId()));

	profile_node["id"] = node->GetNodeId();
	// Nan::Set(profile_node, Nan::New<String>("hitCount").ToLocalChecked(),
	// 		Nan::New<Integer>(node->GetHitCount()));

	profile_node["hitCount"] = node->GetHitCount();


	// Local<Object> call_frame = Nan::New<Object>();
	// Nan::Set(call_frame, Nan::New<String>("functionName").ToLocalChecked(),
	// 		node->GetFunctionName());

	profile_node["functionName"] = StringConvert::convertToString(node->GetFunctionName());

	// Nan::Set(call_frame, Nan::New<String>("scriptId").ToLocalChecked(),
	// 		Nan::New<Integer>(node->GetScriptId()));

	profile_node["scriptId"] = node->GetScriptId();



	// Nan::Set(call_frame, Nan::New<String>("bailoutReason").ToLocalChecked(),
	// 		Nan::New<String>(node->GetBailoutReason()).ToLocalChecked());


	profile_node["bailoutReason"] = StringConvert::convertToString(node->GetBailoutReason());

	// #if defined(V8_MAJOR_VERSION) && (V8_MAJOR_VERSION < 8)
	// Nan::Set(call_frame, Nan::New<String>("callUID").ToLocalChecked(),
	// 		Nan::New<Number>(node->GetCallUid()));
	// #endif
	// Nan::Set(call_frame, Nan::New<String>("url").ToLocalChecked(),
	// 		node->GetScriptResourceName());
	// Nan::Set(call_frame, Nan::New<String>("lineNumber").ToLocalChecked(),
	// 		Nan::New<Integer>(node->GetLineNumber()));
	// Nan::Set(call_frame, Nan::New<String>("columnNumber").ToLocalChecked(),
	// 		Nan::New<Integer>(node->GetColumnNumber()));
	// #if (NODE_MODULE_VERSION >= 42)
	// Local<Value> lineTicks = GetLineTicks_(isolate, node);
	// if (!lineTicks->IsNull()) {
	// 	Nan::Set(call_frame, Nan::New<String>("lineTicks").ToLocalChecked(),
	// 			lineTicks);
	// }
	// #endif

	// Nan::Set(profile_node, Nan::New<String>("callFrame").ToLocalChecked(),
	// 		call_frame);
	// Nan::Set(profile_node, Nan::New<String>("children").ToLocalChecked(),
	// 		children);

	// set node
	// list.push_back(profile_node);

	// for (int32_t index = 0; index < count; index++) {
	// 	setNodes_(isolate, node->GetChild(index), list);
	// }
}


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
    	// v8::HandleScope handle_scope(isolate);
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
		std::string title = argv[0].GetString();
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
    	v8::HandleScope handle_scope(isolate);
		v8::Local<v8::String> titleV8 = v8::String::NewFromUtf8(isolate, title.c_str(), v8::NewStringType::kNormal).ToLocalChecked();

		
		// 结束 CPU 分析
		v8::CpuProfile* node = cpu_profiler_->StopProfiling(titleV8);


		// 处理分析结果
		if (node != nullptr) {
			// 输出分析结果
			// v8::String::Utf8Value profile_json(profile->GetTopDownRoot()->GetV8ProfilerWiseRoot()->GetCallUID());
			// printf("%s\n", *profile_json);
			// 保存为json对象
			    // object->SetAlignedPointerInInternalField(0, profile);
			Local<String> titleLocal = node->GetTitle();

			string title = StringConvert::convertToString(titleLocal);

			std::string typeId = "CPU";
			std::string uid = "0";

			// nodes  数组
			std::vector<Local<Object> > list;
			setNodes_(node->GetTopDownRoot(), list);

			// uint32_t size = static_cast<uint32_t>(list.size());
			// Local<Array> nodes = Nan::New<Array>(size);
			// for (uint32_t idx = 0; idx < size; idx++) {
			// 	Nan::Set(nodes, idx, list[idx]);
			// }


			
			// 释放内存
			node->Delete();
		}
    

		// 	      v8::Local<v8::Object> object
		// object->SetPointerInInternalField(index, value);

		//    Local<Object> profile；
		//    profile；

    	// 释放 CpuProfiler
		// node->Delete();
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
