#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

CCSprite* jesus_christ = nullptr;

float time_counter = 0.0;
float last_jesus_time = -1000.0;

bool isImageValid = false;

class $modify(MyGJBaseGameLayer, GJBaseGameLayer) {
	bool getBoolSetting(std::string key) {
		return Mod::get()->getSettingValue<bool>(key);
	}
	bool modEnabled() {
		return getBoolSetting("enabled");
	}
	bool playLayerEnabled() {
		return getBoolSetting("playLayer") && typeinfo_cast<PlayLayer>(this);
	}
	bool levelEditorLayerEnabled() {
		return getBoolSetting("levelEditorLayer") && typeinfo_cast<LevelEditorLayer>(this);
	}
	void jesus() {
		if (!modEnabled() || !playLayerEnabled() || !levelEditorLayerEnabled()) return;
		
		auto scene = CCDirector::get()->getRunningScene();

		// A section of this code was copied from https://github.com/NicknameGG/robtop-jumpscare
		if (!scene->getChildByID("jesus"_spr)) {
			if (!getBoolSetting("customImage") || !isImageValid) jesus_christ = CCSprite::create("Jesus.png"_spr);
			else jesus_christ = CCSprite::create(Mod::get()->getSettingValue<std::filesystem::path>("customImage"));
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

		if ((time_counter < 1.5) || (time_counter - last_jesus_time < 0.2)) return;
		last_jesus_time = time_counter;

		FMODAudioEngine::sharedEngine()->playEffect("bell.ogg"_spr);

		if (jesus_christ->getActionByTag(1)) jesus_christ->stopActionByTag(1);

		jesus_christ->setOpacity(255);
    		jesus_christ->runAction(CCFadeOut::create(1.0))->setTag(1);
	}

	void update(float dt) {
		GJBaseGameLayer::update(dt);
		if (!modEnabled() || !playLayerEnabled() || !levelEditorLayerEnabled()) return;
		time_counter += dt;
	}

	void collisionCheckObjects(PlayerObject* plr, gd::vector<GameObject*>* objs, int v0, float v1) {
		GJBaseGameLayer::collisionCheckObjects(plr, objs, v0, v1);
		
		if (!modEnabled() || !playLayerEnabled() || !levelEditorLayerEnabled()) return;

		float sensitivity = Mod::get()->getSettingValue<double>("sensitivity");

		for (auto obj : *objs) {
			if (obj->m_objectType != GameObjectType::Hazard && obj->m_objectType != GameObjectType::AnimatedHazard) continue;

			CCRect rect = CCRect(
				obj->getObjectRect().origin - CCPoint(sensitivity, sensitivity),
				obj->getObjectRect().size + CCPoint(sensitivity * 2, sensitivity * 2)
			);

			if (plr->getObjectRect().intersectsRect(rect)) jesus();
		}
	}

	void resetPlayer() {
		GJBaseGameLayer::resetPlayer();
		time_counter = 0.0;
		last_jesus_time = -1000.0;
	}

	bool init() {
		if (!GJBaseGameLayer::init()) return false;
		if (!modEnabled() || !playLayerEnabled() || !levelEditorLayerEnabled()) return true;

		time_counter = 0.0;
		last_jesus_time = -1000.0;
		bool isImageValid = false;

		CCSprite* test = CCSprite::create(Mod::get()->getSettingValue<std::filesystem::path>("customImage"));
		if (!test) isImageValid = false;
		else isImageValid = true;
		
		return true;
	}
};
