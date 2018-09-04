#include <string>
#include <codecvt>
#include "gbkToUtf8.h"


const std::string gbkToUtf8(const std::string & str) {
	using Codecvt = std::codecvt_byname<wchar_t, char, std::mbstate_t>;
	const char * GBK_LOCAL_NAME = ".936";	//
	std::wstring_convert<Codecvt> conv1(new Codecvt(GBK_LOCAL_NAME));
	std::wstring temp_wstr = conv1.from_bytes(str);
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv2;
	std::string utf8Str = conv2.to_bytes(temp_wstr);
	return utf8Str;
}