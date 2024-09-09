#include <Geode/Geode.hpp>

using namespace geode::prelude;

CCSprite* jesus_christ = nullptr;

float time_counter = 0.0;
float last_jesus_time = -1000.0;

#include <Geode/modify/GJBaseGameLayer.hpp>
class $modify(MyGJBaseGameLayer, GJBaseGameLayer) {
	void jesus() {
		auto scene = CCDirector::get()->getRunningScene();

		// A section of this code was copied from https://github.com/NicknameGG/robtop-jumpscare
		if (!scene->getChildByID("jesus"_spr)) {
			jesus_christ = CCSprite::create("Jesus.png"_spr);
			jesus_christ->setID("jesus"_spr);
			CCSize winSize = CCDirector::get()->getWinSize();

			float ratio_x = winSize.width / jesus_christ->getContentSize().width;
			float ratio_y = winSize.height / jesus_christ->getContentSize().height;
			float ratio = std::max(ratio_x, ratio_y);

			jesus_christ->setScaleX(ratio);
			jesus_christ->setScaleY(ratio);

			jesus_christ->setPosition({ winSize.width / 2, winSize.height / 2 });
			scene->addChild(jesus_christ, 100);
			jesus_christ->setOpacity(0);
		}

		if (time_counter < 1.5)
			return;

		if (time_counter - last_jesus_time < 0.2)
			return;
		last_jesus_time = time_counter;

		FMODAudioEngine::sharedEngine()->playEffect("bell.ogg"_spr);

		if (jesus_christ->getActionByTag(1))
      		jesus_christ->stopActionByTag(1);

		jesus_christ->setOpacity(255);
    	jesus_christ->runAction(CCFadeOut::create(1.0))->setTag(1);
	}

	void update(float dt) {
		GJBaseGameLayer::update(dt);

		time_counter += dt;
	}

	void collisionCheckObjects(PlayerObject* plr, gd::vector<GameObject*>* objs, int v0, float v1) {
		GJBaseGameLayer::collisionCheckObjects(plr, objs, v0, v1);

		float sensitivity = Mod::get()->getSettingValue<double>("sensitivity");

		for (auto obj : *objs) {
			if (obj->m_objectType != GameObjectType::Hazard && obj->m_objectType != GameObjectType::AnimatedHazard)
				continue;

			CCRect rect = obj->getObjectRect();
			rect = CCRect(rect.origin - CCPoint(sensitivity, sensitivity), rect.size + CCPoint(sensitivity * 2, sensitivity * 2));

			if (plr->getObjectRect().intersectsRect(rect))
				jesus();
		}
	}

	bool init() {
		if (!init()) return false;

		time_counter = 0.0;
		last_jesus_time = -1000.0;
		return true;
	}
};