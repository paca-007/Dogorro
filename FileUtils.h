#pragma once
#include "pch.h"

enum FileMode : uint8
{
	Write,
	Read
};

class FileUtils
{
public:
	FileUtils();
	~FileUtils();

public:
	void Open(std::wstring filePath, FileMode mode);
	void Close();

public:
	// Write
	template<typename T>
	void Write(const T& data)
	{
		// ���� üũ ����
		uint32 numOfBytes = 0;

		// ������ �����Ͱ� �ϳ��� ���ٸ� numOfBytes �� 0 ����Ʈ �� ���̰�,
		// ���������� numOfBytes �� 0�̶�� false �� ��ȯ�ؼ� ������ ���.
		assert(::WriteFile(m_Handle, &data, sizeof(T), (LPDWORD)&numOfBytes, nullptr) && "���� ���� �������� ���� �߻�");
	}

	template<>
	void Write<std::string>(const std::string& data)
	{
		return Write(data);
	}

	void Write(void* data, uint32 dataSize);
	void Write(const std::string& data);

public:
	// Read
	template<typename T>
	void Read(T& data)
	{
		// ���� üũ ����
		uint32 numOfBytes = 0;

		BOOL readData = ::ReadFile(m_Handle, &data, sizeof(T), (LPDWORD)&numOfBytes, nullptr);

		// ���� �����Ͱ� �ϳ��� ���ٸ� numOfBytes �� 0 ����Ʈ �� ���̰�,
		// ���������� numOfBytes �� 0�̶�� false �� ��ȯ�ؼ� ������ ���.
		assert(readData && "���� �б� �������� ���� �߻�");
	}

	template<typename T>
	T Read()
	{
		T data = {};
		Read(data);
		return data;
	}

	void Read(void** data, uint32 dataSize);
	void Read(std::string& data);

private:
	HANDLE m_Handle = INVALID_HANDLE_VALUE;

};

