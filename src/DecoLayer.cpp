#include "DecoLayer.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

static ccColor4F accent(StyleDef const& s, int i, float a) {
    ccColor3B c = s.accents[((i % 3) + 3) % 3];
    return ccc4f(c.r / 255.f, c.g / 255.f, c.b / 255.f, a);
}

static float frand(float lo, float hi) {
    return lo + (hi - lo) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}

static void ring(CCDrawNode* n, CCPoint c, float r, float th, ccColor4F col) {
    constexpr int seg = 40;
    CCPoint prev = ccp(c.x + r, c.y);
    for (int i = 1; i <= seg; i++) {
        float a = i * 6.2831853f / seg;
        CCPoint p = ccp(c.x + cosf(a) * r, c.y + sinf(a) * r);
        n->drawSegment(prev, p, th, col);
        prev = p;
    }
}

DecoLayer* DecoLayer::create(StyleDef const& style, bool animate) {
    auto ret = new DecoLayer();
    if (ret->init(style, animate)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DecoLayer::init(StyleDef const& style, bool animate) {
    if (!CCNode::init()) return false;
    m_style = &style;
    m_animate = animate;
    m_win = CCDirector::get()->getWinSize();

    m_draw = CCDrawNode::create();
    m_draw->setBlendFunc({ GL_SRC_ALPHA, GL_ONE }); // additive glow
    this->addChild(m_draw);

    for (int i = 0; i < 70; i++) {
        Mote m;
        m.pos = ccp(frand(0, m_win.width), frand(0, m_win.height));
        m.size = frand(1.f, 4.f);
        m.phase = frand(0.f, 6.28f);
        m.col = rand() % 3;
        switch (style.id) {
            case DecoStyle::Neon:           m.vel = ccp(frand(-60, -20), frand(5, 30)); break;
            case DecoStyle::NineCircles:    m.vel = ccp(frand(-10, 10), frand(10, 40)); break;
            case DecoStyle::Slaughterhouse: m.vel = ccp(0, -frand(60, 200)); break;
            case DecoStyle::Grief:          m.vel = ccp(-30, -frand(300, 500)); break;
            case DecoStyle::Galaxy:         m.vel = ccp(-frand(2, 15), 0); break;
            case DecoStyle::Tech:           m.vel = ccp(0, -frand(80, 250)); break;
            case DecoStyle::Hell:           m.vel = ccp(frand(-10, 10), frand(40, 150)); break;
            case DecoStyle::Null:           m.vel = ccp(0, 0); break;
            case DecoStyle::Diamond:        m.vel = ccp(frand(-8, 8), -frand(5, 25)); break;
            case DecoStyle::Amethyst:       m.vel = ccp(frand(-8, 8), frand(5, 25)); break;
        }
        m_motes.push_back(m);
    }

    this->scheduleUpdate();
    return true;
}

void DecoLayer::update(float dt) {
    if (!m_animate) dt = 0.f;
    m_time += dt;
    m_pulse += ((m_intense ? 1.f : 0.f) - m_pulse) * std::min(1.f, dt * 6.f);
    float w = m_win.width, h = m_win.height;

    for (auto& m : m_motes) {
        m.pos = ccp(m.pos.x + m.vel.x * dt, m.pos.y + m.vel.y * dt);
        if (m.pos.x < -30.f) m.pos.x = w + 30.f;
        else if (m.pos.x > w + 30.f) m.pos.x = -30.f;
        if (m.pos.y < -40.f) { m.pos.y = h + 40.f; m.pos.x = frand(0, w); }
        else if (m.pos.y > h + 40.f) { m.pos.y = -40.f; m.pos.x = frand(0, w); }
        if (m_animate && m_style->id == DecoStyle::Null && rand() % 200 == 0) {
            m.pos = ccp(frand(0, w), frand(0, h));
        }
    }

    m_draw->clear();
    this->drawStyle();
}

void DecoLayer::drawStyle() {
    float w = m_win.width, h = m_win.height, t = m_time;
    auto col = [&](int i, float a) { return accent(*m_style, i, a); };
    ccColor4F none = ccc4f(0, 0, 0, 0);

    switch (m_style->id) {
        case DecoStyle::Neon: {
            float off = fmodf(t * 50.f, 90.f);
            for (float x = -off; x < w; x += 90.f)
                m_draw->drawSegment(ccp(x, 0), ccp(x, h), 0.5f, col(0, 0.15f));
            for (float y = 0; y < h; y += 60.f)
                m_draw->drawSegment(ccp(0, y), ccp(w, y), 0.5f, col(1, 0.15f));
            for (auto& m : m_motes)
                m_draw->drawDot(m.pos, m.size, col(m.col, 0.5f + 0.5f * sinf(t * 3.f + m.phase)));
            break;
        }
        case DecoStyle::NineCircles: {
            CCPoint c = ccp(w * 0.5f, h * 0.5f);
            float speed = 1.5f + 2.5f * m_pulse;
            float beat = powf(0.5f + 0.5f * sinf(t * speed * 2.f), 4.f);
            for (int i = 1; i <= 9; i++) {
                float r = i * 38.f + sinf(t * speed - i * 0.4f) * (8.f + 14.f * m_pulse)
                          + beat * 5.f * m_pulse;
                float th = 1.5f + 2.5f * beat * m_pulse;
                float al = 0.35f + 0.4f * beat * m_pulse;
                ring(m_draw, c, r, th, col(i, al));
            }
            // shockwave rings during wave
            if (m_pulse > 0.01f) {
                for (int k = 0; k < 3; k++) {
                    float p = fmodf(t * 0.8f + k / 3.f, 1.f);
                    ring(m_draw, c, p * h * 0.9f, 2.f, col(k, (1.f - p) * 0.6f * m_pulse));
                }
            }
            for (auto& m : m_motes)
                m_draw->drawDot(m.pos, m.size, col(m.col, 0.5f));
            break;
        }
        case DecoStyle::Slaughterhouse: {
            for (int i = 0; i < 7; i++) {
                float x = w * (i + 0.5f) / 7.f;
                float len = h * (0.35f + 0.1f * sinf(i * 3.1f));
                float sway = sinf(t + i) * 6.f;
                for (float y = h; y > h - len; y -= 14.f) {
                    float k = (h - y) / len;
                    CCPoint p = ccp(x + sway * k, y);
                    m_draw->drawDot(p, 3.f, col(2, 0.5f));
                    m_draw->drawSegment(p, ccp(p.x, p.y - 14.f), 1.f, col(2, 0.35f));
                }
                m_draw->drawDot(ccp(x + sway, h - len), 6.f, col(0, 0.6f));
            }
            for (auto& m : m_motes) {
                m_draw->drawSegment(m.pos, ccp(m.pos.x, m.pos.y + m.size * 8.f), 1.f, col(0, 0.7f));
                m_draw->drawDot(m.pos, m.size, col(0, 0.9f));
            }
            break;
        }
        case DecoStyle::Grief: {
            for (auto& m : m_motes)
                m_draw->drawSegment(m.pos, ccp(m.pos.x + 4.f, m.pos.y + m.size * 6.f), 0.5f, col(m.col, 0.3f));
            m_draw->drawDot(ccp(w * 0.8f, h * 0.8f), 50.f, col(0, 0.05f));
            m_draw->drawDot(ccp(w * 0.2f, h * 0.3f), 70.f, col(2, 0.05f));
            break;
        }
        case DecoStyle::Galaxy: {
            m_draw->drawDot(ccp(w * 0.3f + cosf(t * 0.1f) * 40.f, h * 0.6f), 120.f, col(1, 0.06f));
            m_draw->drawDot(ccp(w * 0.7f + sinf(t * 0.12f) * 40.f, h * 0.4f), 140.f, col(2, 0.05f));
            for (auto& m : m_motes)
                m_draw->drawDot(m.pos, m.size * 0.6f, col(m.col, 0.4f + 0.6f * (0.5f + 0.5f * sinf(t * 2.f + m.phase))));
            float ph = fmodf(t, 5.f) / 1.2f;
            if (ph < 1.f) {
                CCPoint head = ccp(w * (1.05f - ph * 0.8f), h * (0.95f - ph * 0.5f));
                CCPoint tail = ccp(head.x + 90.f, head.y + 45.f);
                m_draw->drawSegment(head, tail, 1.2f, col(0, 0.8f * (1.f - ph)));
                m_draw->drawDot(head, 2.5f, col(0, 1.f));
            }
            break;
        }
        case DecoStyle::Tech: {
            for (int i = 0; i < 6; i++) {
                float y0 = h * (i + 0.5f) / 6.f;
                float x1 = w * (0.25f + 0.1f * (i % 3));
                m_draw->drawSegment(ccp(0, y0), ccp(x1, y0), 0.6f, col(0, 0.25f));
                m_draw->drawSegment(ccp(x1, y0), ccp(x1 + 20.f, y0 + 20.f), 0.6f, col(0, 0.25f));
                m_draw->drawSegment(ccp(x1 + 20.f, y0 + 20.f), ccp(w, y0 + 20.f), 0.6f, col(0, 0.25f));
                m_draw->drawDot(ccp(x1 + 20.f, y0 + 20.f), 3.f, col(1, 0.6f));
                float px = fmodf(t * 90.f + i * 130.f, w);
                m_draw->drawDot(ccp(px, px < x1 ? y0 : y0 + 20.f), 2.5f, col(2, 0.9f));
            }
            for (auto& m : m_motes)
                m_draw->drawSegment(m.pos, ccp(m.pos.x, m.pos.y + m.size * 8.f), 1.f, col(m.col, 0.5f));
            break;
        }
        case DecoStyle::Hell: {
            for (float x = 0; x < w + 24.f; x += 24.f) {
                float fh = 30.f + 25.f * sinf(t * 4.f + x * 0.05f) + 15.f * sinf(t * 7.f + x * 0.13f);
                CCPoint v[3] = { ccp(x - 14.f, 0), ccp(x + 14.f, 0), ccp(x, fh) };
                m_draw->drawPolygon(v, 3, col(0, 0.35f), 0.f, none);
            }
            for (auto& m : m_motes)
                m_draw->drawDot(ccp(m.pos.x + sinf(t * 2.f + m.phase) * 6.f, m.pos.y), m.size,
                                col(m.col, 0.5f + 0.5f * sinf(t * 5.f + m.phase)));
            break;
        }
        case DecoStyle::Null: {
            for (auto& m : m_motes) {
                float bw = m.size * 6.f, bh = m.size * 1.5f;
                CCPoint v[4] = { ccp(m.pos.x, m.pos.y), ccp(m.pos.x + bw, m.pos.y),
                                 ccp(m.pos.x + bw, m.pos.y + bh), ccp(m.pos.x, m.pos.y + bh) };
                m_draw->drawPolygon(v, 4, col(m.col, 0.35f), 0.f, none);
            }
            float sy = fmodf(t * 180.f, h);
            m_draw->drawSegment(ccp(0, sy), ccp(w, sy), 1.f, col(1, 0.2f));
            // missing-texture checker in the corner
            for (int x = 0; x < 4; x++) {
                for (int y = 0; y < 4; y++) {
                    if ((x + y) % 2 != 0) continue;
                    float px = w - 80.f + x * 16.f, py = 16.f + y * 16.f;
                    CCPoint v[4] = { ccp(px, py), ccp(px + 16.f, py), ccp(px + 16.f, py + 16.f), ccp(px, py + 16.f) };
                    m_draw->drawPolygon(v, 4, col(2, 0.8f), 0.f, none);
                }
            }
            break;
        }
        case DecoStyle::Diamond: {
            CCPoint big[4] = { ccp(w * 0.5f, h * 0.95f), ccp(w * 0.5f + h * 0.45f, h * 0.5f),
                               ccp(w * 0.5f, h * 0.05f), ccp(w * 0.5f - h * 0.45f, h * 0.5f) };
            m_draw->drawPolygon(big, 4, none, 1.f, col(1, 0.12f));
            m_draw->drawSegment(big[1], big[3], 0.8f, col(1, 0.08f));
            for (auto& m : m_motes) {
                float ww = m.size * 5.f * fabsf(cosf(t * 0.8f + m.phase)) + 1.5f;
                float hh = m.size * 7.f;
                CCPoint v[4] = { ccp(m.pos.x, m.pos.y + hh), ccp(m.pos.x + ww, m.pos.y),
                                 ccp(m.pos.x, m.pos.y - hh), ccp(m.pos.x - ww, m.pos.y) };
                m_draw->drawPolygon(v, 4, col(m.col, 0.18f), 1.f, col(m.col, 0.7f));
                if (sinf(t * 3.f + m.phase) > 0.95f) {
                    m_draw->drawSegment(ccp(m.pos.x - 8.f, m.pos.y), ccp(m.pos.x + 8.f, m.pos.y), 0.8f, col(1, 0.9f));
                    m_draw->drawSegment(ccp(m.pos.x, m.pos.y - 8.f), ccp(m.pos.x, m.pos.y + 8.f), 0.8f, col(1, 0.9f));
                }
            }
            break;
        }
        case DecoStyle::Amethyst: {
            for (int i = 0; i < 15; i++) {
                float x = w * (i + 0.5f) / 15.f;
                float ch = 60.f + 80.f * (0.5f + 0.5f * sinf(i * 2.3f));
                float cw = 14.f + 8.f * sinf(i * 1.7f);
                float pulse = 0.5f + 0.5f * sinf(t * 1.5f + i);
                CCPoint v[5] = { ccp(x - cw, 0), ccp(x - cw * 0.5f, ch * 0.8f), ccp(x, ch),
                                 ccp(x + cw * 0.5f, ch * 0.8f), ccp(x + cw, 0) };
                m_draw->drawPolygon(v, 5, col(i, 0.1f + 0.15f * pulse), 1.f, col(i, 0.35f + 0.4f * pulse));
                CCPoint u[5] = { ccp(x - cw * 0.6f, h), ccp(x - cw * 0.3f, h - ch * 0.4f), ccp(x, h - ch * 0.5f),
                                 ccp(x + cw * 0.3f, h - ch * 0.4f), ccp(x + cw * 0.6f, h) };
                m_draw->drawPolygon(u, 5, col(i + 1, 0.1f), 1.f, col(i + 1, 0.3f + 0.3f * pulse));
            }
            for (auto& m : m_motes)
                m_draw->drawDot(m.pos, m.size * 0.7f, col(m.col, 0.3f + 0.5f * sinf(t * 2.f + m.phase) * sinf(t * 2.f + m.phase)));
            break;
        }
    }
}

// ---------- per-object decorations ----------

void drawObjectDeco(CCDrawNode* n, StyleDef const& s, CCPoint c, float hw, float hh, float rot, int idx) {
    float a = -rot * 3.14159265f / 180.f;
    float cs = cosf(a), sn = sinf(a);
    auto T = [&](float x, float y) { return ccp(c.x + x * cs - y * sn, c.y + x * sn + y * cs); };
    auto C = [&](float al, int off = 0) { return accent(s, idx + off, al); };
    ccColor4F none = ccc4f(0, 0, 0, 0);
    auto poly = [&](std::initializer_list<CCPoint> pts, ccColor4F fill, float bw, ccColor4F border) {
        std::vector<CCPoint> v(pts);
        n->drawPolygon(v.data(), static_cast<unsigned int>(v.size()), fill, bw, border);
    };
    float off = (((idx * 37) % 100) / 100.f - 0.5f) * hw * 1.6f;
    float m = std::min(hw, hh);

    switch (s.id) {
        case DecoStyle::Neon:
            poly({ T(-hw, -hh), T(hw, -hh), T(hw, hh), T(-hw, hh) }, none, 1.2f, C(0.9f));
            poly({ T(-hw * 0.6f, -hh * 0.6f), T(hw * 0.6f, -hh * 0.6f), T(hw * 0.6f, hh * 0.6f), T(-hw * 0.6f, hh * 0.6f) },
                 none, 0.8f, C(0.4f, 1));
            break;
        case DecoStyle::NineCircles:
            for (int k = 1; k <= 3; k++) ring(n, c, m * k * 0.5f + 2.f, 0.8f, C(0.6f / k, k));
            break;
        case DecoStyle::Slaughterhouse:
            n->drawSegment(T(off, -hh), T(off, -hh - 12.f), 1.5f, C(0.8f));
            n->drawDot(T(off, -hh - 12.f), 3.f, C(0.9f));
            n->drawSegment(T(-hw, hh * 0.5f), T(hw * 0.3f, -hh * 0.2f), 1.f, C(0.5f, 2));
            break;
        case DecoStyle::Grief:
            n->drawDot(T(off, 0), 2.5f, C(0.6f));
            n->drawSegment(T(off, 2.5f), T(off, hh * 0.7f), 0.8f, C(0.4f));
            poly({ T(-hw, -hh), T(hw, -hh), T(hw, hh), T(-hw, hh) }, none, 0.8f, C(0.25f, 1));
            break;
        case DecoStyle::Galaxy: {
            float l = m * 0.5f;
            n->drawSegment(T(off - l, 0), T(off + l, 0), 0.8f, C(0.9f));
            n->drawSegment(T(off, -l), T(off, l), 0.8f, C(0.9f));
            n->drawDot(T(off, 0), 1.5f, C(1.f));
            break;
        }
        case DecoStyle::Tech: {
            float l = m * 0.5f;
            float xs[2] = { -hw, hw }, ys[2] = { -hh, hh };
            for (float x : xs) {
                for (float y : ys) {
                    n->drawSegment(T(x, y), T(x - (x > 0 ? l : -l), y), 1.f, C(0.8f));
                    n->drawSegment(T(x, y), T(x, y - (y > 0 ? l : -l)), 1.f, C(0.8f));
                }
            }
            n->drawDot(T(0, 0), 1.5f, C(0.7f, 1));
            break;
        }
        case DecoStyle::Hell:
            poly({ T(-hw * 0.5f, hh), T(hw * 0.5f, hh), T(0, hh + hh * 1.2f) }, C(0.45f), 0.f, none);
            n->drawDot(T(off, hh + hh * 1.5f), 1.5f, C(0.9f, 1));
            break;
        case DecoStyle::Null:
            poly({ T(-hw + 3.f, -hh - 2.f), T(hw + 3.f, -hh - 2.f), T(hw + 3.f, hh - 2.f), T(-hw + 3.f, hh - 2.f) },
                 none, 1.f, accent(s, 2, 0.7f));
            poly({ T(-hw - 3.f, -hh + 2.f), T(hw - 3.f, -hh + 2.f), T(hw - 3.f, hh + 2.f), T(-hw - 3.f, hh + 2.f) },
                 none, 1.f, accent(s, 0, 0.7f));
            break;
        case DecoStyle::Diamond:
            poly({ T(0, hh), T(hw, 0), T(0, -hh), T(-hw, 0) }, C(0.15f), 1.2f, C(0.9f));
            n->drawDot(T(0, 0), 1.5f, C(1.f, 1));
            break;
        case DecoStyle::Amethyst:
            poly({ T(-hw * 0.4f, hh), T(-hw * 0.2f, hh + hh * 0.8f), T(0, hh + hh * 1.3f),
                   T(hw * 0.2f, hh + hh * 0.8f), T(hw * 0.4f, hh) }, C(0.3f), 1.f, C(0.8f));
            break;
    }
}
