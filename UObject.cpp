#include "UObject.h"
#include "Mesh.h"
#include "GraphicsEngine.h"
#include "IGraphicsEngine.h"
#include "pipeline.h"
#include "UIRenderer.h"
#include "AObject.h"
#include "Camera.h"

UObject::UObject()
	: texturePath{}
	, uiRenderer(nullptr)
	, attachedObject(nullptr)
	, offsetDx(0.0f)
	, offsetDy(0.0f)
{

}

UObject::~UObject()
{
	
}

void UObject::Render()
{
	//UIRenderer�� ���� �޽� ���� �����α� 
	if (this->attachedObject)
	{
		Vector3 pos = { 
			this->attachedObject->position[0],
			this->attachedObject->position[1],
			this->attachedObject->position[2] 
		};

		Vector3 sca = {
			this->attachedObject->scale[0],
			this->attachedObject->scale[1],
			this->attachedObject->scale[2]
		};

		Vector3 rot = {
			this->attachedObject->rotation[1],
			this->attachedObject->rotation[0],
			this->attachedObject->rotation[2]
		};
		Quaternion rotation = Quaternion::CreateFromYawPitchRoll(rot);
		Matrix WTM = Matrix::Identity;
		WTM *= Matrix::CreateScale(sca);
		WTM *= Matrix::CreateFromQuaternion(rotation);
		WTM *= Matrix::CreateTranslation(pos);

		Matrix VTM = this->gp->GetCamera()->GetViewTM();
		Matrix PTM = this->gp->GetCamera()->GetProjectionTM();

		Matrix FTM = WTM * VTM * PTM;
	
		Vector3 tempPos = { 0.0f, 0.0f, 0.0f};
		Vector4 tempResult;
		Vector3::Transform(tempPos, FTM, tempResult);

		this->dx = ((tempResult.x / tempResult.w) / 2.0f + 0.5f) * 1920;
		this->dy = ((tempResult.y / tempResult.w) / 2.0f + 0.5f) * 1080;
		this->uiRenderer->SetPosition(this->dx + this->offsetDx, this->dy + this->offsetDy, name);

	}
	this->uiRenderer->GetMesh(name);
}

void UObject::AddPosition(float _x, float _y)
{
	this->dx += _x;
	this->dy += _y;
	this->uiRenderer->SetPosition(this->dx, this->dy, name);
}

void UObject::SetPosition(float _x, float _y)
{
	this->dx = _x;
	this->dy = _y;
	this->uiRenderer->SetPosition(this->dx, this->dy, name);
}

/// <summary>
/// UI�� �����ϱ� ���� �ʱⰪ.
/// </summary>
/// <param name="_gp">graphicsEngine</param>
/// <param name="_sPath">VS, PS Path</param>
/// <param name="_texturePath">Texture Path. .png����</param>
/// <param name="_startPointX">���ۺκ� �»��X��ǥ</param>
/// <param name="_startpointY">���ۺκ� �»��Y��ǥ</param>
/// <param name="_width">����</param>
/// <param name="_height">����</param>
void UObject::Initalize(IGraphicsEngine* _gp, std::string _name, std::wstring* _path, std::wstring _texturePath, float _startPointX, float _startpointY, float _width, float _height)
{
	this->gp = dynamic_cast<GraphicsEngine*>(_gp);
	this->name = _name;
	this->path = _path;
	this->texturePath = _texturePath;
	this->startPointX = _startPointX;
	this->startPointY = _startpointY;

	this->dx = startPointX;
	this->dy = startPointY;

	this->width = _width;
	this->height = _height;

	this->windowWidth = gp->GetWindowWidth();
	this->windowHeight = gp->GetWindowHeight();

	this->uiRenderer = this->gp->uiRenderer;
	
	///���� �Ž��� ���� �׳� png UI�� ���� ���, Vertex�� Index�� ���� �������� �Ѵ�.
	///�׸��� ��� UI�� �׷��� ����Ŷ�� �����Ѵ�. ���ܴ� ���� �ʴ´�.
	CreateMesh();
	this->uiRenderer->CreatePipeline(name, path);

}

void UObject::AnimationInitailze(IGraphicsEngine* _gp, std::string _name, std::wstring _texturePath, float _startPointX, float _startPointY, float _width, float _height, float _aniWidth, int _aniCount)
{
	this->gp = dynamic_cast<GraphicsEngine*>(_gp);
	this->name = _name;
	this->texturePath = _texturePath;
	this->startPointX = _startPointX;
	this->startPointY = _startPointY;
	this->width = _width;
	this->height = _height;
	this->aniWidth = _aniWidth;
	this->aniCount = _aniCount;
	
	this->windowWidth = gp->GetWindowWidth();
	this->windowHeight = gp->GetWindowHeight();

	this->uiRenderer = this->gp->uiRenderer;

	CreateAnimationMesh();
	this->uiRenderer->CreateAnimationPipeline(name);
}

void UObject::AttachTo(AObject* _obj, float _offsetX, float _offsetY)
{
	this->attachedObject = _obj;
	this->offsetDx = _offsetX;
	this->offsetDy = _offsetY;
}

void UObject::DetachTo()
{
	this->attachedObject = nullptr;
}

void UObject::CreateMesh()
{
	this->uiRenderer->CreateMesh(name, width, height, startPointX, startPointY, texturePath);
}

void UObject::CreateAnimationMesh()
{
	this->uiRenderer->CreateAnimationMesh(name, width, height, startPointX, startPointY, aniWidth, texturePath, aniCount);
}

