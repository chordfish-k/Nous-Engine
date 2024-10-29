#pragma once

#include <vector>

#include "msdf-atlas-gen.h"

namespace Nous
{
    struct MSDFData
    {
        std::vector<msdf_atlas::GlyphGeometry> Glyphs;
        msdf_atlas::FontGeometry FontGeometry;
    };
}