#include "enumsubfiles.h"


//return value: 
//    False means that the searching has been aborted by the callback function.
//    It will return true otherwise.
bool enumsubfiles(

	const std::wstring &dir_with_back_slant,        //for example: L"C:\\", L"E:\\test\\"
	const std::wstring &filename,                   //for example: L"*", L"123.txt", L"*.exe", L"123.???"
	unsigned int maxdepth,                 //0 means not searching subdirectories, 1 means maximum depth of subdirectories is 1,
										   //    pass -1 to search all the subdirectories.
	enumflags flags,                       //search files, directories, or both.
	std::function<bool(const std::wstring &dir_with_back_slant, _wfinddata_t &attrib)> callback
) noexcept
{
	_wfinddata_t dat;
	size_t hfile;
	std::wstring fullname = dir_with_back_slant + filename;
	std::wstring tmp;
	bool ret = true;


	hfile = _wfindfirst(fullname.c_str(), &dat);
	if (hfile == -1) goto a;
	do {
		if (!(wcscmp(L".", dat.name) && wcscmp(L"..", dat.name))) continue;
		if (((dat.attrib&_A_SUBDIR) && (!(flags&ENUM_DIR))) || ((!(dat.attrib&_A_SUBDIR)) && (!(flags&ENUM_FILE)))) continue;
		ret = callback(dir_with_back_slant, dat);
		if (!ret) {
			_findclose(hfile);
			return ret;
		}
	} while (_wfindnext(hfile, &dat) == 0);
	_findclose(hfile);

a:

	if (!maxdepth) return ret;

	tmp = dir_with_back_slant + L"*";
	hfile = _wfindfirst(tmp.c_str(), &dat);
	if (hfile == -1) return ret;
	do {
		if (!(wcscmp(L".", dat.name) && wcscmp(L"..", dat.name))) continue;
		if (!(dat.attrib&_A_SUBDIR)) continue;
		tmp = dir_with_back_slant + dat.name + L"\\";
		ret = enumsubfiles(tmp, filename, maxdepth - 1, flags, callback);
		if (!ret) {
			_findclose(hfile);
			return ret;
		}


	} while (_wfindnext(hfile, &dat) == 0);
	_findclose(hfile);

	return ret;

}