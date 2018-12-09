#include "stdafx.h"

#include "ReplaceTags.h"

#include <tuple>
#include <map>

#include <functional>

using namespace std;


#pragma region HelperFunctions

enum _Trimtype {
	TRIM_LEFT = 0x1,
	TRIM_RIGHT = 0x2,
	TRIM_BOTH = 0x3
};
string &trim(__inout string &Str_, char *characters = " \t", _Trimtype Trimtype = TRIM_BOTH) noexcept;
inline string &trim_left(__inout string &Str, char *characters = " \t") { trim(Str, characters, TRIM_LEFT); }
inline string &trim_right(__inout string &Str, char *characters = " \t") { trim(Str, characters, TRIM_RIGHT); }


static string &trim(string &Str, char *characters, _Trimtype Trimtype) noexcept
{
	int start = 0, end;
	const char *str = Str.c_str();
	for (end = 0; str[end]; end++);
	int end_old = end -= 1;
	char c;
	bool f;

	if (Trimtype&TRIM_LEFT) {
		for (; start <= end; start++) {
			f = true;
			for (int i = 0; c = characters[i]; i++) {
				if (str[start] == c) {
					f = false;
					break;
				}
			}
			if (f) break;
		}
	}
	if (Trimtype&TRIM_RIGHT) {
		for (; end >= start; end--) {
			f = true;
			for (int i = 0; c = characters[i]; i++) {
				if (str[end] == c) {
					f = false;
					break;
				}
			}
			if (f) break;
		}
	}
	if (start == 0 && end == end_old) return Str;

	Str.assign(str + start, end - start + 1);

	return Str;

}

#pragma endregion


static std::tuple<const char *, const char*> GetNextTag(__out std::string &tag, const char *p) noexcept;

size_t ReplaceTags(
	__out std::string& result,
	const char *src,
	const std::map<const std::string, const std::string> &NameAndValuePairs
) noexcept;




//tag:
//    receive the tagname without "<" and ">".
//return value:
//    0: start position of the tag (pointing to "<") ("<<" is omitted). If no "<" is found, return nullptr.
//    1: end position of the tag (pointing to ">"). If no ">" is found, return nullptr.
static std::tuple<const char *, const char*> GetNextTag(__out std::string &tag, const char *p) noexcept
{
	if (!p) return make_tuple(nullptr, nullptr);

	const char *startpos;

	for (; *p; p++) {
		if (p[0] == '<') {
			if (p[1] != '<') {        //found a single "<"
				startpos = p;
				for (p++; *p; p++) {
					if (*p == '>') {
						tag.assign(startpos + 1, p - startpos - 1);
						return make_tuple(startpos, p);
					}
				}

				//no ">" is found
				return make_tuple(startpos, nullptr);

			}
			else {                    //found "<<"
				p++;
				continue;
			}
		}
	}

	//no single "<" is found
	return make_tuple(nullptr, nullptr);
};




//All the strings should be encoded with utf-8.
//result:
//    receive the result string. 
//    you can call result.reserve(...) before this function to boost the performance.
//src:
//    the string to be processed. The leading and trailing space (' ' and '\t')
//    in the tag names is ommitted.
//    you must ensure that "src" is null-terminated.
//NameAndValuePairs:
//    The tagname-and-value pairs. The leading and trailing space (' ' and '\t')
//    in NameAndValuePairs is NOT ommitted.
//return value:
//    the number of tags which have been replaced.
size_t ReplaceTags(
	__out std::string& result,
	const char *src,
	const std::map<const std::string, const std::string> &NameAndValuePairs
) noexcept
{
	if (!src) return 0;

	size_t srclen = strlen(src);
	size_t ret = 0;

	result.clear();
	//copy "str" to "result" until "str" ends or "count" is reached, converting "<<" to "<"
	function<void(const char *start, size_t count)> append = [&result](const char *str, size_t count)
	{
		for (size_t i = 0; str[i] && i < count; i++) {
			char current = str[i];
			char next = str[i + 1];
			if (current == '<' && next == '<') {      //found "<<"
				i++;
				result.append(1, '<');
				continue;
			}
			result.append(1, current);
		}
	};


	const char *tagstartpos = src, *tagendpos = src;
	const char *p = src;
	string tagname;

	while (*p) {          //loop until p reaches the end of the string
		tie(tagstartpos, tagendpos) = GetNextTag(tagname, p);
		if (!tagendpos) {               //no more tag is found or finished
			append(p, -1);                //copy remaining characters
			break;
		}
		append(p, tagstartpos - p);

		trim(tagname);
		auto match = NameAndValuePairs.find(tagname);

		if (match != NameAndValuePairs.end()) {
			//found a tag which needs to be replaced
			append(match->second.c_str(), -1);
			ret++;

		}
		else {
			//found a tag which doesn't need to be replaced
			append(tagstartpos, tagendpos - tagstartpos + 1);

		}

		p = tagendpos + 1;
	}


	return ret;
}





#ifdef _DEBUG

#include <cstdlib>

static const char *str1 = u8R"__(<1><2><<<3><>)__";
static const char *str2 = u8R"__(abc<1><2><<<3><4>abc)__";
static const char *str3 = u8R"__(a<bc<1><2><<<3><4>abc)__";


void ReplaceTags_test(__out std::string &TestResult) noexcept;


void ReplaceTags_test(__out std::string &TestResult) noexcept
{
	string indent;
	indent.reserve(32);
	TestResult.clear();

	function<void()> incindent = [&indent]() {indent.assign(indent.size() + 2, ' '); };
	function<void()> decindent = [&indent]() {if (indent.size() >= 2) indent.assign(indent.size() - 2, ' '); };

	size_t ret;
	string result;
	char tmp[32];

	TestResult += indent + u8R"(ReplaceTags_test started. 1->"AAA" 2->"B" 3->"")" + "\n";

	//----------------test1-------------------

	incindent();
	TestResult += indent + u8"test1: a well-formed src: " + str1 + "\n";
	TestResult += indent + u8"  expected output:\n";
	TestResult += indent + u8"    AAAB<<>\n";
	TestResult += indent + u8"    replaced tag count: 3\n";

	incindent();
	TestResult += indent + u8"test1 output:\n";

	incindent();
	ret = ReplaceTags(result, str1, { {"1","AAA"},{"2","B"},{"3",""} });
	itoa(ret, tmp, 10);
	TestResult += indent + result + "\n";
	TestResult += indent + "replaced tag count: " + tmp + "\n";


	decindent();
	decindent();
	decindent();

	//----------------test2-------------------

	incindent();
	TestResult += indent + u8"test2: a well-formed src: " + str2 + "\n";
	TestResult += indent + u8"  expected output:\n";
	TestResult += indent + u8"    abcAAAB<<4>abc\n";
	TestResult += indent + u8"    replaced tag count: 3\n";

	incindent();
	TestResult += indent + u8"test2 output:\n";

	incindent();
	ret = ReplaceTags(result, str2, { { "1","AAA" },{ "2","B" },{ "3","" } });
	itoa(ret, tmp, 10);
	TestResult += indent + result + "\n";
	TestResult += indent + "replaced tag count: " + tmp + "\n";

	decindent();
	decindent();
	decindent();

	//----------------test3-------------------

	incindent();
	TestResult += indent + u8"test3: a ill-formed src: " + str3 + "\n";
	TestResult += indent + u8"  expected output:\n";
	TestResult += indent + u8"    a<bc<1>B<<4>abc\n";
	TestResult += indent + u8"    replaced tag count: 2\n";

	incindent();
	TestResult += indent + u8"test3 output:\n";

	incindent();
	ret = ReplaceTags(result, str3, { { "1","AAA" },{ "2","B" },{ "3","" } });
	itoa(ret, tmp, 10);
	TestResult += indent + result + "\n";
	TestResult += indent + "replaced tag count: " + tmp + "\n";

	decindent();
	decindent();
	decindent();

	TestResult += indent + u8"ReplaceTags_test finished.\n";

}

#endif

