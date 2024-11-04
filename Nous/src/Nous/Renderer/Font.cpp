#include "pch.h"
#include "Font.h"

#include <msdf-atlas-gen.h>
#include <FontGeometry.h>
#include <GlyphGeometry.h>

#include "MSDFData.h"

namespace Nous
{
    template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
    static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fonstSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
        const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height)
    {
        msdf_atlas::GeneratorAttributes attributes;
        attributes.config.overlapSupport = true;
        attributes.scanlinePass = true;

        msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
        generator.setAttributes(attributes);
        generator.setThreadCount(8);
        generator.generate(glyphs.data(), (int)glyphs.size());

        msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

        TextureSpecification spec;
        spec.Width = bitmap.width;
        spec.Height = bitmap.height;
        spec.MinFilter = ImageFilter::Linear;
        spec.MaxFilter = ImageFilter::Linear;
        spec.Format = ImageFormat::RGB8;
        spec.GenerateMips = false;

        Ref<Texture2D> texture = Texture2D::Create(spec);
        texture->SetData(Buffer((void*)bitmap.pixels, bitmap.width * bitmap.height * 3));
        return texture;
    }

    Font::Font(const std::filesystem::path& filepath)
        : m_Data(new MSDFData())
    {
        msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
        
        NS_CORE_ASSERT(ft);

        std::string fileString = filepath.string();

        msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());
        if (!font)
        {
            NS_CORE_ASSERT("无法加载字体: {}", fileString);
            return;
        }

        struct CharsetRange
        {
            uint32_t Begin, End;
        };

        // From imgui_draw.cpp
        static const CharsetRange charsetRanges[] =
        {
           {0x0020, 0x00FF}, // Basic Latin + Latin Supplement
           //{0x2000, 0x206F}, // General Punctuation
           //{0x3000, 0x30FF}, // CJK Symbols and Punctuations, Hiragana, Katakana
           //{0x31F0, 0x31FF}, // Katakana Phonetic Extensions
           //0xFF00, 0xFFEF}, // Half-width characters
           //{0x4e00, 0x9FAF}  // CJK Ideograms
           // 资源耗费过大，只使用拉丁字母
        };

        msdf_atlas::Charset charset;
        for (auto range : charsetRanges)
        {
            for (uint32_t c = range.Begin; c <= range.End; c++)
                charset.add(c);
        }

        double fontScale = 1.0;
        m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
        int glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
        NS_CORE_INFO("成功加载 {} / {} 个字形", glyphsLoaded, charset.size());

        double emSize = 30.0;

        msdf_atlas::TightAtlasPacker atlasPacker;
        atlasPacker.setPixelRange(2.0);
        atlasPacker.setMiterLimit(2.0);
        atlasPacker.setScale(emSize);
        atlasPacker.setInnerPixelPadding(0);
        int remaining = atlasPacker.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());
        NS_CORE_ASSERT(remaining == 0);

        int width, height;
        atlasPacker.getDimensions(width, height);
        emSize = atlasPacker.getScale();

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MUTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8

        uint64_t coloringSeed = 0;
        bool expensiceColoring = false;
        if (expensiceColoring)
        {
            msdf_atlas::Workload([&glyphs = m_Data->Glyphs, &coloringSeed](int i, int threadNo) -> bool {
                uint64_t glyphSeed = (LCG_MUTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT, glyphSeed);
                glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
                return true;
            }, m_Data->Glyphs.size()).finish(THREAD_COUNT);
        }
        else
        {
            uint64_t glyphSeed = coloringSeed;
            for (msdf_atlas::GlyphGeometry& glyph : m_Data->Glyphs)
            {
                glyphSeed *= LCG_MUTIPLIER;
                glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
            }
        }

        m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);

#if 0
        msdfgen::Shape shape;
        if (msdfgen::loadGlyph(shape, font, 'A'))
        {
            shape.normalize();
            //                      max. angle
            msdfgen::edgeColoringSimple(shape, 3.0);
            //           image width, height
            msdfgen::Bitmap<float, 3> msdf(32, 32);
            //                     range, scale, translation
            msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
            msdfgen::saveBmp(msdf, "output.bmp");
        }
#endif
        msdfgen::destroyFont(font);
        msdfgen::deinitializeFreetype(ft);
    }

    Font::~Font()
    {
        delete m_Data;
    }

    Ref<Font> Font::GetDefault()
    {
        static Ref<Font> DefaultFont;
        if (!DefaultFont)
            DefaultFont = CreateRef<Font>("assets/fonts/NotoSansSC/NotoSansSC-Regular.ttf");
        return DefaultFont;
    }
}