#pragma once
#include "Styles.hpp"
#include <vector>

// Animated background decoration (sits above the game's bg, below objects)
class DecoLayer : public CCNode {
protected:
    struct Mote {
        CCPoint pos;
        CCPoint vel;
        float size;
        float phase;
        int col;
    };

    StyleDef const* m_style = nullptr;
    CCDrawNode* m_draw = nullptr;
    std::vector<Mote> m_motes;
    CCSize m_win;
    float m_time = 0.f;
    bool m_animate = true;
    bool m_intense = false;
    float m_pulse = 0.f;

    bool init(StyleDef const& style, bool animate);
    void update(float dt) override;
    void drawStyle();

public:
    static DecoLayer* create(StyleDef const& style, bool animate);
    void setIntense(bool v) { m_intense = v; }
};

// Draws one decoration around a level object into a shared draw node
void drawObjectDeco(CCDrawNode* n, StyleDef const& s, CCPoint c, float hw, float hh, float rot, int idx);
