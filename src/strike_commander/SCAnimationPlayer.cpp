//
//  SCAnimationPlayer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include "SCAnimationPlayer.h"
#include "SCMidGamesEnum.h"

static bool compareDrawEntries(const MIDGAME_DRAW_ENTRY &a, const MIDGAME_DRAW_ENTRY &b) {
    if (a.depth != b.depth) {
        return a.depth < b.depth;
    }
    return a.category < b.category;
}


void SCAnimationPlayer::initMid1(){
    this->loadShot("MID_1.IFF");
}


SCAnimationPlayer::SCAnimationPlayer() { this->fps_timer = SDL_GetTicks() / 10; }

SCAnimationPlayer::~SCAnimationPlayer(){
}

void SCAnimationPlayer::init(){
    TreEntry *midgames_entry = Assets.GetEntryByName(
        "..\\..\\DATA\\MIDGAMES\\MIDGAMES.PAK"
    );
    PakArchive *midgames_arch = new PakArchive();
    this->midgames.InitFromRAM("MIDGAMES.PAK", midgames_entry->data, midgames_entry->size);
    std::vector<std::string> midgames_files = {
        "MID1",
        "MID2",
        "MID3",
        "MID5",
        "MID12",
        "MID14",
        "MID15",
        "MID16",
        "MID17",
        "MID20",
        "MID36"
    };
    this->mid.clear();
    this->mid.shrink_to_fit();
    for (int i = 0; i < midgames_files.size(); i++) {
        std::string file_path = "..\\..\\DATA\\MIDGAMES\\" + midgames_files[i] + ".PAK";
        TreEntry *entry = Assets.GetEntryByName(file_path.c_str());
        if (entry == nullptr) {
            continue;
        }
        PakArchive *arch = new PakArchive();
        arch->InitFromRAM(midgames_files[i].c_str(), entry->data, entry->size);
        this->mid.push_back(arch);
    }
    if (this->midvoc.size() == 0) {

        for (int i = 0; i < midgames_files.size(); i++) {
            std::string file_path = "..\\..\\DATA\\MIDGAMES\\" + midgames_files[i] + "VOC.PAK";
            TreEntry *entry = Assets.GetEntryByName(file_path.c_str());
            if (entry == nullptr) {
                continue;
            }
            PakArchive *arch = new PakArchive();
            arch->InitFromRAM(file_path.c_str(), entry->data, entry->size);
            this->midvoc.push_back(arch);
        }
        std::string file_path = "..\\..\\DATA\\MIDGAMES\\MID33VOC.PAK";
        TreEntry *entry = Assets.GetEntryByName(file_path.c_str());
        if (entry != nullptr) {
            PakArchive *arch = new PakArchive();
            arch->InitFromRAM(file_path.c_str(), entry->data, entry->size);
            this->midvoc.push_back(arch);
        }
        file_path = "..\\..\\DATA\\MIDGAMES\\MID34VOC.PAK";
        entry = Assets.GetEntryByName(file_path.c_str());
        if (entry != nullptr) {
            PakArchive *arch = new PakArchive();
            arch->InitFromRAM(file_path.c_str(), entry->data, entry->size);
            this->midvoc.push_back(arch);
        }
    }
    TreEntry *optShapEntry = Assets.GetEntryByName(
        Assets.optshps_filename
    );
    this->optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    TreEntry *optPalettesEntry = Assets.GetEntryByName(
        Assets.optpals_filename
    );
    this->optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    
    this->initMid1();

    this->original_palette.CopyFromOtherPalette(VGA.getPalette(), false);
    this->previous_palette.CopyFromOtherPalette(VGA.getPalette(), false);
    this->palette.CopyFromOtherPalette(VGA.getPalette(), false);
    this->shot_counter = 0;
    this->fps_counter = 0;
    this->fps = 1;
    this->midgame_text_db = new RSMidgame();
    std::string mdtxtdb_file_path = "..\\..\\DATA\\MIDGAMES\\MIDGAMES.IFF";
    TreEntry *mdtxtdb_entry = Assets.GetEntryByName(mdtxtdb_file_path.c_str());
    this->midgame_text_db->InitFromRAM(mdtxtdb_entry->data, mdtxtdb_entry->size);
    RSFont *font{nullptr};
    PakEntry *font_entry = this->mid[0]->GetEntry(27);
    PakArchive *fontPak = new PakArchive();
    fontPak->InitFromRAM("MID1.PAK", font_entry->data, font_entry->size);
    font = new RSFont();
    font->InitFromPAK(fontPak);
    this->fonts.push_back(font);
    this->fonts.push_back(FontManager.GetFont("..\\..\\DATA\\FONTS\\CONVFONT.SHP"));
}

static char lipChar(const std::string &text, int index) {
    if (index < 0 || index >= (int)text.size()) {
        return 0;
    }
    return (char)std::tolower((unsigned char)text[index]);
}

bool SCAnimationPlayer::isInWindow(uint16_t frame_start, uint16_t nb_frames, int frame) {
    if (frame < frame_start) {
        return false;
    }
    if (nb_frames == 0) {
        return true;
    }
    return frame < frame_start + nb_frames;
}

MIDGAME_LAYER_REF SCAnimationPlayer::findLayer(MIDGAME_SHOT *shot, int16_t layer_id) {
    MIDGAME_LAYER_REF ref;
    for (auto bg : shot->background) {
        if (bg->common.layer_id == layer_id) {
            ref.common = &bg->common;
            ref.position = &bg->current_position;
            return ref;
        }
    }
    for (auto chara : shot->characters) {
        if (chara->common.layer_id == layer_id) {
            ref.common = &chara->common;
            ref.position = &chara->current_position;
            return ref;
        }
    }
    for (auto sprt : shot->sprites) {
        if (sprt->common.layer_id == layer_id) {
            ref.common = &sprt->common;
            ref.position = &sprt->current_position;
            return ref;
        }
    }
    for (auto fg : shot->foreground) {
        if (fg->common.layer_id == layer_id) {
            ref.common = &fg->common;
            ref.position = &fg->current_position;
            return ref;
        }
    }
    return ref;
}

Point2D SCAnimationPlayer::screenPosition(MIDGAME_SHOT *shot, const MIDGAME_LAYER_COMMON &common, Point2D position) {
    Point2D result = position;
    result.x += common.jitter_offset.x;
    result.y += common.jitter_offset.y;
    int16_t parent_id = common.parent_id;
    int depth = 0;
    while (parent_id >= 0 && depth < 16) {
        MIDGAME_LAYER_REF parent = findLayer(shot, parent_id);
        if (parent.common == nullptr) {
            break;
        }
        result.x += parent.position->x + parent.common->jitter_offset.x;
        result.y += parent.position->y + parent.common->jitter_offset.y;
        parent_id = parent.common->parent_id;
        depth++;
    }
    return result;
}

void SCAnimationPlayer::applyPath(MIDGAME_LAYER_COMMON &common, Point2D &position, int frame) {
    if (common.path.empty()) {
        return;
    }
    if (!isInWindow(common.frame_start, common.nb_frames, frame)) {
        return;
    }
    int index = frame - common.frame_start;
    if (index >= (int)common.path.size()) {
        index = (int)common.path.size() - 1;
    }
    position = common.path[index];
}

void SCAnimationPlayer::moveTowardsEnd(Point2D &position, Point2D end, Point2D velocity) {
    if ((velocity.x < 0 && position.x > end.x) || (velocity.x > 0 && position.x < end.x)) {
        position.x += velocity.x;
    }
    if ((velocity.y < 0 && position.y > end.y) || (velocity.y > 0 && position.y < end.y)) {
        position.y += velocity.y;
    }
}

void SCAnimationPlayer::jitterStep(MIDGAME_LAYER_COMMON &common) {
    if (std::rand() % 2 == 0) {
        int step = std::rand() % 2;
        if (std::rand() % 2 == 0) {
            common.jitter_offset.y += step;
        } else {
            common.jitter_offset.y -= step;
        }
    }
    if (std::rand() % 2 == 0) {
        int step = std::rand() % 2;
        if (std::rand() % 2 == 0) {
            common.jitter_offset.x += step;
        } else {
            common.jitter_offset.x -= step;
        }
    }
}

void SCAnimationPlayer::blitLayer(FrameBuffer *fb, FrameBuffer *texture, Point2D position, const MIDGAME_LAYER_COMMON &common, uint8_t mask) {
    if (common.clip) {
        for (int ty = 0; ty < 200; ty++) {
            int sy = ty + position.y;
            for (int tx = 0; tx < 320; tx++) {
                int sx = tx + position.x;
                if (sx < common.clip_min.x || sx > common.clip_max.x || sy < common.clip_min.y || sy > common.clip_max.y) {
                    texture->framebuffer[ty * 320 + tx] = mask;
                }
            }
        }
    }
    fb->blitWithMask(texture->framebuffer, position.x, position.y, 320, 200, mask);
}

void SCAnimationPlayer::blitAnchoredShape(FrameBuffer *fb, RLEShape *shape, Point2D position,
                                          const MIDGAME_LAYER_COMMON &common) {
    FrameBuffer *texture = new FrameBuffer(320, 200);
    texture->fillWithColor(255);
    texture->drawShape(shape);
    position.x -= shape->position.x + shape->GetLeft();
    position.y -= shape->position.y + shape->GetTop();
    blitLayer(fb, texture, position, common, 255);
    delete texture;
}

int SCAnimationPlayer::nextLipMouth(MIDGAME_SHOT_CHARACTER *chara, const std::string &text) {
    static const char *lip_table = "06891599098769362999252999";
    int mouth = -1;
    char c = lipChar(text, chara->lip_index);
    if (c == 0) {
        return -1;
    }
    chara->lip_index++;
    char next = lipChar(text, chara->lip_index);
    if (c == 'h') {
        chara->lip_index++;
    } else if (c == 'c') {
        if (next == 'h') {
            mouth = 6;
            chara->lip_index++;
        }
    } else if (c == 'e') {
        if (next < 'a' || next > 'z') {
            c = next;
            chara->lip_index++;
        } else if (next == 'r') {
            mouth = 6;
            chara->lip_index++;
        }
    } else if (c == 't') {
        if (next == 'h') {
            mouth = 9;
            chara->lip_index++;
        }
    }
    if (mouth == -1 && c >= 'a' && c <= 'z') {
        mouth = lip_table[c - 'a'] - '0' + 2;
    }
    return mouth;
}

void SCAnimationPlayer::updateLips(MIDGAME_SHOT_CHARACTER *chara, int frame) {
    if (chara->lips.empty()) {
        return;
    }
    int entry = -1;
    for (size_t i = 0; i < chara->lips.size(); i++) {
        if (frame >= chara->lips[i].frame_start) {
            entry = (int)i;
        }
    }
    if (entry != chara->lip_entry) {
        chara->lip_entry = entry;
        chara->lip_index = 0;
        if (entry >= 0) {
            printf("MIDGAME: %s, texte de levres \"%s\" a l'image %d\n", chara->character_name.c_str(),
                   chara->lips[entry].text.c_str(), frame);
        }
    }
    if (entry < 0) {
        chara->mouth = -1;
        return;
    }
    chara->mouth = nextLipMouth(chara, chara->lips[entry].text);
}

int SCAnimationPlayer::imageIndex(const MIDGAME_LAYER_COMMON &common, int value) {
    if (common.game_numbering) {
        return value + common.image_offset;
    }
    return value;
}

uint8_t SCAnimationPlayer::sequenceValue(const MIDGAME_LAYER_COMMON &common, int frame, bool repeat) {
    int index = frame - common.frame_start;
    int count = (int)common.sequence.size();
    if (index < 0) {
        index = 0;
    }
    if (index >= count) {
        if (repeat) {
            index = index % count;
        } else {
            index = count - 1;
        }
    }
    return common.sequence[index];
}

void SCAnimationPlayer::resetShot(MIDGAME_SHOT *shot) {
    for (auto bg : shot->background) {
        bg->current_position = bg->position_start;
        bg->common.jitter_offset = Point2D{};
    }
    for (auto sprt : shot->sprites) {
        if (sprt->common.game_numbering) {
            sprt->current_frame = imageIndex(sprt->common, sprt->start_frame > 0 ? sprt->start_frame : 0);
        } else if (sprt->start_frame > 0) {
            sprt->current_frame = sprt->start_frame;
        } else {
            sprt->current_frame = 1;
        }
        sprt->finished = false;
        if (sprt->common.game_numbering && sprt->image != nullptr && sprt->current_frame > spriteLast(sprt)) {
            int first = sprt->common.image_offset;
            int last = spriteLast(sprt);
            if (sprt->repeat_animation && last >= first) {
                sprt->current_frame = first + (sprt->current_frame - first) % (last - first + 1);
            } else {
                sprt->current_frame = last;
            }
        }
        sprt->current_position = sprt->position_start;
        sprt->common.jitter_offset = Point2D{};
    }
    for (auto fg : shot->foreground) {
        fg->current_position = fg->position_start;
        fg->common.jitter_offset = Point2D{};
    }
    for (auto chara : shot->characters) {
        chara->current_position = chara->position_start;
        chara->common.jitter_offset = Point2D{};
        chara->current_frame = 0;
        chara->lip_entry = -1;
        chara->lip_index = 0;
        chara->mouth = -1;
    }
    this->fade_start = 0;
    this->fade_done = false;
}

void SCAnimationPlayer::applyPaletteRef(const MIDGAME_PALETTE_REF &ref) {
    PakEntry *entry = nullptr;
    if (ref.archive.rfind("FACEPAL:", 0) == 0) {
        ConvAssetManager &conv = ConvAssetManager::getInstance();
        entry = conv.convPals.GetEntry(conv.GetFacePaletteID(ref.archive.substr(8)));
    } else if (ref.pak != nullptr) {
        entry = ref.pak->GetEntry(ref.entry);
    }
    if (entry == nullptr || entry->size == 0) {
        return;
    }
    ByteStream paletteReader;
    paletteReader.Set(entry->data, 772);
    this->palette.ReadPatch(&paletteReader);
}

int SCAnimationPlayer::textWidth(RSFont *font, const std::string &text) {
    RLEShape *shape = font->GetShapeForChar('A');
    if (shape == nullptr) {
        shape = font->GetShapeForChar('0');
    }
    int space = shape->GetWidth() - 2;
    int width = 0;
    for (char c : text) {
        if (c == ' ') {
            width += space;
            continue;
        }
        RLEShape *s = font->GetShapeForChar(c);
        if (s == nullptr) {
            continue;
        }
        width += s->GetWidth() + 1;
    }
    return width;
}

void SCAnimationPlayer::drawText(RSFont *font, MIDGAME_TEXT_LINE *textLine, const std::string &text) {
    static const int max_width = 300;
    Point2D position = textLine->position_start;
    if (textLine->align != 2) {
        VGA.getFrameBuffer()->printText(font, position, text, textLine->color);
        return;
    }
    RLEShape *letter = font->GetShapeForChar('A');
    if (letter == nullptr) {
        letter = font->GetShapeForChar('0');
    }
    int line_step = letter->GetHeight() + 2;
    std::vector<std::string> lines;
    size_t paragraph_start = 0;
    while (paragraph_start <= text.size()) {
        size_t paragraph_end = text.find('\n', paragraph_start);
        if (paragraph_end == std::string::npos) {
            paragraph_end = text.size();
        }
        std::string paragraph = text.substr(paragraph_start, paragraph_end - paragraph_start);
        std::string line;
        size_t start = 0;
        while (start <= paragraph.size()) {
            size_t end = paragraph.find(' ', start);
            if (end == std::string::npos) {
                end = paragraph.size();
            }
            std::string word = paragraph.substr(start, end - start);
            std::string candidate = line.empty() ? word : line + " " + word;
            if (!line.empty() && textWidth(font, candidate) > max_width) {
                lines.push_back(line);
                line = word;
            } else {
                line = candidate;
            }
            start = end + 1;
        }
        lines.push_back(line);
        paragraph_start = paragraph_end + 1;
    }
    for (size_t i = 0; i < lines.size(); i++) {
        position.x = (320 - textWidth(font, lines[i])) / 2;
        position.y = textLine->position_start.y + (int)i * line_step;
        VGA.getFrameBuffer()->printText(font, position, lines[i], textLine->color);
    }
}

void SCAnimationPlayer::drawBackground(FrameBuffer *fb, MIDGAME_SHOT *shot, MIDGAME_SHOT_BG *bg, int frame, int fpsupdate) {
    if (bg->image == nullptr) {
        return;
    }
    if (!isInWindow(bg->common.frame_start, bg->common.nb_frames, frame)) {
        return;
    }
    if (bg->image->GetNumImages() == 0) {
        return;
    }
    int shape = bg->shapeid;
    if (!bg->common.sequence.empty()) {
        uint8_t value = sequenceValue(bg->common, frame, false);
        shape = value == MIDGAME_NO_IMAGE ? -1 : imageIndex(bg->common, value);
    } else if (bg->common.game_numbering) {
        shape = imageIndex(bg->common, bg->shapeid);
    }
    if (shape >= 0 && shot->version >= 2) {
        if (fpsupdate && frame == bg->common.frame_start) {
            RLEShape *s = bg->image->GetShape(shape);
            printf("MIDGAME: calque %d, image %d en (%d, %d), ancrage (%d, %d)\n", bg->common.layer_id, shape,
                   bg->current_position.x, bg->current_position.y, s->position.x + s->GetLeft(),
                   s->position.y + s->GetTop());
        }
        blitAnchoredShape(fb, bg->image->GetShape(shape), screenPosition(shot, bg->common, bg->current_position),
                          bg->common);
    } else if (shape >= 0) {
        FrameBuffer *texture = new FrameBuffer(320, 200);
        texture->fillWithColor(255);
        texture->drawShape(bg->image->GetShape(shape));
        blitLayer(fb, texture, screenPosition(shot, bg->common, bg->current_position), bg->common, 255);
        delete texture;
    }
    if (fpsupdate) {
        if (bg->common.path.empty()) {
            moveTowardsEnd(bg->current_position, bg->position_end, bg->velocity);
        }
        if (bg->common.jitter) {
            jitterStep(bg->common);
        }
    }
}

void SCAnimationPlayer::drawCharacter(FrameBuffer *fb, MIDGAME_SHOT *shot, MIDGAME_SHOT_CHARACTER *chara, int frame,
                                      int fpsupdate) {
    if (chara->image == nullptr) {
        return;
    }
    if (!isInWindow(chara->common.frame_start, chara->common.nb_frames, frame)) {
        return;
    }
    if (chara->image->GetNumImages() == 0) {
        return;
    }
    RLEShape *shp = chara->image->GetShape(1);
    FrameBuffer *texture = new FrameBuffer(320, 200);
    texture->fillWithColor(255);
    texture->drawShape(shp);
    if (!chara->lips.empty()) {
        if (chara->mouth >= 0) {
            shp = chara->image->GetShape(chara->mouth + 1);
            texture->drawShape(shp);
        }
    } else if (chara->talking) {
        if (fpsupdate) {
            chara->current_frame = (chara->current_frame + 1) % 10;
        }
        shp = chara->image->GetShape(3 + chara->current_frame);
        texture->drawShape(shp);
    }
    shp = chara->image->GetShape(chara->head_id);
    texture->drawShape(shp);
    shp = chara->image->GetShape(chara->cloth_id);
    texture->drawShape(shp);
    shp = chara->image->GetShape(chara->expression_id);
    texture->drawShape(shp);
    blitLayer(fb, texture, screenPosition(shot, chara->common, chara->current_position), chara->common, 255);
    if (fpsupdate) {
        updateLips(chara, frame);
        if (chara->common.path.empty()) {
            moveTowardsEnd(chara->current_position, chara->position_end, chara->velocity);
        }
        if (chara->common.jitter) {
            jitterStep(chara->common);
        }
    }
    delete texture;
}

int SCAnimationPlayer::spriteLast(MIDGAME_SHOT_SPRITE *sprt) {
    int last = (int)sprt->image->GetNumImages() - 1;
    if (sprt->common.game_numbering) {
        if (sprt->repeat_animation) {
            return last;
        }
        if (sprt->end_frame > 0) {
            return imageIndex(sprt->common, sprt->end_frame);
        }
        int count = (int)sprt->image->GetNumImages() - sprt->common.image_offset;
        return imageIndex(sprt->common, count + sprt->end_frame);
    }
    if (sprt->end_frame > 0 && sprt->end_frame < last) {
        last = sprt->end_frame;
    }
    return last;
}

void SCAnimationPlayer::advanceSprite(MIDGAME_SHOT_SPRITE *sprt) {
    int last = spriteLast(sprt);
    if (sprt->current_frame < last) {
        sprt->current_frame++;
    } else if (sprt->repeat_animation) {
        sprt->current_frame = sprt->common.game_numbering ? sprt->common.image_offset : 1;
    } else {
        sprt->current_frame = last;
    }
    bool finished = !sprt->repeat_animation && sprt->current_frame >= last;
    if (finished && !sprt->finished) {
        printf("MIDGAME: calque %d, fin de l'animation sur l'image %d (%s)\n", sprt->common.layer_id,
               sprt->current_frame, sprt->current_frame < (int)sprt->image->GetNumImages() ? "gardee" : "disparait");
    }
    sprt->finished = finished;
}

void SCAnimationPlayer::drawSprite(FrameBuffer *fb, MIDGAME_SHOT *shot, MIDGAME_SHOT_SPRITE *sprt, int frame, int fpsupdate) {
    if (sprt->image == nullptr) {
        return;
    }
    if (!isInWindow(sprt->common.frame_start, sprt->common.nb_frames, frame)) {
        return;
    }
    RLEShape *first_shape = sprt->image->GetShape(sprt->common.game_numbering ? sprt->common.image_offset : 1);
    FrameBuffer *texture = new FrameBuffer(320, 200);
    texture->fillWithColor(255);
    if (sprt->keep_first_frame && shot->version < 2) {
        texture->drawShape(first_shape);
    }
    if (fpsupdate && sprt->common.path.empty()) {
        moveTowardsEnd(sprt->current_position, sprt->position_end, sprt->velocity);
    }
    bool visible = true;
    if (!sprt->common.sequence.empty()) {
        uint8_t value = sequenceValue(sprt->common, frame, sprt->repeat_animation);
        if (value == MIDGAME_NO_IMAGE) {
            visible = false;
        } else {
            sprt->current_frame = imageIndex(sprt->common, value);
        }
    }
    if (sprt->current_frame >= (int)sprt->image->GetNumImages()) {
        visible = false;
    }
    Point2D position = screenPosition(shot, sprt->common, sprt->current_position);
    if (shot->version >= 2) {
        if (fpsupdate && frame == sprt->common.frame_start && visible) {
            RLEShape *s = sprt->image->GetShape(sprt->current_frame);
            printf("MIDGAME: calque %d, animation image %d en (%d, %d), ancrage (%d, %d)\n", sprt->common.layer_id,
                   sprt->current_frame, sprt->current_position.x, sprt->current_position.y,
                   s->position.x + s->GetLeft(), s->position.y + s->GetTop());
        }
        if (sprt->keep_first_frame) {
            blitAnchoredShape(fb, first_shape, position, sprt->common);
        }
        if (visible) {
            blitAnchoredShape(fb, sprt->image->GetShape(sprt->current_frame), position, sprt->common);
        }
    } else {
        if (visible) {
            texture->drawShape(sprt->image->GetShape(sprt->current_frame));
        }
        blitLayer(fb, texture, position, sprt->common, texture->framebuffer[0]);
    }
    if (fpsupdate && sprt->common.sequence.empty()) {
        advanceSprite(sprt);
    }
    if (fpsupdate && sprt->common.jitter) {
        jitterStep(sprt->common);
    }
    delete texture;
}

void SCAnimationPlayer::runFrame(void){
    checkKeyboard();
    VGA.activate();
    MIDGAME_SHOT *shot = this->midgames_shots[1][shot_counter];
    if (!shot->full_screen) {
        VGA.getFrameBuffer()->clear();
    }
    if (shot->clear_palette || (shot->version >= 2 && !shot->keep_palette)) {
        for (int i = 0; i < 256; i++) {
            this->palette.colors[i].r = 0;
            this->palette.colors[i].g = 0;
            this->palette.colors[i].b = 0;
            this->palette.colors[i].a = 255;
        }
    } else if (shot->keep_palette) {
        this->palette.CopyFromOtherPalette(&this->previous_palette, false);
    } else {
        this->palette.CopyFromOtherPalette(&this->original_palette, false);
    }
    for (const MIDGAME_PALETTE_REF &ref : shot->palettes) {
        applyPaletteRef(ref);
    }
    VGA.setPalette(&this->palette);
    FrameBuffer *fb = VGA.getFrameBuffer();
    if (!shot->full_screen) {
        fb->fillWithColor(0);
    }
    int fpsupdate = 0;
    if (!pause) {
        if (shot->fps > 0) {
            uint64_t ticks = 60 / shot->fps;
            if (ticks < 1) {
                ticks = 1;
            }
            uint64_t now = (uint64_t)SDL_GetTicks() * 60 / 1000;
            if (now - this->frame_tick >= ticks) {
                fpsupdate = 1;
                this->frame_tick += ticks;
                if (now - this->frame_tick >= ticks) {
                    printf("MIDGAME: plan %d, retard de %llu tics, cadence recalee\n", shot_counter,
                           (unsigned long long)(now - this->frame_tick));
                    this->frame_tick = now;
                }
            }
        } else {
            fpsupdate = (SDL_GetTicks() / 10) - fps_timer > 6;
            if (fpsupdate) {
                fps_timer = (SDL_GetTicks() / 10);
            }
        }
    }
    int frame = this->fps - 1;

    if (shot->music == MIDGAME_MUSIC_STOP) {
        if (this->current_music != MIDGAME_MUSIC_STOP) {
            printf("MIDGAME: plan %d, arret de la musique\n", shot_counter);
            Mixer.stopMusic();
            this->current_music = MIDGAME_MUSIC_STOP;
        }
    } else if (this->current_music != shot->music && shot->music != MIDGAME_MUSIC_KEEP && shot->music != -1) {
        printf("MIDGAME: plan %d, musique %d\n", shot_counter, shot->music);
        Mixer.switchBank(0);
        Mixer.playMusic(shot->music);
        this->current_music = shot->music;
    }
    for (auto sound : shot->sounds) {
        if (!sound->played && sound->sound != nullptr && frame >= sound->frame_start) {
            printf("MIDGAME: plan %d, image %d, son %d (prevu image %d)\n", shot_counter, frame, sound->pak_entry_id,
                   sound->frame_start);
            Mixer.playSoundVoc(sound->sound->data, sound->sound->size);
            sound->played = true;
        }
    }
    for (auto bg : shot->background) {
        applyPath(bg->common, bg->current_position, frame);
    }
    for (auto chara : shot->characters) {
        applyPath(chara->common, chara->current_position, frame);
    }
    for (auto sprt : shot->sprites) {
        applyPath(sprt->common, sprt->current_position, frame);
    }
    for (auto fg : shot->foreground) {
        applyPath(fg->common, fg->current_position, frame);
    }
    for (auto bg : shot->background) {
        if (!isInWindow(bg->common.frame_start, bg->common.nb_frames, frame)) {
            continue;
        }
        if (bg->palette != 255) {
            ByteStream paletteReader;
            if (bg->pak_palette == nullptr) {
                paletteReader.Set(this->optPals.GetEntry(bg->palette)->data, 772);
            } else {
                if (bg->palette >= bg->pak_palette->GetNumEntries()) {
                    continue;
                }
                paletteReader.Set(bg->pak_palette->GetEntry(bg->palette)->data, 772);
            }
            this->palette.ReadPatch(&paletteReader);
        }
        if (bg->pal != nullptr && !bg->use_external_palette) {
            this->palette.CopyFromOtherPalette(bg->pal->GetColorPalette());
        }
    }
    for (auto bg: shot->foreground) {
        if (!isInWindow(bg->common.frame_start, bg->common.nb_frames, frame)) {
            continue;
        }
        if (bg->palette != 255) {
            ByteStream paletteReader;
            if (bg->pak_palette == nullptr) {
                paletteReader.Set(this->optPals.GetEntry(bg->palette)->data, 772);
            } else {
                if (bg->palette >= bg->pak_palette->GetNumEntries()) {
                    continue;
                }
                paletteReader.Set(bg->pak_palette->GetEntry(bg->palette)->data, 772);
            }
            
            this->palette.ReadPatch(&paletteReader);
        }
        if (bg->pal != nullptr && !bg->use_external_palette) {
            this->palette.CopyFromOtherPalette(bg->pal->GetColorPalette());
        }
    }
    for (auto sprt: shot->sprites) {
        if (!isInWindow(sprt->common.frame_start, sprt->common.nb_frames, frame)) {
            continue;
        }
        if (sprt->palette != 255) {
            ByteStream paletteReader;
            if (sprt->pak_palette == nullptr) {
                paletteReader.Set(this->optPals.GetEntry(sprt->palette)->data, 772);
            } else {
                if (sprt->palette >= sprt->pak_palette->GetNumEntries()) {
                    continue;
                }
                paletteReader.Set(sprt->pak_palette->GetEntry(sprt->palette)->data, 772);
            }
            this->palette.ReadPatch(&paletteReader);
        }
        if (sprt->pal != nullptr && !sprt->use_external_palette) {
            this->palette.CopyFromOtherPalette(sprt->pal->GetColorPalette());
        }
    }
    for (auto chara: shot->characters) {
        if (!isInWindow(chara->common.frame_start, chara->common.nb_frames, frame)) {
            continue;
        }
        if (chara->palette != 255) {
            ByteStream paletteReader;
            PakEntry *palEntry = nullptr;
            palEntry = ConvAssetManager::getInstance().convPals.GetEntry(chara->palette);
            if (palEntry == nullptr) {
                continue;
            }
            if (palEntry->size == 0) {
                continue;
            }
            paletteReader.Set(palEntry->data, 772);
            this->palette.ReadPatch(&paletteReader);
        }
    }
    if (shot->fade_in > 0 && !this->fade_done) {
        uint32_t now = SDL_GetTicks();
        if (this->fade_start == 0) {
            this->fade_start = now;
            printf("MIDGAME: plan %d, fondu d'apparition, pas %d\n", shot_counter, shot->fade_in);
        }
        int steps = 0;
        for (int i = 0; i < 256; i++) {
            steps = (std::max)(steps, (int)this->palette.colors[i].r >> 2);
            steps = (std::max)(steps, (int)this->palette.colors[i].g >> 2);
            steps = (std::max)(steps, (int)this->palette.colors[i].b >> 2);
        }
        uint32_t duration = (uint32_t)steps * shot->fade_in * 1000 / 70;
        float f = duration > 0 ? (float)(now - this->fade_start) / (float)duration : 1.0f;
        if (f >= 1.0f) {
            this->fade_done = true;
        } else {
            for (int i = 0; i < 256; i++) {
                this->palette.colors[i].r = (uint8_t)(this->palette.colors[i].r * f);
                this->palette.colors[i].g = (uint8_t)(this->palette.colors[i].g * f);
                this->palette.colors[i].b = (uint8_t)(this->palette.colors[i].b * f);
            }
            fpsupdate = 0;
        }
    }
    VGA.setPalette(&this->palette);

    std::vector<MIDGAME_DRAW_ENTRY> entries;
    for (size_t i = 0; i < shot->background.size(); i++) {
        entries.push_back({shot->background[i]->common.depth, 0, (int)i});
    }
    for (size_t i = 0; i < shot->characters.size(); i++) {
        entries.push_back({shot->characters[i]->common.depth, 1, (int)i});
    }
    for (size_t i = 0; i < shot->sprites.size(); i++) {
        entries.push_back({shot->sprites[i]->common.depth, 2, (int)i});
    }
    for (size_t i = 0; i < shot->foreground.size(); i++) {
        entries.push_back({shot->foreground[i]->common.depth, 3, (int)i});
    }
    std::stable_sort(entries.begin(), entries.end(), compareDrawEntries);
    for (const MIDGAME_DRAW_ENTRY &entry : entries) {
        if (entry.category == 0) {
            drawBackground(fb, shot, shot->background[entry.index], frame, fpsupdate);
        } else if (entry.category == 1) {
            drawCharacter(fb, shot, shot->characters[entry.index], frame, fpsupdate);
        } else if (entry.category == 2) {
            drawSprite(fb, shot, shot->sprites[entry.index], frame, fpsupdate);
        } else {
            drawBackground(fb, shot, shot->foreground[entry.index], frame, fpsupdate);
        }
    }
    if (!shot->full_screen) {
        for (int i = 0; i < MIDGAME_BAND_TOP; i++) {
            VGA.getFrameBuffer()->fillLineColor(i, 0x00);
        }
        for (int i = MIDGAME_BAND_BOTTOM + 1; i < 200; i++) {
            VGA.getFrameBuffer()->fillLineColor(i, 0x00);
        }
    }
    for (auto textLine : shot->textes) {
        if (textLine->key.empty()) {
            continue;
        }
        if (!isInWindow(textLine->frame_start, textLine->nb_frames, frame)) {
            continue;
        }
        if (textLine->id < 0 || textLine->id >= this->midgame_text_db->midgame_texts[textLine->key].DATA.size()) {
            continue;
        }
        if (textLine->font_id < 0 || textLine->font_id >= this->fonts.size()) {
            continue;
        }
        std::string text = this->midgame_text_db->midgame_texts[textLine->key].DATA[textLine->id];
        if (fpsupdate && frame == textLine->frame_start) {
            printf("MIDGAME: plan %d, texte %s %d, police %d, couleur %d, alignement %d : \"%s\"\n", shot_counter,
                   textLine->key.c_str(), textLine->id, textLine->font_id, textLine->color, textLine->align,
                   text.c_str());
        }
        drawText(this->fonts[textLine->font_id], textLine, text);
    }
    if (!pause) {
        fps_counter++;
        fps+=fpsupdate;
    } else if (pause && fps == 1) {
        resetShot(shot);
    }
    bool ended = fps > shot->nbframe;
    if (shot->end_layer >= 0) {
        ended = false;
        for (auto sprt : shot->sprites) {
            if (sprt->common.layer_id == shot->end_layer && sprt->finished) {
                ended = true;
            }
        }
    }
    if (shot->wait_sound) {
        for (auto sound : shot->sounds) {
            if (!sound->played && sound->sound != nullptr) {
                ended = false;
            }
        }
        if (Mixer.isSoundPlaying()) {
            ended = false;
        }
    }
    if (ended) {
        printf("MIDGAME: fin du plan %d a l'image %d\n", shot_counter, frame);
        fps = 1;
        for (auto sound : shot->sounds) {
            sound->played = false;
        }
        this->previous_palette.CopyFromOtherPalette(&this->palette, false);
        resetShot(shot);
        if (!stay_on_current_shot) {
            shot_counter++;
            if (shot_counter>this->midgames_shots[1].size()-1) {
                shot_counter = 0;
                if (auto_stop) {
                    Game->stopTopActivity();
                }
            }
            shot = this->midgames_shots[1][shot_counter];
            resetShot(shot);
            printf("MIDGAME: plan %d, %d images, cadence %d, attendre le son %d, garder la palette %d\n", shot_counter,
                   shot->nbframe, shot->fps, shot->wait_sound, shot->keep_palette);
        }
    }
    VGA.vSync();
}

void SCAnimationPlayer::loadShot(std::string filename) {
    SCAnimationArchive archive;
    this->midgames_shots[1].clear();
    archive.LoadFromFile(filename.c_str(), this->midgames_shots[1]);
    this->shot_counter = 0;
    this->fps = 1;
    this->frame_tick = (uint64_t)SDL_GetTicks() * 60 / 1000;
    for (auto shot : this->midgames_shots[1]) {
        resetShot(shot);
    }
}
