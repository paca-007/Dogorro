#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <wrl\client.h>

using Microsoft::WRL::ComPtr;

// 일단 옮기긴 했는데 어떻게 이 엔진에 사용하지..?

// 템플릿으로 만들기.
template <typename T>
class ComResourceManager
{
public:
	ComResourceManager() 
	{
		resourceMap = std::unordered_map<std::string, ComPtr<T>>();
	};
	~ComResourceManager() {};

	void StoreResource(std::string _name, ComPtr<T> _value);
	void ForceStoreResource(std::string _name, ComPtr<T> _value);


	// 링크로 찾는다. return 값이 shared_ptr이다.
	ComPtr<T> GetResource(const std::string& _path);

	// Resource Map 전부 삭제
	void UnLoadResource();


private:

	// 템플릿으로 만들기
	// 앞에 string은 파일 링크, 뒤에는 저장되어 있는 Resource
	std::unordered_map<std::string, ComPtr<T>> resourceMap;

};

template <typename T>
void ComResourceManager<T>::ForceStoreResource(std::string _name, ComPtr<T> _value)
{
	resourceMap[_name] = _value;
}

template <typename T>
void ComResourceManager<T>::StoreResource(std::string _name, ComPtr<T> _value)
{
	if (resourceMap.find(_name) == resourceMap.end())
	{
		resourceMap[_name] = _value;
	}
}

template<typename T>
void ComResourceManager<T>::UnLoadResource()
{
	resourceMap.clear();
}

template<typename T>
ComPtr<T> ComResourceManager<T>::GetResource(const std::string& _path)
{
	auto iter = resourceMap.find(_path);

	if (iter != resourceMap.end())
	{
		return iter->second;
	}
	else return nullptr;
}

