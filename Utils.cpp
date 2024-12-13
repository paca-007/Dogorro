#include "Utils.h"


/// <summary>
/// wstring �� string ���� ��ȯ
/// </summary>
std::string Utils::ToString(wstring value)
{
	return string(value.begin(), value.end());
}

/// <summary>
/// string �� wstring ���� ��ȯ
/// </summary>
std::wstring Utils::ToWString(string value)
{
	return wstring(value.begin(), value.end());
}

/// <summary>
/// ���ڿ� str ���� comp �� ã�Ƽ� rep �� ��ü�ϴ� �Լ�
/// ���ڿ��� �����ϰ� �����Ѵ�.
/// </summary>
/// <param name="str">��� ���ڿ�</param>
/// <param name="comp">��ü�� �κ� ���ڿ�</param>
/// <param name="rep">comp�� ��ü�� �� �κ� ���ڿ�</param>
void Utils::Replace(string& str, string comp, string rep)
{
	string temp = str;

	size_t startPos = 0;

	// find �Լ��� ���н� npos �� ��ȯ
	while ((startPos = temp.find(comp, startPos)) != string::npos)
	{
		temp.replace(startPos, comp.length(), rep);
		startPos += rep.length();
	}

	str = temp;
}

void Utils::Replace(wstring& str, wstring comp, wstring rep)
{
	wstring temp = str;

	size_t startPos = 0;
	while ((startPos = temp.find(comp, startPos)) != wstring::npos)
	{
		temp.replace(startPos, comp.length(), rep);
		startPos += rep.length();
	}

	str = temp;
}

/// <summary>
/// �־��� ���ڿ��� Ư�� ���ڿ��� �����ϴ��� ���θ� Ȯ���ϴ� �Լ�
/// </summary>
/// <param name="str">�־��� ���ڿ�</param>
/// <param name="comp">���� ���� ���� ���ڿ�</param>
/// <returns></returns>
bool Utils::StartsWith(string str, string comp)
{
	wstring::size_type index = str.find(comp);
	if (index != wstring::npos && index == 0)
		return true;

	return false;
}

bool Utils::StartsWith(wstring str, wstring comp)
{
	wstring::size_type index = str.find(comp);
	if (index != wstring::npos && index == 0)
		return true;

	return false;
}