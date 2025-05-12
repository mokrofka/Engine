#pragma once
#include "Entity.h"

#include <cassert>
#include <unordered_map>

class IComponentArray {
public:
	virtual      ~IComponentArray() = default;
	virtual void EntityDestroyed(Entity entity) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
	// The packed array of components (of generic type T),
	// set to a specified maximum amount, matching the maximum number
	// of entities allowed to exist simultaneously, so that each entity
	// has a unique spot.
	std::array<T, MAX_ENTITIES> m_componentArray;

	// Map from an entity ID to an array index.
	std::unordered_map<Entity, u64> m_entityToIndexMap;

	// Map from an array index to an entity ID.
	std::unordered_map<u64, Entity> m_indexToEntityMap;

	// Total size of valid entries in the array.
	u64 m_size{};

public:
	void InsertData(Entity entity, T component) {
		assert(m_entityToIndexMap.find(entity) == m_entityToIndexMap.end() && "Component added to same entity more than once.");

		// Put new entry at end and update the maps
		u64 newIndex                 = m_size;
		m_entityToIndexMap[entity]   = newIndex;
		m_indexToEntityMap[newIndex] = entity;
		m_componentArray[newIndex]   = component;
		++m_size;
	}

	void RemoveData(Entity entity) {
		assert(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() && "Removing non-existent component.");

		// Copy element at end into deleted element's place to maintain density
		u64 indexOfRemovedEntity               = m_entityToIndexMap[entity];
		u64 indexOfLastElement                 = m_size - 1;
		m_componentArray[indexOfRemovedEntity] = m_componentArray[indexOfLastElement];

		// Update map to point to moved spot
		Entity entityOfLastElement               = m_indexToEntityMap[indexOfLastElement];
		m_entityToIndexMap[entityOfLastElement]  = indexOfRemovedEntity;
		m_indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

		m_entityToIndexMap.erase(entity);
		m_indexToEntityMap.erase(indexOfLastElement);

		--m_size;
	}

	T& GetData(Entity entity) {
		assert(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() && "Retrieving non-existent component.");

		// Return a reference to the entity's component
		return m_componentArray[m_entityToIndexMap[entity]];
	}

	void EntityDestroyed(Entity entity) override {
		if (m_entityToIndexMap.find(entity) != m_entityToIndexMap.end()) {
			// Remove the entity's component if it existed
			RemoveData(entity);
		}
	}
};
