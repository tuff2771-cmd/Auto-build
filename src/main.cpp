#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
#include <cmath>
#include "DecoLayer.hpp"

using namespace geode::prelude;

class $modify(AutoDecoPlayLayer, PlayLayer) {
    struct Fields {
        DecoLayer* deco = nullptr;
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("enabled")) return;

        auto const& style = getStyle(mod->getSettingValue<std::string>("style"));
        float strength = static_cast<float>(mod->getSettingValue<double>("tint-strength"));
        float bgOpacity = static_cast<float>(mod->getSettingValue<double>("bg-opacity"));
        int density = std::max<int>(1, static_cast<int>(mod->getSettingValue<int64_t>("decor-density")));
        int maxDecos = static_cast<int>(mod->getSettingValue<int64_t>("max-object-decos"));
        bool animate = mod->getSettingValue<bool>("animate");

        auto win = CCDirector::get()->getWinSize();

        // 1) Background: style gradient + animated decoration layer
        CCNode* bgParent = this;
        int bgZ = -1;
        if (m_background && m_background->getParent()) {
            bgParent = m_background->getParent();
            bgZ = m_background->getZOrder() + 1;
        }
        auto alpha = static_cast<GLubyte>(255.f * bgOpacity);
        auto grad = CCLayerGradient::create(
            ccc4(style.bgTop.r, style.bgTop.g, style.bgTop.b, alpha),
            ccc4(style.bgBottom.r, style.bgBottom.g, style.bgBottom.b, alpha),
            ccp(0, -1)
        );
        bgParent->addChild(grad, bgZ);

        auto deco = DecoLayer::create(style, animate);
        m_fields->deco = deco;
        bgParent->addChild(deco, bgZ);

        // 2) Ground colors (if this errors on build, delete these 2 lines)
        if (m_groundLayer) m_groundLayer->updateGround01Color(style.ground1);
        if (m_groundLayer) m_groundLayer->updateGround02Color(style.ground2);

        // 3) Per-object decorations, all in one draw node
        if (m_objects && m_objectLayer && maxDecos > 0) {
            auto node = CCDrawNode::create();
            node->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            int i = 0, count = 0;
            for (auto obj : CCArrayExt<GameObject*>(m_objects)) {
                if (count >= maxDecos) break;
                if (i++ % density != 0) continue;
                auto size = obj->getContentSize();
                if (size.width < 1.f || size.height < 1.f) continue;
                float hw = std::min(size.width * 0.5f * std::fabs(obj->getScaleX()), 90.f);
                float hh = std::min(size.height * 0.5f * std::fabs(obj->getScaleY()), 90.f);
                drawObjectDeco(node, style, obj->getPosition(), hw, hh, obj->getRotation(), count);
                count++;
            }
            m_objectLayer->addChild(node, 100);
        }

        // 4) Whole-screen tint (multiply blend) so everything matches the style
        auto k = std::clamp(strength, 0.f, 1.f);
        auto mix = [&](GLubyte v) { return static_cast<GLubyte>(255.f + (static_cast<float>(v) - 255.f) * k); };
        auto overlay = CCLayerColor::create(ccc4(mix(style.tint.r), mix(style.tint.g), mix(style.tint.b), 255), win.width, win.height);
        overlay->setBlendFunc({ GL_DST_COLOR, GL_ZERO });
        this->addChild(overlay, 1000);
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (m_fields->deco && m_player1) {
            m_fields->deco->setIntense(m_player1->m_isDart);
        }
    }
};
