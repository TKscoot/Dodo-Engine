#pragma once
//#include "Entity.h"
#include "components/ECS.h"

namespace Dodo
{
	namespace Entity
	{
		class CEntityHandler
		{
		public:
			CEntityHandler();
			~CEntityHandler();

			static void Update();
			static void Refresh();

			static void AddEntity(Dodo::Entity::CEntity *_ent);

		private:
			static std::vector<std::unique_ptr<Dodo::Entity::CEntity>> m_vEntities;
		};
	}
}
