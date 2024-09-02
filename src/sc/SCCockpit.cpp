#include "precomp.h"

SCCockpit::SCCockpit() {

}
SCCockpit::~SCCockpit(){

}
void SCCockpit::Init( ) {
    cockpit = new RSCockpit();
    TreEntry *cockpit_def = Assets.tres[AssetManager::TRE_OBJECTS]->GetEntryByName("..\\..\\DATA\\OBJECTS\\F16-CKPT.IFF");
    cockpit->InitFromRam(cockpit_def->data, cockpit_def->size);
}
void SCCockpit::Render() {
    
}