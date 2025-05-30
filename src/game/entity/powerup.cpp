
#include "powerup.hpp"

#include "player.hpp"
#include "particle/blow.hpp"
#include "game/level/level.hpp"
#include "../level/tile.hpp"
#include "game/emitter.hpp"
#include "game/sounds.hpp"

/*
 * PowerUpEntity
 */

PowerUpEntity::Type PowerUpEntity::randomPick() {
	int pick = randomInt(0, 100);
	if (pick >=   0 && pick <=  30) return LIVE;
	if (pick >=  30 && pick <=  60) return DOUBLE_BARREL;
	if (pick >=  60 && pick <=  80) return SHIELD;

	return randomPick();
}

PowerUpEntity::PowerUpEntity(double x, double y, Type type)
: Entity(32, x, y), type(type) {

}

bool PowerUpEntity::checkPlacement(Level& level) {
	return true;
}

void PowerUpEntity::applyEffect(Level& level, PlayerEntity* player) {
	if (type == Type::LIVE) {
		player->onDamage(level, -10, this);
	}

	if (type == Type::DOUBLE_BARREL) {
		player->double_barrel_ticks += 50;
	}

	if (type == Type::SHIELD) {
		player->enableShield(level);
	}
}

void PowerUpEntity::onDamage(Level& level, int damage, Entity* damager) {
	if (damager->isCausedByPlayer()) {
		std::shared_ptr<Entity> parent = damager->getParent();
		SoundSystem::getInstance().add(Sounds::coin).play();

		if (!dead && parent) {
			applyEffect(level, dynamic_cast<PlayerEntity*>(parent.get()));
			dead = true;
		}

		Entity::onDamage(level, damage, damager);
	}
}

void PowerUpEntity::tick(Level& level) {
	this->angle = sin(this->age * 0.07f) * 0.5;

	Collision collision = level.checkCollision(this);

	if (!dead && (collision.type == Collision::ENTITY)) {
		if(PlayerEntity* player = dynamic_cast<PlayerEntity*>(collision.entity)) {
			level.addEntity(new BlowEntity(x, y));
			SoundSystem::getInstance().add(Sounds::coin).play();

			applyEffect(level, player);
			dead = true;
		}
	}

	Entity::tick(level);
}

void PowerUpEntity::draw(Level& level, TileSet& tileset, BufferWriter<Vert4f4b>& writer) {
	emitEntityQuad(level, writer, tileset.sprite(type, 1), size, angle, Color::white());
}
