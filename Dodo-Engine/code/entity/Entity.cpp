#include "dodopch.h"
#include "Entity.h"
#include "EntityHandler.h"

Dodo::Entity::CEntity::CEntity()
{
	CEntityHandler::AddEntity(this);
}