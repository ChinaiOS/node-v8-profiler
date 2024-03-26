#pragma once
#define NAPI_EXPERIMENTAL
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <node_api.h>


#include <string>
#include <list>
#include <vector>

#define SAFE_DELETE_ARRAY(pArr) if(pArr){ delete[] pArr; pArr = NULL; }
#define SAFE_DELETE_OBJ(pObj) if(pObj){ delete pObj; pObj = NULL; }

enum cpp_type
{
	cpp_boolean,
	cpp_int,
	cpp_uint,
	cpp_double,
	cpp_string,
	cpp_object,
	cpp_array,
};
union unionValue
{
	double doubleValue;
	bool boolValue;
	char *strValue;
	napi_value napiValue;
};

template <class T1>
struct STParamList
{
	int paramNum = 0;
	std::vector<T1 *> pList;
	void addParam(T1 *pParam)
	{
		paramNum++;
		pList.push_back(pParam);
	}
};
typedef struct _CPPParam_
{
	cpp_type type;
	std::string strValue = "";
	std::vector<std::string> vecValue;
	int64_t intValue = 0;
	bool bValue = false;

	//添加拷贝构造,把vector深拷贝
	_CPPParam_(const _CPPParam_ & other) : type(other.type), strValue(other.strValue), intValue(other.intValue), bValue(other.bValue)
	{
		vecValue = other.vecValue;
	}
	_CPPParam_(const std::string intput) : strValue(intput)
	{
		type = cpp_string;
	}
	_CPPParam_(int64_t intput) : intValue(intput)
	{
		type = cpp_int;
	}
	_CPPParam_(const bool intput, cpp_type t)
	{
		bValue = intput;
		type = cpp_boolean;
	}
	napi_value GetNapiValue(napi_env env)
	{
		napi_value ret;
		if (type == cpp_boolean)
		{
			napi_get_boolean(env, bValue, &ret);
		}
		else if (type == cpp_int)
		{
			napi_create_int64(env, intValue, &ret);
		}
		else if (type == cpp_string)
		{
			napi_create_string_utf8(env, strValue.c_str(), strValue.length(), &ret);
		}
		else if (type == cpp_array)
		{
			napi_create_array(env, &ret);
			for (int i = 0; i < vecValue.size(); i++)
			{
				napi_value str;
				napi_create_string_utf8(env, vecValue[i].c_str(), vecValue[i].length(), &str);
				napi_set_element(env, ret, i, str);
			}
		}
		else if (type == cpp_uint)
		{
		}
		else if (type == cpp_double)
		{
		}
		else if (type == cpp_object)
		{
		}
		return ret;
	}
} CPPParam;
typedef STParamList<CPPParam> CPPParamList;

void deleteCPPParamList(CPPParamList *pCPPParmList)
{
	if (NULL == pCPPParmList) return;
	//回收CPPParmList
	for (int j = 0; j < pCPPParmList->pList.size(); ++j)
	{
		delete pCPPParmList->pList[j];
		pCPPParmList->pList[j] = NULL;
	}
	pCPPParmList->pList.clear();
	delete pCPPParmList;
	pCPPParmList = NULL;
}
typedef struct _STThreadsafeFunction_
{
	void CallThreadSafeFunction(CPPParamList *pCPPParmList)
	{
		napi_status status = napi_acquire_threadsafe_function(m_callBack);
		assert(status == napi_ok);
		status = napi_call_threadsafe_function(m_callBack, pCPPParmList, napi_tsfn_blocking);
	}
	napi_value work_name;
	napi_threadsafe_function m_callBack;
} STThreadsafeFunction;

extern void CallJs(napi_env env, napi_value js_cb, void *context, void *data);
typedef struct _JSParam_
{
	napi_value value;
	napi_env env;
	_JSParam_()
	{
	}
	_JSParam_(napi_env in_env, napi_value in_napiValue) : env(in_env), value(in_napiValue)
	{
	}
	napi_valuetype GetType()
	{
		napi_valuetype jsType;
		napi_status status = napi_typeof(env, value, &jsType);
		assert(status == napi_ok);
		return jsType;
	}
	std::string GetString()
	{
		size_t len;
		napi_value result;
		napi_status status1 = napi_coerce_to_string(env, value, &result);
		// 目前只能接收128 * 1024长度的数据 by zhangchao
		napi_status status = napi_get_value_string_utf8(env, result, NULL, 128 * 1024, &len);
		char *pCS = (char *)malloc(len + 1);
		status = napi_get_value_string_utf8(env, result, pCS, 128 * 1024, &len);
		pCS[len] = '\0';  // 添加字符串终止符
		std::string strRet = std::string(pCS, 0, len);
		free(pCS);
		return strRet;
	}

	std::list<int32_t> GetListInt()
	{
		uint32_t len;
		std::list<int32_t> ret;
		napi_get_array_length(env, value, &len);
		for (int i = 0; i < len; i++)
		{
			napi_value e;
			napi_get_element(env, value, i, &e);
			int32_t value = 0;
			napi_get_value_int32(env, e, &value);
			ret.push_back(value);
		}
		return ret;
	}
	std::list<std::string> GetListStr()
	{
		uint32_t len;
		napi_get_array_length(env, value, &len);
		std::list<std::string> ret;
		for (int i = 0; i < len; i++)
		{
			napi_value e;
			napi_get_element(env, value, i, &e);
			char szValue[64] = { 0 };
			size_t szLen;
			napi_get_value_string_utf8(env, e, szValue, 64, &szLen);
			ret.push_back(std::string(szValue, 0, szLen));
		}
		return ret;
	}
	size_t GetArray(char **array)
	{
		size_t len;
		napi_status status = napi_get_arraybuffer_info(env, value, (void **)array, &len);
		return len;
	}
	const char *GetCStr()
	{
		size_t len;
		napi_status status = napi_get_value_string_utf8(env, value, NULL, 64 * 1024, &len);
		char *pCS = (char *)malloc(len);
		status = napi_get_value_string_utf8(env, value, pCS, 64 * 1024, &len);
		std::string strRet = std::string(pCS, 0, len);
		free(pCS);
		return strRet.c_str();
	}
	bool GetBoolen()
	{
		bool ret;
		napi_status status = napi_get_value_bool(env, value, &ret);
		return ret;
	}
	int64_t GetInt()
	{
		int64_t ret;
		napi_status status = napi_get_value_int64(env, value, &ret);
		return ret;
	}
	bool CallFunction(CPPParam *paramList, int num)
	{
		napi_value *argv = new napi_value[num];
		for (int i = 0; i < num; i++)
		{
			if (paramList[i].type == cpp_string)
			{
				napi_create_string_utf8(env, paramList[i].strValue.c_str(), NAPI_AUTO_LENGTH, argv + i);
			}
		}
		napi_value global;
		napi_get_global(env, &global);
		napi_value result;
		napi_call_function(env, global, value, num, argv, &result);
		SAFE_DELETE_ARRAY(argv);
		return true;
	}
	STThreadsafeFunction *GenThreadSafeFunction()
	{
		STThreadsafeFunction *pSTThreadsafeFunction = new STThreadsafeFunction();
		napi_status status;
		napi_value work_name;
		status = napi_create_string_utf8(env, "Node-API Thread-safe Call from Async Work Item",
			NAPI_AUTO_LENGTH,
			&work_name);
		assert(status == napi_ok);
		status = napi_create_threadsafe_function(env, value, NULL, work_name, 0, 1,
			NULL, NULL, NULL, CallJs, &pSTThreadsafeFunction->m_callBack);
		assert(status == napi_ok);
		return pSTThreadsafeFunction;
	}
} JSParam;

typedef STParamList<JSParam> JSParamList;
typedef CPPParam *(*MethodType)(napi_env, JSParam *argv, int argc);
typedef struct _ST_FUNCTION_
{
	MethodType m_method;
	char m_name[32] = { 0 };
	_ST_FUNCTION_(MethodType in_method, const char* name)
	{
		m_method = in_method;
		strcpy(m_name, name);
	}
}ST_FUNCTION;


void CallJs(napi_env env, napi_value js_cb, void *context, void *data)
{
	// This parameter is not used.
	(void)context;
	napi_status status;
	// items.
	if (env == NULL)
	{
		return;
	}
	napi_value undefined;
	status = napi_get_undefined(env, &undefined);
	assert(status == napi_ok);
	CPPParamList *pCPPParmList = (CPPParamList *)data;
	// printf("num of CPPParam=%d\n", pCPPParmList->paramNum);
	napi_value *paramList = new napi_value[pCPPParmList->paramNum];
	for (int i = 0; i < pCPPParmList->paramNum; i++)
	{
		*(paramList + i) = pCPPParmList->pList[i]->GetNapiValue(env);
	}
	status = napi_call_function(env, undefined, js_cb, pCPPParmList->paramNum, paramList, NULL);
	if ((status) != napi_ok)
	{
		const napi_extended_error_info *error_info;
		napi_get_last_error_info((env), &error_info);
		bool is_pending;
		const char* err_message = error_info->error_message;
		napi_is_exception_pending((env), &is_pending);
		if (!is_pending)
		{
			const char* error_message = err_message != NULL ?
				err_message :
				"empty error message";
			napi_throw_error((env), NULL, error_message);
		}
		//return;
	}
	SAFE_DELETE_ARRAY(paramList);
	deleteCPPParamList(pCPPParmList);
}



napi_value Call_CPP_METHOD(napi_env env, napi_callback_info info)
{
	static std::string arrayNapiType[] = {
		"napi_undefined",
		"napi_null",
		"napi_boolean",
		"napi_number",
		"napi_string",
		"napi_symbol",
		"napi_object",
		"napi_function",
		"napi_external",
		"napi_bigint"
	};
	//MethodType m_method = NULL;
	ST_FUNCTION *pMethod;
	size_t argc;
	napi_value this_arg;
	napi_status status = napi_get_cb_info(env, info, &(argc), NULL, &this_arg, (void **)(&pMethod));
	//xassert2(status == napi_ok && pMethod != NULL);
	if (status != napi_ok || pMethod == NULL)
	{
		return NULL;
	}
	napi_value *argv = (argc > 0) ? new napi_value[argc] : NULL;
	status = napi_get_cb_info(env, info, &(argc), argv, NULL, NULL);
	if (status != napi_ok)
	{
		SAFE_DELETE_ARRAY(argv);
		return NULL;
	}
	//xlog2(kLevelInfo, "[%s::%s", __FUNCTION__, pMethod->m_name);
	JSParam *pParmList = new JSParam[argc];
	for (int i = 0; i < argc; i++)
	{
		pParmList[i].env = env;
		pParmList[i].value = argv[i];
		//xlog2(kLevelInfo, "argv[%d].paramType=%s",i, arrayNapiType[pParmList[i].GetType()].c_str());
	}
	napi_value ret = NULL;
	CPPParam *pCPPParam = pMethod->m_method(env, pParmList, argc);
	if (pCPPParam)
	{
		ret = pCPPParam->GetNapiValue(env);
	}
	SAFE_DELETE_OBJ(pCPPParam);
	SAFE_DELETE_ARRAY(pParmList);
	SAFE_DELETE_ARRAY(argv);
	return ret;
}



