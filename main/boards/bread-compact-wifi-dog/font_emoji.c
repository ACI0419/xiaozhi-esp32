#include "lvgl.h"
#include <esp_log.h>

extern const lv_image_dsc_t emoji_1f636; // neutral
extern const lv_image_dsc_t emoji_1f642; // happy
extern const lv_image_dsc_t emoji_1f606; // laughing
extern const lv_image_dsc_t emoji_1f602; // funny
extern const lv_image_dsc_t emoji_1f614; // sad
extern const lv_image_dsc_t emoji_1f620; // angry
extern const lv_image_dsc_t emoji_1f62d; // crying
extern const lv_image_dsc_t emoji_1f60d; // loving
extern const lv_image_dsc_t emoji_1f633; // embarrassed
extern const lv_image_dsc_t emoji_1f62f; // surprised
extern const lv_image_dsc_t emoji_1f631; // shocked
extern const lv_image_dsc_t emoji_1f914; // thinking
extern const lv_image_dsc_t emoji_1f609; // winking
extern const lv_image_dsc_t emoji_1f60e; // cool
extern const lv_image_dsc_t emoji_1f60c; // relaxed
extern const lv_image_dsc_t emoji_1f924; // delicious
extern const lv_image_dsc_t emoji_1f618; // kissy
extern const lv_image_dsc_t emoji_1f60f; // confident
extern const lv_image_dsc_t emoji_1f634; // sleepy
extern const lv_image_dsc_t emoji_1f61c; // silly
extern const lv_image_dsc_t emoji_1f644; // confused

typedef struct emoji {
    const lv_image_dsc_t* emoji;
    uint32_t unicode;
} emoji_t;

static const void* get_imgfont_path(const lv_font_t * font, uint32_t unicode, uint32_t unicode_next, int32_t * offset_y, void * user_data) {
    static const emoji_t emoji_table[] = {
        { &emoji_1f636, 0x1f636 }, // neutral
        { &emoji_1f642, 0x1f642 }, // happy
        { &emoji_1f606, 0x1f606 }, // laughing
        { &emoji_1f602, 0x1f602 }, // funny
        { &emoji_1f614, 0x1f614 }, // sad
        { &emoji_1f620, 0x1f620 }, // angry
        { &emoji_1f62d, 0x1f62d }, // crying
        { &emoji_1f60d, 0x1f60d }, // loving
        { &emoji_1f633, 0x1f633 }, // embarrassed
        { &emoji_1f62f, 0x1f62f }, // surprised
        { &emoji_1f631, 0x1f631 }, // shocked
        { &emoji_1f914, 0x1f914 }, // thinking
        { &emoji_1f609, 0x1f609 }, // winking
        { &emoji_1f60e, 0x1f60e }, // cool
        { &emoji_1f60c, 0x1f60c }, // relaxed
        { &emoji_1f924, 0x1f924 }, // delicious
        { &emoji_1f618, 0x1f618 }, // kissy
        { &emoji_1f60f, 0x1f60f }, // confident
        { &emoji_1f634, 0x1f634 }, // sleepy
        { &emoji_1f61c, 0x1f61c }, // silly
        { &emoji_1f644, 0x1f644 }, // confused
    };

    for (size_t i = 0; i < sizeof(emoji_table) / sizeof(emoji_table[0]); i++) {
        if (emoji_table[i].unicode == unicode) {
            return emoji_table[i].emoji;
        }
    }
    return NULL;
}


const lv_font_t* font_emoji_init(void) {
    static lv_font_t* font = NULL;
    if (font == NULL) {
        font = lv_imgfont_create(64, get_imgfont_path, NULL);
        if (font == NULL) {
            LV_LOG_ERROR("Failed to allocate memory for emoji font");
            return NULL;
        }
        font->base_line = 0;
        font->fallback = NULL;
    }
    return font;
}

