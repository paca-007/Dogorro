#pragma once
#include "pch.h"
using std::string;
using std::wstring;


/// <summary>
/// ��ƿ��Ƽ ����� �����ϴ� Ŭ����
/// </summary>
class Utils
{
public:
	// string -> wstring
	// wstring -> string
	static wstring ToWString(string value);
	static string ToString(wstring value);

	// ���ڿ� ��ü 
	static void Replace(string& str, string comp, string rep);
	static void Replace(wstring& str, wstring comp, wstring rep);

	// Ư�� ���ڿ��� �����ϴ��� ���� �˻�
	static bool StartsWith(string str, string comp);
	static bool StartsWith(wstring str, wstring comp);
};

