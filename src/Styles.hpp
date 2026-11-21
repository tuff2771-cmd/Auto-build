#pragma once
#include <Geode/Geode.hpp>
using namespace geode::prelude;

enum class DecoStyle {
    Neon, NineCircles, Slaughterhouse, Grief, Galaxy,
    Tech, Hell, Null, Diamond, Amethyst
};

struct StyleDef {
    DecoStyle id;
    const char* name;
    ccColor3B bgTop;
    ccColor3B bgBottom;
    ccColor3B ground1;
    ccColor3B ground2;
    ccColor3B tint;
    ccColor3B accents[3];
};

StyleDef const& getStyle(std::string const& name);
