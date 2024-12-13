#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <wrl\client.h>

using Microsoft::WRL::ComPtr;

// �ϴ� �ű�� �ߴµ� ��� �� ������ �������..?

// ���ø����� �����.
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


	// ��ũ�� ã�´�. return ���� shared_ptr�̴�.
	ComPtr<T> GetResource(const std::string& _path);

	// Resource Map ���� ����
	void UnLoadResource();


private:

	// ���ø����� �����
	// �տ� string�� ���� ��ũ, �ڿ��� ����Ǿ� �ִ� Resource
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

