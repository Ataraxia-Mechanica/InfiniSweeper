#ifndef RAYLIB_CPP_INCLUDE_FONT_HPP_
#define RAYLIB_CPP_INCLUDE_FONT_HPP_

#include <string>

#include "./raylib.hpp"
#include "./raylib-cpp-utils.hpp"
#include "./RaylibException.hpp"

namespace raylib {
/**
 * Font type, includes texture and charSet array data
 */
class Font : public ::Font {
 public:
    Font(int baseSize,
            int glyphCount,
            int glyphPadding,
            ::Texture2D texture,
            ::Rectangle *recs = nullptr,
            ::GlyphInfo *glyphs = nullptr) : ::Font{baseSize, glyphCount, glyphPadding, texture, recs, glyphs} {
        // Nothing.
    }

    /**
     * Retrieves the default Font.
     */
    Font() {
        set(::GetFontDefault());
    }

    Font(const ::Font& font) {
        set(font);
    }

    /**
     * Loads a Font from the given file.
     *
     * @param fileName The file name of the font to load.
     *
     * @throws raylib::RaylibException Throws if the given font failed to initialize.
     */
    Font(const std::string& fileName) {
        Load(fileName);
    }

    /**
     * Loads a Font from the given file, with generation parameters.
     *
     * @param fileName The file name of the font to load.
     *
     * @throws raylib::RaylibException Throws if the given font failed to initialize.
     *
     * @see ::LoadFontEx
     */
    Font(const std::string& fileName, int fontSize, int* fontChars = 0, int charCount = 0)  {
        Load(fileName, fontSize, fontChars, charCount);
    }

    /**
     * Loads a Font from the given image with a color key.
     *
     * @param image The image to load the fond from.
     *
     * @throws raylib::RaylibException Throws if the given font failed to initialize.
     *
     * @see ::LoadFontFromImage()
     */
    Font(const ::Image& image, ::Color key, int firstChar) {
        Load(image, key, firstChar);
    }

    /**
     * Loads a font from memory, based on the given file type and file data.
     *
     * @throws raylib::RaylibException Throws if the given font failed to initialize.
     *
     * @see ::LoadFontFromMemory()
     */
    Font(const std::string& fileType, const unsigned char* fileData, int dataSize, int fontSize,
            int *fontChars, int charsCount)  {
        Load(fileType, fileData, dataSize, fontSize, fontChars, charsCount);
    }

    Font(const Font&) = delete;

    Font(Font&& other) {
        set(other);

        other.baseSize = 0;
        other.glyphCount = 0;
        other.glyphPadding = 0;
        other.texture = {};
        other.recs = nullptr;
        other.glyphs = nullptr;
    }

    ~Font() {
        Unload();
    }

    void Unload() {
        UnloadFont(*this);
    }

    GETTERSETTER(int, BaseSize, baseSize)
    GETTERSETTER(int, GlyphCount, glyphCount)
    GETTERSETTER(int, GlyphPadding, glyphPadding)
    GETTERSETTER(::Texture2D, Texture, texture)
    GETTERSETTER(::Rectangle*, Recs, recs)
    GETTERSETTER(::GlyphInfo*, Glyphs, glyphs)

    Font& operator=(const ::Font& font) {
        Unload();
        set(font);
        return *this;
    }

    Font& operator=(const Font&) = delete;

    Font& operator=(Font&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        Unload();
        set(other);

        other.baseSize = 0;
        other.glyphCount = 0;
        other.glyphPadding = 0;
        other.texture = {};
        other.recs = nullptr;
        other.glyphs = nullptr;

        return *this;
    }

    /**
     * Loads a font from a given file.
     *
     * @param fileName The filename of the font to load.
     *
     * @throws raylib::RaylibException Throws if the given font failed to initialize.
     *
     * @see ::LoadFont()
     */
    void Load(const std::string& fileName) {
        set(::LoadFont(fileName.c_str()));
        if (!IsReady()) {
            throw new RaylibException("Failed to load Font with from file: " + fileName);
        }
    }

    /**
     * Loads a font from a given file with generation parameters.
     *
     * @param fileName The filename of the font to load.
     * @param fontSize The desired size of the font.
     *
     * @throws raylib::RaylibException Throws if the given font failed to initialize.
     *
     * @see ::LoadFontEx()
     */
    void Load(const std::string& fileName, int fontSize, int* fontChars, int charCount)  {
        set(::LoadFontEx(fileName.c_str(), fontSize, fontChars, charCount));
        if (!IsReady()) {
            throw new RaylibException("Failed to load Font with from file with font size: " + fileName);
        }
    }

    void Load(const ::Image& image, ::Color key, int firstChar) {
        set(::LoadFontFromImage(image, key, firstChar));
        if (!IsReady()) {
            throw new RaylibException("Failed to load Font with from image");
        }
    }

    void Load(const std::string& fileType, const unsigned char* fileData, int dataSize, int fontSize,
            int *fontChars, int charsCount)  {
        set(::LoadFontFromMemory(fileType.c_str(), fileData, dataSize, fontSize, fontChars,
            charsCount));
        if (!IsReady()) {
            throw new RaylibException("Failed to load Font " + fileType + " with from file data");
        }
    }

    /**
     * Returns if the font is ready to be used.
     */
    bool IsReady() {
        return baseSize > 0;
    }

    /**
     * Draw text using font and additional parameters.
     */
    inline Font& DrawText(const std::string& text, ::Vector2 position, float fontSize,
            float spacing, ::Color tint = WHITE) {
        ::DrawTextEx(*this, text.c_str(), position,  fontSize,  spacing,  tint);
        return *this;
    }

    /**
     * Draw text using font and additional parameters.
     */
    inline Font& DrawText(const std::string& text, int posX, int posY, float fontSize,
            float spacing, ::Color tint = WHITE) {
        ::DrawTextEx(*this, text.c_str(),
            { static_cast<float>(posX), static_cast<float>(posY) },
            fontSize, spacing, tint);
        return *this;
    }

    inline Font& DrawText(
            const std::string& text,
            ::Vector2 position,
            ::Vector2 origin,
            float rotation,
            float fontSize,
            float spacing,
            ::Color tint = WHITE) {
        ::DrawTextPro(*this, text.c_str(),
            position, origin,
            rotation, fontSize,
            spacing, tint);
        return *this;
    }

    /**
     * Draw one character (codepoint)
     */
    inline Font& DrawText(int codepoint,
            ::Vector2 position,
            float fontSize,
            ::Color tint = { 255, 255, 255, 255 }) {
        ::DrawTextCodepoint(*this, codepoint, position, fontSize, tint);
        return *this;
    }

    /**
     * Draw multiple character (codepoint)
     */
    inline Font& DrawText(const int *codepoints,
            int count, ::Vector2 position,
            float fontSize, float spacing,
            ::Color tint = { 255, 255, 255, 255 }) {
        ::DrawTextCodepoints(*this,
            codepoints, count,
            position, fontSize,
            spacing, tint);
        return *this;
    }

    /**
     * Measure string size for Font
     */
    inline Vector2 MeasureText(const std::string& text, float fontSize, float spacing) const {
        return ::MeasureTextEx(*this, text.c_str(), fontSize, spacing);
    }

    /**
     * Get index position for a unicode character on font
     */
    inline int GetGlyphIndex(int character) const {
        return ::GetGlyphIndex(*this, character);
    }

    /**
     * Create an image from text (custom sprite font)
     */
    inline ::Image ImageText(const std::string& text, float fontSize,
            float spacing, ::Color tint) const {
        return ::ImageTextEx(*this, text.c_str(), fontSize, spacing, tint);
    }

 private:
    void set(const ::Font& font) {
        baseSize = font.baseSize;
        glyphCount = font.glyphCount;
        glyphPadding = font.glyphPadding;
        texture = font.texture;
        recs = font.recs;
        glyphs = font.glyphs;
    }
};
}  // namespace raylib

using RFont = raylib::Font;

#endif  // RAYLIB_CPP_INCLUDE_FONT_HPP_
