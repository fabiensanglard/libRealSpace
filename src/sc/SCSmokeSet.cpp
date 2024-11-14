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
    
    /*for (auto img_set: this->smoke_set->images) {
        for (int i=0; i<img_set->GetNumImages(); i++) {
            RLEShape *img = img_set->GetShape(i);
            size_t imgsize = img->GetWidth()*img->GetHeight();
            byte *imgdata = new byte[imgsize];
            size_t byteRead = 0;
            img->Expand(imgdata, &byteRead);
            texture *tex = new texture();
            glGenTextures(1, &tex->texture_id);
            tex->data = new Texel[imgsize];

            Texel *dst = tex->data;
            for (size_t i = 0; i < imgsize; i++) {
                Texel *rgba = this->palette.GetRGBColor(imgdata[i]);
                if (imgdata[i] == 255) {
                    rgba->a = 0;
                }
                *dst = *rgba;
                dst++;
            }

            glBindTexture(GL_TEXTURE_2D, tex->texture_id);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            
            this->textures.push_back(tex);
        }
    }*/
}