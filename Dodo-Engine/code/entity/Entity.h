#pragma once
#include "dodopch.h"
#include "components/ECS.h"


namespace Dodo
{
	namespace Entity
	{
		class CEntity
		{
		public:

			CEntity();
			~CEntity() {}

			void Update() {}

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
			std::shared_ptr<T> AddComponent(TArgs&&... _args)
			{
				static_assert(std::is_base_of_v<Components::CComponent, T>,
					"Invalid type T. T is not a component");
				T* c(new T(std::forward<TArgs>(_args)...));
				c->entity = this;
				std::shared_ptr<Components::CComponent> sPtr{ c };
				m_vecComponents.emplace_back(std::move(sPtr));

				m_arrComponentArray[Components::getComponentTypeID<T>()] = c;
				m_bitComponentBitSet[Components::getComponentTypeID<T>()] = true;

				c->Initialize();
				std::shared_ptr<T> tPtr{ c };

				return tPtr;
			}

			template<typename T>
			T* GetComponent() const
			{
				auto ptr(m_arrComponentArray[Components::getComponentTypeID<T>()]);
				
				return static_cast<T*>(ptr);
			}

			bool isActive() const { return m_bActive; }
			void setActive(bool _val) { m_bActive = _val; }

			// Dont just use this if you're not knowing what you are doing
			void SetID(size_t _id) { ID = _id; }

		private:
			size_t ID;
			bool m_bActive = true;
			std::string m_sName;

			std::vector<std::shared_ptr<Components::CComponent>> m_vecComponents;
			Components::ComponentArray  m_arrComponentArray;
			Components::ComponentBitSet m_bitComponentBitSet;
		};


		class TestEnt : public CEntity
		{
		public:
			void Update()
			{
				Environment::CLog::Message("This is the Test entity!");
			}
		};
	}
}

