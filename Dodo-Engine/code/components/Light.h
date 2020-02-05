#pragma once
#include "ECS.h"
#include "common/DodoTypes.h"

namespace Dodo
{
	namespace Components
	{
		using namespace Dodo::Math;

		class CLight : public CComponent
		{
		public:
			enum LightType{LIGHT_DIRECTIONAL, LIGHT_POINT};

			struct LightProperties
			{
				Vector4f  position;
				Vector4f  direction;
				Vector4f  color;
				LightType	  type;
				float	  ambientIntensity;
				float	  diffuseIntensity;
				float	  specularIntensity;
			};

			CLight() = default;
			CLight(LightProperties properties)
			: m_properties(properties)
			{
			}
			~CLight();

			LightProperties& GetProperties() { return m_properties; }
			void SetPosition(Vector4f pos) { m_properties.position = pos; }

		private:
			LightProperties m_properties = {};
		};
	}
}
