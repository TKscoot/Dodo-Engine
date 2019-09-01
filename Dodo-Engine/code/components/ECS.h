#pragma once
#include "dodopch.h"

#include "environment/Error.h"

namespace Dodo
{
	namespace Components
	{
		class CComponent;
	}
	namespace Entity
	{
		class CEntity;
	}

	namespace Components
	{
		using ComponentID = std::size_t;

		inline ComponentID getComponentTypeID()
		{
			static ComponentID lastID = 0;
			return lastID++;
		}

		template<typename T>
		inline ComponentID getComponentTypeID() noexcept
		{
			static ComponentID typeID = getComponentTypeID();
			return typeID;
		}

		constexpr size_t maxComponents = 32;

		using ComponentBitSet = std::bitset<maxComponents>;
		using ComponentArray = std::array<CComponent*, maxComponents>;

	}

	namespace Components
	{
		class CComponent
		{
		public:
			Entity::CEntity* entity = nullptr;

			virtual Dodo::Environment::DodoError Initialize() { return Dodo::Environment::DodoError::DODO_OK; }
			virtual void Update() {}
			virtual void Finalize() {}


			//virtual ~CComponent() {}
		};
	}

}

