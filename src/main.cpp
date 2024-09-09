#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

CCSprite* jesus_christ = nullptr;

float time_counter = 0.0;
float last_jesus_time = -1000.0;

bool isImageValid = false;

bool getBoolSetting(std::string key) {
	return Mod::get()->getSettingValue<bool>(key);
}
std::filesystem::path getFileSetting(std::string key) {
	return Mod::get()->getSettingValue<std::filesystem::path>(key);
}
bool modEnabled() {
	return getBoolSetting("enabled");
}
bool playLayerEnabled() {
	auto gjbgl = GJBaseGameLayer::get();
	if (!gjbgl) return false;
	return getBoolSetting("playLayer") && typeinfo_cast<PlayLayer*>(gjbgl);
}
bool levelEditorLayerEnabled() {
	auto gjbgl = GJBaseGameLayer::get();
	if (!gjbgl) return false;
	return getBoolSetting("levelEditorLayer") && typeinfo_cast<LevelEditorLayer*>(gjbgl);
}
void resetJesus() {
	time_counter = 0.0;
	last_jesus_time = -1000.0;
}

class $modify(MyGJBaseGameLayer, GJBaseGameLayer) {
	void jesus() {
		if (!modEnabled() || (!playLayerEnabled() && !levelEditorLayerEnabled())) return;
		
		auto scene = CCDirector::get()->getRunningScene();

		// A section of this code was copied from https://github.com/NicknameGG/robtop-jumpscare
		if (!scene->getChildByID("jesus"_spr)) {
			if (!getBoolSetting("customImage") || !isImageValid) jesus_christ = CCSprite::create("Jesus.png"_spr);
			else jesus_christ = CCSprite::create(getFileSetting("customImage").string.c_str());
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

		if (getFileSetting("customSound") == "Please choose an audio file.") FMODAudioEngine::sharedEngine()->playEffect("bell.ogg"_spr);
		else FMODAudioEngine::sharedEngine()->playEffect(getFileSetting("customSound").string.c_str());

		if (jesus_christ->getActionByTag(1)) jesus_christ->stopActionByTag(1);

		jesus_christ->setOpacity(255);
		jesus_christ->runAction(CCFadeOut::create(1.0))->setTag(1);
	}

	void update(float dt) {
		GJBaseGameLayer::update(dt);
		if (!modEnabled() || (!playLayerEnabled() && !levelEditorLayerEnabled())) return;
		time_counter += dt;
	}

	void collisionCheckObjects(PlayerObject* plr, gd::vector<GameObject*>* objs, int v0, float v1) {
		GJBaseGameLayer::collisionCheckObjects(plr, objs, v0, v1);
		
		if (!modEnabled() || (!playLayerEnabled() && !levelEditorLayerEnabled())) return;

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

	bool init() {
		if (!GJBaseGameLayer::init()) return false;
		if (!modEnabled() || (!playLayerEnabled() && !levelEditorLayerEnabled())) return true;

		resetJesus();
		bool isImageValid = false;

		CCSprite* test = CCSprite::create(getFileSetting("customImage").string.c_str());
		if (!test) isImageValid = false;
		else isImageValid = true;
		
		return true;
	}
};

class $modify(MyPlayerObject, PlayerObject) {
	void playerDestroyed(bool p0) {
		PlayerObject::playerDestroyed(p0);
		if (!modEnabled() || (!playLayerEnabled() && !levelEditorLayerEnabled())) return;
		resetJesus();
		auto gjbgl = GJBaseGameLayer::get();
		if (!gjbgl) return;
		log::info("isLevelEditor: {}", typeinfo_cast<bool>(typeinfo_cast<LevelEditorLayer*>(gjbgl)));
	}
};