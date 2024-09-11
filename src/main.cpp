#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

CCSprite* jesus_christ = nullptr;

float time_counter = 0.0;
float last_jesus_time = -1000.0;

bool isValidImage = false;

bool getBoolSetting(std::string_view key) {
	return Mod::get()->getSettingValue<bool>(key);
}
std::filesystem::path getFileSetting(std::string_view key) {
	return Mod::get()->getSettingValue<std::filesystem::path>(key);
}
std::string getFileSettingAsString(std::string_view key) {
	return getFileSetting(key).string();
}
int64_t getIntSetting(std::string_view key) {
	return Mod::get()->getSettingValue<int64_t>(key);
}
bool modEnabled() {
	return getBoolSetting("enabled");
}
bool isValidSprite(CCNode* obj) {
	return obj && !obj->getUserObject("geode.texture-loader/fallback");
}
bool playLayerEnabled() {
	#ifdef GEODE_IS_WINDOWS
	return true; // this defaults to returning true because CLion sucks at understanding the nuances of Geode settings; defaulting to false would ultimately have the same effect
	#endif
	auto gjbgl = GJBaseGameLayer::get();
	if (!gjbgl) return false;
	return getBoolSetting("playLayer") && typeinfo_cast<PlayLayer*>(gjbgl);
}
bool levelEditorLayerEnabled() {
	#ifdef GEODE_IS_WINDOWS
	return false;
	#endif
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
		if (!scene->getChildByIDRecursive("jesus"_spr)) {
			if (isValidImage && getFileSettingAsString("customImage") != "Please choose an image file.")
				jesus_christ = CCSprite::create(getFileSettingAsString("customImage").c_str());
			else jesus_christ = CCSprite::create("jesus.png"_spr);
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

		auto system = FMODAudioEngine::get()->m_system;
		FMOD::Channel* channel;
		FMOD::Sound* sound;
		if (getFileSettingAsString("customSound") != "Please choose an audio file.") system->createSound(getFileSettingAsString("customSound").c_str(), FMOD_DEFAULT, nullptr, &sound);
		else system->createSound((Mod::get()->getResourcesDir() / "bell.ogg").string().c_str(), FMOD_DEFAULT, nullptr, &sound);
		system->playSound(sound, nullptr, false, &channel);
		channel->setVolume(getIntSetting("volume") / 100.0f);

		if (jesus_christ->getActionByTag(1)) jesus_christ->stopActionByTag(1);

		jesus_christ->setOpacity(255);
		jesus_christ->runAction(CCFadeOut::create(1.0))->setTag(1);
	}

	void update(float dt) {
		GJBaseGameLayer::update(dt);
		if (!modEnabled() || (!playLayerEnabled() && !levelEditorLayerEnabled())) return;
		time_counter += dt;
	}

	void collisionCheckObjects(PlayerObject* player, gd::vector<GameObject*>* objs, int v0, float v1) {
		if (modEnabled() && (playLayerEnabled() || levelEditorLayerEnabled())) {
			float sensitivity = Mod::get()->getSettingValue<double>("sensitivity");
			for (auto obj : *objs) {
				if (obj == nullptr) continue;
				if (obj->m_objectType != GameObjectType::Hazard && obj->m_objectType != GameObjectType::AnimatedHazard) continue;
				if (getBoolSetting("skipInvisibleObjects") && obj->m_isHide || obj->getOpacity() == 0) continue;

				const auto sensitivityRect = CCRect(obj->getObjectRect().origin - CCPoint(sensitivity, sensitivity), obj->getObjectRect().size + CCPoint(sensitivity * 2, sensitivity * 2));

				if (player->getObjectRect().intersectsRect(sensitivityRect)) jesus();
			}
		}

		GJBaseGameLayer::collisionCheckObjects(player, objs, v0, v1);
	}

	void resetLevelVariables() {
		GJBaseGameLayer::resetLevelVariables();
		resetJesus();
	}
  
	bool init() {
		if (!GJBaseGameLayer::init()) return false;
		if (!modEnabled() || (!playLayerEnabled() && !levelEditorLayerEnabled())) return true;

		resetJesus();
		CCSprite* sprite = CCSprite::create(getFileSettingAsString("customImage").c_str());
		isValidImage = sprite;
		if (isValidImage) isValidImage = isValidSprite(sprite);
		log::info("isValidImage: {}", isValidImage);

		return true;
	}
};