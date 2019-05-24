#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>

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
			Entity::CEntity* entity;
		
			virtual Dodo::Environment::DodoError Initialize() { return Dodo::Environment::DodoError::DODO_OK; }
			virtual void Update() {}
			virtual void Finalize() {}
		
		
			virtual ~CComponent() {}
		};
	}
		
	namespace Entity
	{
		class CEntity
		{
		public:

			CEntity() {}
			~CEntity() {}

			virtual void Update() {}
			void UpdateComponents()
			{
				for (auto& c : m_vecComponents)
				{
					c->Update();
				}
			}

			void Destroy() { m_bActive = false; }


			template<typename T>
			bool hasComponent() const
			{
				return m_bitComponentBitSet[Components::getComponentTypeID<T>];
			}

			template<typename T, typename... TArgs>
			T& AddComponent(TArgs&&... _args)
			{
				static_assert(std::is_base_of_v<Components::CComponent, T>,
					"Invalid type T. T is not a component");
				T* c(new T(std::forward<TArgs>(_args)...));
				c->entity = this;
				std::unique_ptr<Components::CComponent> uPtr{ c };
				m_vecComponents.emplace_back(std::move(uPtr));

				m_arrComponentArray[Components::getComponentTypeID<T>()] = c;
				m_bitComponentBitSet[Components::getComponentTypeID<T>()] = true;

				c->Initialize();
				return *c;
			}

			template<typename T>
			T& GetComponent() const
			{
				auto ptr(m_arrComponentArray[Components::getComponentTypeID<T>()]);
				return *static_cast<T*>(ptr);
			}

			bool isActive() const { return m_bActive; }
			void setActive(bool _val) { m_bActive = _val; }

			// Dont just use this if you're not knowing what you are doing
			void SetID(size_t _id) { ID = _id; }

		private:
			size_t ID;
			bool m_bActive = true;
			std::string m_sName;

			std::vector<std::unique_ptr<Components::CComponent>> m_vecComponents;
			Components::ComponentArray  m_arrComponentArray;
			Components::ComponentBitSet m_bitComponentBitSet;
		};
	}
		
}

