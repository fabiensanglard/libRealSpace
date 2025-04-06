#include "precomp.h"
#include "SDL2/SDL_opengl_glext.h"

SCSmokeSet::SCSmokeSet(){

}
SCSmokeSet::~SCSmokeSet(){

}
void SCSmokeSet::Init(){

    IffLexer lexer;
    FileData *paletteFile = Assets.GetFileData("PALETTE.IFF");
    lexer.InitFromRAM(paletteFile->data, paletteFile->size);

    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();

    std::string path = "..\\..\\DATA\\OBJECTS\\SMOKESET.IFF";
    TreEntry *smk = Assets.GetEntryByName(path.c_str());

    RSSmokeSet *smoke_set = new RSSmokeSet();
    smoke_set->InitFromRam(smk->data, smk->size);
    this->smoke_set = smoke_set;
    int qsdf = 0;
    RSImageSet *img_set = this->smoke_set->images[0];
    for (auto img_set: this->smoke_set->images) {
        size_t numimages = img_set->GetNumImages();
        for (size_t i=0; i<numimages; i++) {
            RLEShape *img = img_set->GetShape(i);
            texture *tex = new texture();
            tex->height = img->GetHeight();
            tex->width = img->GetWidth();
            img->position.x = 0;
            img->position.y = 0;
            img->buffer_size.x = tex->width;
            img->buffer_size.y = tex->height;
            size_t imgsize = tex->width*tex->height;
            uint8_t *imgdata = (uint8_t *)malloc(imgsize);
            size_t byteRead = 0;
            img->Expand(imgdata, &byteRead);
            if (byteRead > imgsize) {
                printf("RLEShape::Expand failed\n");
            }
            
            tex->data = (uint8_t *)malloc(imgsize*4);

            uint8_t *dst = tex->data;
            long checksum = 0;
            for (size_t j = 0; j < imgsize; j++) {
                Texel *rgba = this->palette.GetRGBColor(imgdata[j]);
                if (imgdata[j] == 255) {
                    rgba->a = 0;
                }
                if (rgba->a == 255 && rgba->r == 210 && rgba->g == 208 && rgba->b == 208) {
                    rgba->a = 0;
                }
                dst[0] = rgba->r;
                dst[1] = rgba->g;
                dst[2] = rgba->b;
                dst[3] = rgba->a;
                dst += 4;
            }
            tex->initialized = false;
            
            this->textures.push_back(tex);
        }
    }
}