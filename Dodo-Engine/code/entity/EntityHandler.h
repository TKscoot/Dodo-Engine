#pragma once
#include "dodopch.h"
#include "entity/Entity.h"

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

			static void AddEntity(Dodo::Entity::CEntity *_ent);
			//static std::vector<std::shared_ptr<Dodo::Entity::CEntity>> GetEntities() { return m_vEntities; }
			static std::vector<Dodo::Entity::CEntity*> GetEntities() { return m_vEntities; }

		private:
			//static std::vector<std::shared_ptr<Dodo::Entity::CEntity>> m_vEntities;
			static std::vector<Dodo::Entity::CEntity*> m_vEntities;
		};
	}
}
