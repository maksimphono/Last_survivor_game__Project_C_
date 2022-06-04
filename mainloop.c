#include "event_manager.c"
#include <winuser.h>

void mainloop(const char* arg) {
	ExMessage message;

	// world creation V
	GameField gamefield_00 = *init_gamefield("gamefield_00.txt", background_00_png);
	GameField gamefield_01 = *init_gamefield("gamefield_01.txt", background_01_png);
	setNeighbours(&gamefield_00, &gamefield_01, NULL, NULL, NULL);
	// world creation /\
	
	createObjects00(&gamefield_00);
	createObjects01(&gamefield_01);

	setupGameField(&gamefield_00);
	
	initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	BeginBatchDraw();
	SetWorkingImage();
	for (;; tick++) {
		if (tick == 999999999) tick = 0;
		
		renderBG();
		
		peekmessage(&message, EM_MOUSE | EM_KEY);

		event_manager(&message); //				manage event
		if (message.message == WM_LBUTTONDOWN) message.message = NULL;
		all_actions(tick);
		Reduce_Fullness_Action(main_player->parent, tick);
		renderAll(true);
		showPlayerInfo();
		nextCadrAll(tick);
		checkPlayer(tick);
		FlushBatchDraw();

	}
	EndBatchDraw();
	closegraph();
}