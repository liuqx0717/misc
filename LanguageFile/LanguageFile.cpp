#include "stdafx.h"
#include "LanguageFile.h"

#include <map>
#include <tuple>


using namespace std;


#pragma region HelperFunctions


enum _Trimtype {
	TRIM_LEFT = 0x1,
	TRIM_RIGHT = 0x2,
	TRIM_BOTH = 0x3
};
string &trim(__inout string &Str_, char *characters = " \t", _Trimtype Trimtype = TRIM_BOTH) noexcept;
inline string &trim_left(__inout string &Str, char *characters = " \t") { trim(Str, characters, TRIM_LEFT); }
inline string &trim_right(__inout string &Str,char *characters = " \t") { trim(Str, characters, TRIM_RIGHT); }

bool IsSubString(const char *p, const char *substr) noexcept;
size_t IsLineEnding(const char *p) noexcept;







static bool IsSubString(const char *p, const char *substr) noexcept
{
	if (!p || !substr) return false;
	do {
		if (*p != *substr) return false;
		p++;
		substr++;
	} while (*substr);
	return true;
}

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

static size_t IsLineEnding(const char *p) noexcept
{
	if (!p) return 0;
	if (p[0] == '\n') return 1;
	if (p[0] == '\r'&&p[1] != '\n') return 1;
	if (p[0] == '\r'&&p[1] == '\n') return 2;
	return 0;
}


#pragma endregion














//tag:
//    receive the tagname without "<" and ">".
//return value:
//    0: start position of the tag (pointing to "<") ("<<" is omitted). If no "<" is found, return nullptr.
//    1: end position of the tag (pointing to ">"). If no ">" is found, return nullptr.
tuple<const char *, const char*> GetNextTag(__out string &tag, const char *p) noexcept;


//tag:
//    receive the tagname without "<" and ">".
//return value:
//    0: start position of the tag (pointing to "<") ("<<" is omitted). If no "<" is found, return nullptr.
//    1: end position of the tag (pointing to ">"). If no ">" is found, return nullptr.
static tuple<const char *, const char*> GetNextTag(__out string &tag, const char *p) noexcept
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


//result:
//    "string name / string value" pairs.
//    all these strings should be encoded with utf-8.
//buffer:
//    the entire content read from a language file. 
//    the language file should be encoded with utf-8.
//    you must ensure that "buffer" is null-terminated.
//LineEndingNew: 
//    convert all the original line endings("\r" or "\n" or "\r\n") to LineEndingNew.
//    pass nullptr to keep the original line endings.
//    pass "\0" to erase all the original line endings.
//greedy:
//    force searching the entire buffer, in case there are duplicate tags.
//return value:
//    return result.size()
//remarks:
//    the leading/trailing space (space and \t) in tags read from file will be ignored.
//    the leading/trailing space (space and \t) in parameter "LanguageName" will be ignored.
size_t ParseLangFile(
	__out std::map<std::string, std::string> &result,
	const char *buffer,
	const char *LanguageName,
	const char *LineEndingNew,
	bool greedy
) noexcept
{
	if (!buffer || !LanguageName) return 0;

	result.clear();

	string tagname;
	string langname(LanguageName);
	trim(langname);

	string strname;
	string strcontent;
	strcontent.reserve(4096);

	const char *tagstartpos = buffer, *tagendpos = buffer;
	const char *contentstartpos = buffer, *contentendpos = buffer;
	const char *p = buffer;

	int state = 0;
	size_t newlineendinglen = 0;
	if (LineEndingNew) newlineendinglen = strlen(LineEndingNew);
	size_t currentlineendinglen = 0;


	while (*p) {          //loop until p reaches the end of the string
		tie(tagstartpos, tagendpos) = GetNextTag(tagname, p);
		if (!tagendpos) {               //no more tag is found or finished
			break;
		}

		trim(tagname);
		p = tagendpos + 1;

		switch (state)
		{
		case 0:
			if (tagname == langname) {    //found the tag <LanguageName>
				state = 1;

			}
			break;

		case 1:
			if (tagname[0] != '/') {      //found a tag <stringname>
										  // copy string name to strname
				strname.assign(tagname);
				contentstartpos = tagendpos + 1;
				state = 2;

			}
			else {                        //found a tag </...>
				state = 0;
				//if not greedy, exit this function immediately
				if (!greedy) {
					return result.size();
				}

			}
			break;

		case 2:
			if (tagname[0] == '/') {      //found a tag </...>
				contentendpos = tagstartpos;
				size_t contentsize = contentendpos - contentstartpos;


				//copy characters between strstartpos(included strstartpos) and 
				//strendpos(included strendpos) to strcontent.
				strcontent.clear();
				for (size_t i = 0; i < contentsize; i++) {
					char current = contentstartpos[i];
					char next = contentstartpos[i + 1];

					if (current == '<' && next == '<') {      //found "<<"
						i++;
						strcontent.append(1, '<');
						continue;
					}
					else {

						if (LineEndingNew) {                      //line ending conversion needed
							currentlineendinglen = IsLineEnding(contentstartpos + i);
							if (currentlineendinglen) {           //found line ending
																  //copy new line ending to strcontentptr
								strcontent.append(LineEndingNew);

								i += currentlineendinglen - 1;
								//now i points to the last character in the original line ending

								continue;

							}

						}

						//found ordinary character
						strcontent.append(1, current);
					}
				}

				//add "stringname,stringcontent" pair to result
				result.insert(pair<string, string>(string(strname), string(strcontent)));

				state = 1;
			}
			else {                     //if another tag (not </...>) is found, it will be omitted
				state = 2;
				continue;

			}
			break;

		default:
			break;
		}  //switch


	}   //while

	return result.size();

}












#pragma region Test



#ifdef _DEBUG
// test

#include <Windows.h>
#include <locale>
#include <functional>
#include <iostream>

//a well-formed file, encoded by utf-8.
static const char *str1 = u8R"__(
<   en   >
<s1>string1
9<<10 3>2</s1>
<s2>
   string2
</s2>
</enenen>

<chs>
<s1>字符串1 معدل خطأ القراءة</s1>
<s2>字符串2 😊週末繪圖 อัตโนมัติ</s2>
</chs>

<en>
<s1>duplicate</>
<s3>
   string3
</>
</>

)__";

//a well-formed file, encoded by utf-8, with different line endings.
static const char *str2 =
u8"<en>"
u8"<s1>"
u8"123\r456\n789\r\n\n"
u8"</s1>"
u8"</en>";

//an ill-formed file, encoded by utf-8.
static const char *str3 = u8R"__(
<   en   
<s1>string1
9<<10 3>2</s1>
<s2>
   string2
</s2
</enenen>

<chs>
<s1>字符串1 معدل خطأ القراءة</s1>
<s2>字符串2 😊週末繪圖 อัตโนมัติ</s2>
</chs>

<en>
<s1>duplicate<<>
<s3
   string3


)__";




//TestResult is encoded by utf-8
void ParseLangFile_test(__out string &TestResult) noexcept
{
	string indent;
	indent.reserve(32);
	TestResult.clear();

	function<void()> incindent = [&indent]() {indent.assign(indent.size() + 2, ' '); };
	function<void()> decindent = [&indent]() {if (indent.size() >= 2) indent.assign(indent.size() - 2, ' '); };

	map<string, string> mapresult;

	TestResult += indent + u8"ParseLangFile_test started.\n";

	//----------------test1-------------------

	incindent();
	TestResult += indent + u8"test1: a well-formed file, not greedy, convert line endings to \"\\n\".\n";
	TestResult += indent + u8"  expected output:\n";
	TestResult += indent + u8"    <en>\n";
	TestResult += indent + u8"    s1=string1\\n9<10 3>2\n";
	TestResult += indent + u8"    s2=\\n   string2\\n\n";
	TestResult += indent + u8"    </en>\n";
	incindent();
	TestResult += indent + u8"test1 output:\n";

	incindent();
	if (!ParseLangFile(mapresult, str1, " en \t", "\\n", false)) {
		TestResult += indent + u8"ERROR: the size of result is 0.\n";
	}
	else {
		TestResult += indent + u8"<en>\n";
		for (auto pair : mapresult) {
			TestResult += indent +	pair.first + '=' + pair.second + '\n';

		}
		TestResult += indent + u8"</en>\n";

	}
	decindent();
	decindent();
	decindent();

	//----------------test2-------------------

	incindent();
	TestResult += indent + u8"test2: a well-formed file, greedy, convert line endings to \"\\n\".\n";
	TestResult += indent + u8"  expected output:\n";
	TestResult += indent + u8"    <en>\n";
	TestResult += indent + u8"    s1=string1\\n9<10 3>2\n";
	TestResult += indent + u8"    s2=\\n   string2\\n\n";
	TestResult += indent + u8"    s3=\\n   string3\\n\n";
	TestResult += indent + u8"    </en>\n";
	incindent();
	TestResult += indent + u8"test2 output:\n";

	incindent();
	if (!ParseLangFile(mapresult, str1, "en", "\\n", true)) {
		TestResult += indent + u8"ERROR: the size of result is 0.\n";
	}
	else {
		TestResult += indent + u8"<en>\n";
		for (auto pair : mapresult) {
			TestResult += indent + pair.first + '=' + pair.second + '\n';

		}
		TestResult += indent + u8"</en>\n";

	}
	decindent();
	decindent();
	decindent();

	//----------------test3-------------------

	incindent();
	TestResult += indent + u8"test3: a well-formed file, not greedy, with Chinese, Arbic ...\n";
	TestResult += indent + u8"  expected output:\n";
	TestResult += indent + u8"    <chs>\n";
	TestResult += indent + u8"    s1=字符串1 معدل خطأ القراءة\n";
	TestResult += indent + u8"    s2=字符串2 😊週末繪圖 อัตโนมัติ\n";
	TestResult += indent + u8"    </chs>\n";
	incindent();
	TestResult += indent + u8"test3 output:\n";

	incindent();
	if (!ParseLangFile(mapresult, str1, " chs \t", "\\n", false)) {
		TestResult += indent + u8"ERROR: the size of result is 0.\n";
	}
	else {
		TestResult += indent + u8"<chs>\n";
		for (auto pair : mapresult) {
			TestResult += indent + pair.first + '=' + pair.second + '\n';

		}
		TestResult += indent + u8"</chs>\n";

	}
	decindent();
	decindent();
	decindent();

	//----------------test4-------------------

	incindent();
	TestResult += indent + u8"test4: a well-formed file, with different line endings, convert line endings to \"\\n\".\n";
	TestResult += indent + u8"  expected output:\n";
	TestResult += indent + u8"    <en>\n";
	TestResult += indent + u8"    s1=123\\n456\\n789\\n\\n\n";
	TestResult += indent + u8"    </en>\n";
	incindent();
	TestResult += indent + u8"test4 output:\n";

	incindent();
	if (!ParseLangFile(mapresult, str2, " en \t", "\\n", false)) {
		TestResult += indent + u8"ERROR: the size of result is 0.\n";
	}
	else {
		TestResult += indent + u8"<en>\n";
		for (auto pair : mapresult) {
			TestResult += indent + pair.first + '=' + pair.second + '\n';

		}
		TestResult += indent + u8"</en>\n";

	}
	decindent();
	decindent();
	decindent();

	//----------------test5-------------------

	incindent();
	TestResult += indent + u8"test5: an ill-formed file.\n";
	TestResult += indent + u8"  expected output:\n";
	TestResult += indent + u8"    ERROR: the size of result is 0.\n";
	incindent();
	TestResult += indent + u8"test5 output:\n";

	incindent();
	if (!ParseLangFile(mapresult, str3, " en \t", "\\n", false)) {
		TestResult += indent + u8"ERROR: the size of result is 0.\n";
	}
	else {
		TestResult += indent + u8"<en>\n";
		for (auto pair : mapresult) {
			TestResult += indent + pair.first + '=' + pair.second + '\n';

		}
		TestResult += indent + u8"</en>\n";

	}
	decindent();
	decindent();
	decindent();






	TestResult += indent + u8"ParseLangFile_test finished.\n";

}





#endif


#pragma endregion


