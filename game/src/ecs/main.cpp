#include "Coordinator.h"
#include "defines.h"

#include <thread>

struct vec3 {
	f32 x, y, z;
	vec3() {}
	vec3(f32 x, f32 y, f32 z) :
	x(x), y(y), z(z) {}
};

struct Gravity {
	f32 f;
};

struct Transform {
	vec3 position;
};

struct PhysicsSystem : public System {
	void Update();
};

Coordinator Coordinator;

void PhysicsSystem::Update() {
	for (auto const& entity : m_entities) {
		auto&       transform = Coordinator.GetComponent<Transform>(entity);
		auto const& gravity   = Coordinator.GetComponent<Gravity>(entity);

		transform.position.y += gravity.f;
	}
}

void foo(i32 a){
  
}

i32 main() {
	Coordinator.Init();

	Coordinator.RegisterComponent<Gravity>();
	Coordinator.RegisterComponent<Transform>();

	auto physicsSystem = Coordinator.RegisterSystem<PhysicsSystem>();

	Signature signature;
	signature.set(Coordinator.GetComponentType<Gravity>());
	signature.set(Coordinator.GetComponentType<Transform>());
	Coordinator.SetSystemSignature<PhysicsSystem>(signature);

	std::vector<Entity> entities(MAX_ENTITIES);
	
	for (Entity& entity : entities) {
		entity = Coordinator.CreateEntity();

		Coordinator.AddComponent(entity, Gravity{f32(rand() % 10 + 1)});
		Coordinator.AddComponent(entity, Transform{vec3(0.0, 1.0, 0.0)});
		Coordinator.AddComponent(entity, 1);
	}

	static i32 a = 100;
	while (a) {
		--a;
		using namespace std::chrono_literals;
		physicsSystem->Update();

		f32 y1 = Coordinator.GetComponent<Transform>(entities[0]).position.y;
		printf("first entity = %f\n", y1);
		f32 y2 = Coordinator.GetComponent<Transform>(entities[1]).position.y;
		printf("second entity = %f\n", y2);

		std::this_thread::sleep_for(10ms);
	}
}
