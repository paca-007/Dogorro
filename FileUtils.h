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
		// 오류 체크 변수
		uint32 numOfBytes = 0;

		// 쓰여진 데이터가 하나도 없다면 numOfBytes 는 0 바이트 일 것이고,
		// 내부적으로 numOfBytes 가 0이라면 false 를 반환해서 오류가 뜬다.
		assert(::WriteFile(m_Handle, &data, sizeof(T), (LPDWORD)&numOfBytes, nullptr) && "파일 쓰기 과정에서 오류 발생");
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
		// 오류 체크 변수
		uint32 numOfBytes = 0;

		BOOL readData = ::ReadFile(m_Handle, &data, sizeof(T), (LPDWORD)&numOfBytes, nullptr);

		// 읽은 데이터가 하나도 없다면 numOfBytes 는 0 바이트 일 것이고,
		// 내부적으로 numOfBytes 가 0이라면 false 를 반환해서 오류가 뜬다.
		assert(readData && "파일 읽기 과정에서 오류 발생");
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

