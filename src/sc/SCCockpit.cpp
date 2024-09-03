#include "precomp.h"

SCCockpit::SCCockpit() {

}
SCCockpit::~SCCockpit(){

}
void SCCockpit::Init( ) {
    IffLexer lexer;
    lexer.InitFromFile("PALETTE.IFF");
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();
    cockpit = new RSCockpit();
    TreEntry *cockpit_def = Assets.tres[AssetManager::TRE_OBJECTS]->GetEntryByName("..\\..\\DATA\\OBJECTS\\F16-CKPT.IFF");
    cockpit->InitFromRam(cockpit_def->data, cockpit_def->size);
}
void SCCockpit::Render(int face) {
    VGA.SwithBuffers();
    VGA.Activate();
    VGA.Clear();
    VGA.SetPalette(&this->palette);
    VGA.DrawShape(this->cockpit->ARTP.GetShape(face));
    VGA.VSync();
    VGA.SwithBuffers(); 
}