#pragma once
#include "dodopch.h"
#include "common/VKIntegration.h"
#include "common/DodoTypes.h"
#include "ECS.h"

namespace Dodo
{
	namespace Components
	{
		using namespace Dodo::Math;

		class CTransform : public CComponent
		{
		public:
			inline CTransform()
				: m_scale({ 1.0f, 1.0f, 1.0f })
				, m_rotation(Matrix4x4())
				, m_translation({ 0.0f, 0.0f, 0.0f })
				, m_direction({ 0.0f, 0.0f, 1.0f })
				, m_right({ 1.0f, 0.0f, 0.0f })
				, m_up({ 0.0f, 1.0f, 0.0f })
				, m_local(Matrix4x4())
				, m_composed(Matrix4x4())
			{}

			inline const Vector3f& getDirection() const { return m_direction; };
			inline const Vector3f& getUp()        const { return m_up; };
			inline const Vector3f& getRight()     const { return m_right; };

			inline void setScaleX(const float& factor) { m_scale.x = factor; invalidate(); }
			inline void setScaleY(const float& factor) { m_scale.y = factor; invalidate(); }
			inline void setScaleZ(const float& factor) { m_scale.z = factor; invalidate(); }
			inline void setScale(
				const float& x,
				const float& y,
				const float& z)
			{
				setScaleX(x);
				setScaleY(y);
				setScaleZ(z);
				invalidate();
			}
			inline void setScale(const Vector3f& vec) { m_scale = vec; invalidate(); }

			inline const Vector3f& getScale() const { return m_scale; }

			inline void invalidate()
			{
				m_local = glm::translate(m_local, m_translation);
				m_local = glm::scale(m_local, m_scale);
				m_local = glm::matrixCompMult(m_local, m_rotation);			// BUGGY!!!
			}
		

		private:
			Vector3f  m_translation;
			Matrix4x4 m_rotation;
			Vector3f  m_scale;

			Vector3f  m_direction;
			Vector3f  m_right;
			Vector3f  m_up;
					  
			Matrix4x4 m_local;
			Matrix4x4 m_composed;
		};
	}
}

