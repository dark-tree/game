#include "physics.hpp"
#include <iostream>

void Physics::tick() {
	float deltaTime = 1.0f / 60.0f * 2.0f;

	for (auto& crb : colliders_and_rigidbodies) {
		if (crb.second == nullptr) continue;

		crb.first->collision = false;
		crb.second->velocity += gravity * deltaTime;
		
		for (auto& other : colliders_and_rigidbodies) {
			if (crb.first != other.first) {
				float bounciness = crb.first->get_bounciness() * other.first->get_bounciness();

				float x = crb.second->transform->position.x;
				crb.second->transform->position.x += crb.second->velocity.x * deltaTime * Physics::world_scale;
				if (crb.first->collides(other.first)) {
					crb.second->velocity.x = -crb.second->velocity.x * bounciness;
					crb.second->transform->position.x = x;

					// friction
					crb.second->velocity.y *= 0.1f;
				}

				float y = crb.second->transform->position.y;
				crb.second->transform->position.y += crb.second->velocity.y * deltaTime * Physics::world_scale;
				if (crb.first->collides(other.first)) {
					crb.second->velocity.y = -crb.second->velocity.y * bounciness;
					crb.second->transform->position.y = y;

					// friction
					crb.second->velocity.x *= 0.1f;
				}
			}
		}
	}
}

void Physics::add_rigidbody(RigidBody* rb) {
	colliders_and_rigidbodies.push_back({ rb->collider, rb });
}

void Physics::add_collider(Collider* collider) {
	colliders_and_rigidbodies.push_back({ collider, nullptr });
}

float Physics::get_world_scale() {
	return world_scale;
}

std::vector<std::pair<Collider*, RigidBody*>> Physics::colliders_and_rigidbodies;
float Physics::world_scale = 64.0f;
glm::vec2 Physics::gravity = glm::vec2(0, -9.81f);