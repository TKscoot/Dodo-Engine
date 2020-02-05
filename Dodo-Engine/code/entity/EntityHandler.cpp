#include "dodopch.h"
#include "EntityHandler.h"

//std::vector<std::shared_ptr<Dodo::Entity::CEntity>> Dodo::Entity::CEntityHandler::m_vEntities = {};
std::vector<Dodo::Entity::CEntity*> Dodo::Entity::CEntityHandler::m_vEntities = {};

Dodo::Entity::CEntityHandler::CEntityHandler()
{
}


Dodo::Entity::CEntityHandler::~CEntityHandler()
{
}

void Dodo::Entity::CEntityHandler::Update()
{
	for (auto& ent : m_vEntities)
	{
		if (ent != nullptr)
		{

			for (auto& c : ent->GetAllComponents())
			{
				c.second->Update();
			}

			ent->Update();
		}
	}
}

void Dodo::Entity::CEntityHandler::AddEntity(Dodo::Entity::CEntity *_ent)
{
	//std::shared_ptr<Dodo::Entity::CEntity> uPtr{ _ent };
	m_vEntities.push_back(_ent);
	_ent->SetID(reinterpret_cast<size_t>(_ent));
}
