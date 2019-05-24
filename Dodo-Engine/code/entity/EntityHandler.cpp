#include "EntityHandler.h"

std::vector<std::unique_ptr<Dodo::Entity::CEntity>> Dodo::Entity::CEntityHandler::m_vEntities = {};

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
		ent->UpdateComponents();
		ent->Update();
	}
}

void Dodo::Entity::CEntityHandler::Refresh()
{
	m_vEntities.erase(std::remove_if(std::begin(m_vEntities), std::end(m_vEntities), 
		[](const std::unique_ptr<Dodo::Entity::CEntity> &_entity)
		{
		return !_entity->isActive();
		}
	), std::end(m_vEntities));
}

void Dodo::Entity::CEntityHandler::AddEntity(Dodo::Entity::CEntity *_ent)
{
	std::unique_ptr<Dodo::Entity::CEntity> uPtr{ _ent };
	m_vEntities.emplace_back(std::move(uPtr));
	_ent->SetID(reinterpret_cast<size_t>(_ent));
}
