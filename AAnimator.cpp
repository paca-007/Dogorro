#include "AAnimator.h"
#include "AObject.h"
#include "AAnimation.h"
#include "ABone.h"

void AAnimator::CreateAniMatrix(AObject* _obj)
{
	// a 번째 애니메이션
	this->aniMatrix.resize(_obj->animations.size());
	for (uint32 a = 0; a < _obj->animations.size(); a++)
	{
		auto& ani = _obj->animations[a];
		// f 번째 프레임
		this->aniMatrix[a].resize(ani->frameCount);
		for (uint32 f = 0; f < ani->frameCount; f++)
		{
			std::vector<Matrix> tempMatirx(_obj->bones.size());

			// b 번째 본 데이터
			this->aniMatrix[a][f].resize(_obj->bones.size());
			for (uint32 b = 0; b < _obj->bones.size(); b++)
			{
				std::shared_ptr<ABone> bone = _obj->bones[b];
				std::shared_ptr<AKeyFrame> frame = ani->keyframes[bone->name];


				Matrix resultMatrix = Matrix::Identity;

				if (frame)
				{
					if (!frame->transforms.empty())
					{
						AFrameData& data = frame->transforms[f];

						resultMatrix *= Matrix::CreateScale(data.scale.x, data.scale.y, data.scale.z);
						resultMatrix *= Matrix::CreateFromQuaternion(data.rotation);
						resultMatrix *= Matrix::CreateTranslation(data.translation.x, data.translation.y, data.translation.z);
					}
				}

				Matrix toRoot = bone->transform;

				int32 parentIndex = bone->parentIndex;

				Matrix matParent = Matrix::Identity;
				if (parentIndex >= 0)
				{
					matParent = tempMatirx[parentIndex];
				}
				tempMatirx[b] = resultMatrix * matParent;

				this->aniMatrix[a][f][b] = tempMatirx[b];
			}
		}
	}
}
