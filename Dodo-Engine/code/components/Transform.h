#pragma once
#include "dodopch.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
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
				, m_rotation(Vector3f())
				, m_translation({ 0.0f, 0.0f, 1.0f })
				, m_direction({ 0.0f, 0.0f, 1.0f })
				, m_right({ 1.0f, 0.0f, 0.0f })
				, m_up({ 0.0f, 1.0f, 0.0f })
				, m_composed(Matrix4x4(1.0f))
			{}
		
			inline const Vector3f&  getDirection() const { return m_direction; };
			inline const Vector3f&  getUp()        const { return m_up; };
			inline const Vector3f&  getRight()     const { return m_right; };
			inline const Vector3f&  getRotation()  const { return m_rotation; }
			inline const Vector3f&  getPosition()  const { return m_translation; }
		
			inline void setPositionX(const float& x) { m_translation.x = x; invalidate(); }
			inline void setPositionY(const float& y) { m_translation.y = y; invalidate(); }
			inline void setPositionZ(const float& z) { m_translation.z = z; invalidate(); }
			inline void setPosition(
				const float& x,
				const float& y,
				const float& z)
			{
				setPositionX(x);
				setPositionY(y);
				setPositionZ(z);
				invalidate();
			}
			inline void setPosition(const Vector3f& vec) { m_translation = vec; invalidate(); }
		
			inline void setRotationX(const float& x) { m_rotation.x = x; invalidate(); }
			inline void setRotationY(const float& y) { m_rotation.y = y; invalidate(); }
			inline void setRotationZ(const float& z) { m_rotation.z = z; invalidate(); }
			inline void setRotation(
				const float& x,
				const float& y,
				const float& z)
			{
				setRotationX(x);
				setRotationY(y);
				setRotationZ(z);
				invalidate();
			}
			inline void setRotation(const Vector3f& vec) { m_rotation = Vector4f(vec, 1.0f); invalidate(); }
		
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
			inline void setScale(const Vector3f& vec) { m_scale = Vector4f(vec, 1.0f); invalidate(); }
		
			inline const Vector3f& getScale() const { return m_scale; }
		
			inline const Matrix4x4& getComposed() { invalidate(); return m_composed; }
		
			inline void invalidate()
			{
				//m_rotationMat = glm::eulerAngleXYZ(m_rotation.x, m_rotation.y, m_rotation.z);

				m_composed = glm::rotate(m_composed, m_rotation.x, m_right);
				m_composed = glm::rotate(m_composed, m_rotation.y, m_up);
				m_composed = glm::rotate(m_composed, m_rotation.z, m_direction);
				m_composed = glm::translate(m_composed, m_translation);
				m_composed = glm::scale(m_composed, m_scale);
				//m_composed = m_rotationMat * m_composed;			// BUGGY!!!
				//m_composed = glm::rotate(m_composed, )
		
				m_transformed = m_composed * m_transformed;
			}

			inline void Update() override {}
		
		
		private:
			Vector3f  m_translation = Vector3f(0.0f, 1.0f, 0.0f);
			Vector3f  m_rotation = Vector3f(0.0f, 1.0f, 0.0f);
			Vector3f  m_scale = Vector3f(1.0f, 1.0f, 1.0f);
			Matrix4x4 m_rotationMat;
			
			Vector3f  m_direction;
			Vector3f  m_right;
			Vector3f  m_up;
					  
			Matrix4x4 m_composed = Matrix4x4(1.0f);
		
			Vector4f m_transformed = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
		};










		//class CTransform :
		//	public CComponent
		//{
		//public:
		//	/**
		//	 * Creates a new transform.
		//	 * @param position The position.
		//	 * @param rotation The rotation.
		//	 * @param scale The scale.
		//	 */
		//	CTransform() { }
		//	CTransform(const Vector3f &position, const Vector3f &rotation, const Vector3f &scale) {}
		//
		//	~CTransform() {}
		//
		//	/**
		//	 * Multiplies this transform with another transform.
		//	 * @param other The other transform.
		//	 * @return The resultant transform.
		//	 */
		//	CTransform Multiply(const CTransform &other) const;
		//
		//	Matrix4x4 GetWorldMatrix() const;
		//
		//	Vector3f GetPosition() const;
		//
		//	Vector3f GetRotation() const;
		//
		//	Vector3f GetScale() const;
		//
		//	const Vector3f &GetLocalPosition() const { return m_position; }
		//
		//	void SetLocalPosition(const Vector3f &localPosition);
		//
		//	const Vector3f &GetLocalRotation() const { return m_rotation; }
		//
		//	void SetLocalRotation(const Vector3f &localRotation);
		//
		//	const Vector3f &GetLocalScale() const { return m_scale; }
		//
		//	void SetLocalScale(const Vector3f &localScale);
		//
		//	CTransform *GetParent() const { return m_parent; }
		//
		//	void SetParent(CTransform *parent);
		//
		//	//void SetParent(Entity *parent);
		//
		//	const std::vector<CTransform *> &GetChildren() const { return m_children; }
		//
		//	std::string ToString() const;
		//
		//	bool operator==(const CTransform &other) const;
		//
		//	bool operator!=(const CTransform &other) const;
		//
		//	friend CTransform operator*(const CTransform &left, const CTransform &right);
		//	
		//	CTransform &operator*=(const CTransform &other);
		//
		//	//friend const Metadata &operator>>(const Metadata &metadata, CTransform &transform);
		//	//
		//	//friend Metadata &operator<<(Metadata &metadata, const CTransform &transform);
		//	//
		//	//friend std::ostream &operator<<(std::ostream &stream, const CTransform &transform);
		//
		//private:
		//	const CTransform *GetWorldTransform() const;
		//
		//	void AddChild(CTransform *child);
		//
		//	void RemoveChild(CTransform *child);
		//
		//	Vector3f m_position;
		//	Vector3f m_rotation;
		//	Vector3f m_scale;
		//
		//	CTransform *m_parent{};
		//	std::vector<CTransform *> m_children;
		//	mutable CTransform *m_worldTransform{};
		//
		//	Vector3f Left{ -1.0f, 0.0f, 0.0f };
		//	Vector3f Right{ 1.0f, 0.0f, 0.0f };
		//	Vector3f Up{ 0.0f, 1.0f, 0.0f };
		//	Vector3f Down{ 0.0f, -1.0f, 0.0f };
		//	Vector3f Front{ 0.0f, 0.0f, 1.0f };
		//	Vector3f Back{ 0.0f, 0.0f, -1.0f };
		//};
	}
}

