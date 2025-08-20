#include "precomp.h"
#include "SDL2/SDL_opengl_glext.h"

SCSmokeSet::SCSmokeSet(){

}
SCSmokeSet::~SCSmokeSet(){

}
void SCSmokeSet::init(){

    RSPalette palette;
    palette.initFromFileData(Assets.GetFileData("PALETTE.IFF"));
    this->palette = *palette.GetColorPalette();

    std::string path = "..\\..\\DATA\\OBJECTS\\SMOKESET.IFF";
    TreEntry *smk = Assets.GetEntryByName(path.c_str());

    RSSmokeSet *smoke_set = new RSSmokeSet();
    smoke_set->InitFromRam(smk->data, smk->size);
    this->smoke_set = smoke_set;
    int qsdf = 0;
    for (auto img_set: this->smoke_set->images) {
        size_t numimages = img_set->GetNumImages();
        std::vector<Texture *> smk_set;

        for (size_t i=0; i<numimages; i++) {
            RLEShape *img = img_set->GetShape(i);
            Texture *tex = new Texture();
            tex->height = img->GetHeight();
            tex->width = img->GetWidth();
            img->position.x = 0;
            img->position.y = 0;
            img->buffer_size.x = (int32_t) tex->width;
            img->buffer_size.y = (int32_t) tex->height;
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
                if (rgba->a == 255 && rgba->r == 211 && rgba->g == 211 && rgba->b == 211) {
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
            smk_set.push_back(tex);
        }
        this->smoke_textures.push_back(smk_set);
    }
}