#include "dodopch.h"
#include "Entity.h"
#include "EntityHandler.h"

Dodo::Entity::CEntity::CEntity()
{
	CEntityHandler::AddEntity(this);
}

Dodo::Entity::CEntity::CEntity(std::string _name)
{
	m_sName = _name;
	CEntityHandler::AddEntity(this);
}