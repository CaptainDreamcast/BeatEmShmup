#include <prism/framerateselectscreen.h>
#include <prism/pvr.h>
#include <prism/physics.h>
#include <prism/file.h>
#include <prism/drawing.h>
#include <prism/log.h>
#include <prism/wrapper.h>
#include <prism/system.h>
#include <prism/stagehandler.h>
#include <prism/logoscreen.h>
#include <prism/mugentexthandler.h>

#include "gamescreen.h"
#include "playerhandler.h"
#include "titlescreen.h"
#include "storyscreen.h"

#ifdef DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

#endif


void exitGame() {
	shutdownPrismWrapper();

#ifdef DEVELOP
	abortSystem();
#else
	returnToMenu();
#endif
}

void setMainFileSystem() {
#ifdef DEVELOP
	setFileSystem("/pc");
#else
	setFileSystem("/cd");
#endif
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	setGameName("BEAT EM SHMUP");
	setScreenSize(640, 480);
	
	setMainFileSystem();	
	initPrismWrapperWithConfigFile("$/cd/data/config.cfg");
	setMainFileSystem();	
	setFont("$/rd/fonts/segoe.hdr", "$/rd/fonts/segoe.pkg");

	logg("Check framerate");
	FramerateSelectReturnType framerateReturnType = selectFramerate();
	if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
		exitGame();
	}
	
	addMugenFont(1, "font/1.fnt");
	addMugenFont(2, "font/2.fnt");
	addMugenFont(3, "font/3.fnt");

	resetGame();
	setCurrentStoryDefinitionFile("story/OUTRO.def");
	setWrapperTitleScreen(&TitleScreen);
	setScreenAfterWrapperLogoScreen(getLogoScreenFromWrapper());
	startScreenHandling(&TitleScreen);

	exitGame();
	
	return 0;
}


