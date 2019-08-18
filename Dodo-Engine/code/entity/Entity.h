#pragma once
#include "dodopch.h"
#include "components/ECS.h"
#include "components/Transform.h"


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

			//template<typename T, typename... TArgs>
			//std::shared_ptr<T> AddComponent(TArgs&&... _args)
			//{
			//	static_assert(std::is_base_of_v<Components::CComponent, T>,
			//		"Invalid type T. T is not a component");
			//	T* c(new T(std::forward<TArgs>(_args)...));
			//	c->entity = this;
			//	//std::shared_ptr<Components::CComponent> sPtr{ c };
			//
			//	c->Initialize();
			//	std::shared_ptr<T> tPtr{ c };
			//	m_vecComponents.emplace_back(std::move(tPtr));
			//
			//	m_arrComponentArray[Components::getComponentTypeID<T>()] = c;
			//	m_bitComponentBitSet[Components::getComponentTypeID<T>()] = true;
			//
			//	return tPtr;
			//}

			template<typename T, typename... TArgs>
			std::shared_ptr<T> AddComponent(TArgs&&... _args)
			{
				static_assert(std::is_base_of_v<Components::CComponent, T>,
					"Invalid type T. T is not a component");
				T* c(new T(std::forward<TArgs>(_args)...));
				//std::shared_ptr<T> c(std::make_shared<T>(std::forward<TArgs>(_args)...));
				std::shared_ptr<T> sPtr{ c };
				//sPtr->entity = this;
				//std::shared_ptr<Components::CComponent> sPtr{ c };

				sPtr->Initialize();
				//std::shared_ptr<T> tPtr{ c };
				//m_vecComponents.emplace_back(std::move(tPtr));
				//m_vecComponents.push_back(c);
				m_components[&typeid(*sPtr)] = sPtr;
				m_components[&typeid(*sPtr)]->entity = this;

				//m_arrComponentArray[Components::getComponentTypeID<T>()] = c;
				//m_bitComponentBitSet[Components::getComponentTypeID<T>()] = true;


				return sPtr;
			}

			template<typename T>
			std::shared_ptr<T> GetComponent()
			{
				//auto ptr(m_arrComponentArray[Components::getComponentTypeID<T>()]);
				//return static_cast<T*>(ptr);;
				
				if (m_components.count(&typeid(T)) != 0)
				{
					auto c = std::dynamic_pointer_cast<T>(m_components[&typeid(T)]);

					return c;

					//return static_cast<T*>();
				}
				else
				{
					return nullptr;
				}
			}

			bool isActive() const { return m_bActive; }
			void setActive(bool _val) { m_bActive = _val; }

			// Dont just use this if you're not knowing what you are doing
			void SetID(size_t _id) { ID = _id; }

		private:
			size_t ID;
			bool m_bActive = true;
			std::string m_sName;

			std::unordered_map<const std::type_info*, std::shared_ptr<Components::CComponent>> m_components;
			std::vector<Components::CComponent*> m_vecComponents = {};
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

