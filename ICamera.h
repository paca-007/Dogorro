#pragma once
#include <vector>

class ICamera abstract
{
public:
	float screenHight;
	float screenWidth;

public:

	ICamera(float _screenHight, float _screenWidth) 
		: screenWidth(_screenWidth)
		, screenHight(_screenHight)
	{};
	virtual ~ICamera() {};

	virtual void AddFOV(float _value) abstract;
	virtual void Rotate(float _xValue, float _yValue, float _zValue) abstract;
	virtual void Traslation(float _xValue, float _yValue, float _zValue) abstract;

	virtual void MoveFoward(float _value) abstract;
	virtual void MoveRight(float _value) abstract;
	virtual void MoveUP(float _value) abstract;

	virtual void RotateRight(float _value) abstract;
	virtual void RotateUp(float _value) abstract;

	virtual void SetPosition(float _xValue, float _yValue, float _zValue) abstract;
	virtual void SetRotation(float _xValue, float _yValue, float _zValue) abstract;
	virtual void SetFov(float _value) abstract;

	virtual std::vector<float> GetPostion() abstract;
	virtual std::vector<float> GetClickVector(int _x, int _y) abstract;

	virtual std::vector<std::vector<float>> GetViewMatrix() abstract;
};