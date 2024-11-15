#include "precomp.h"
#include "SDL2/SDL_opengl_glext.h"

SCSmokeSet::SCSmokeSet(){

}
SCSmokeSet::~SCSmokeSet(){

}
void SCSmokeSet::Init(){

    IffLexer lexer;
    lexer.InitFromFile("PALETTE.IFF");

    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();

    std::string path = "..\\..\\DATA\\OBJECTS\\SMOKESET.IFF";
    TreArchive *tre = Assets.tres[AssetManager::TRE_OBJECTS];
    TreEntry *smk = tre->GetEntryByName(path.c_str());

    RSSmokeSet *smoke_set = new RSSmokeSet();
    smoke_set->InitFromRam(smk->data, smk->size);
    this->smoke_set = smoke_set;
    int qsdf = 0;
    RSImageSet *img_set = this->smoke_set->images[0];
    //for (auto img_set: this->smoke_set->images) {
        size_t numimages = img_set->GetNumImages();
        for (size_t i=0; i<numimages; i++) {
            RLEShape *img = img_set->GetShape(i);
            size_t imgsize = 320*200;
            byte *imgdata = (byte *)malloc(imgsize);
            size_t byteRead = 0;
            img->Expand(imgdata, &byteRead);
            if (byteRead > imgsize) {
                printf("RLEShape::Expand failed\n");
            }
            texture *tex = new texture();
            tex->data = (uint8_t *)malloc(imgsize*4);

            byte *dst = tex->data;
            long checksum = 0;
            for (size_t j = 0; j < imgsize; j++) {
                Texel *rgba = this->palette.GetRGBColor(imgdata[j]);
                if (imgdata[j] == 255) {
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
    //}
}