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
			CEntity(std::string _name);
			~CEntity() {}

			void Update() {}

			void UpdateComponents()
			{
				for (auto& c : m_components)
				{
					c.second->Update();
				}
			}

			void Destroy() { m_bActive = false; }


			//template<typename T>
			//bool hasComponent() const
			//{
			//	return m_bitComponentBitSet[Components::getComponentTypeID<T>];
			//}

			template<typename T, typename... TArgs>
			std::shared_ptr<T> AddComponent(TArgs&&... _args)
			{
				static_assert(std::is_base_of_v<Components::CComponent, T>,
					"Invalid type T. T is not a component");
				//T* c(new T(std::forward<TArgs>(_args)...));
				//std::shared_ptr<T> sPtr{ c };
				
				std::shared_ptr<T> component = std::make_shared<T>(std::forward<TArgs>(_args)...);
				component->Initialize();

				//sPtr->Initialize();

				m_components[&typeid(*component)] = component;
				m_components[&typeid(*component)]->entity = this;


				return component;
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
			void setName(std::string _name) { m_sName = _name; }

			// Dont just use this if you're not knowing what you are doing
			void SetID(size_t _id) { ID = _id; }

		private:
			size_t ID;
			bool m_bActive      = true;
			std::string m_sName = "";

			std::unordered_map<const std::type_info*, std::shared_ptr<Components::CComponent>> m_components = {};
			//std::vector<Components::CComponent*> m_vecComponents = {};
			//Components::ComponentArray  m_arrComponentArray;
			//Components::ComponentBitSet m_bitComponentBitSet;
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

