//#include "Project_header.h"
#include "event_manager.c"
#include "objects.c"

void mainloop(const char* arg) {
	ExMessage message;

	GameField gf = *init_gamefield(L"", background_png);
	
	createObjects(&gf);
	
	initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	BeginBatchDraw();
	SetWorkingImage();
	for (;; tick++) {
		if (tick == MAX_INT) tick = 0;
		
		renderBG();
		
		getmessage(&message, EM_MOUSE | EM_KEY);

		event_manager(&message); //				manage event

		all_actions(tick);
		renderAll(true);
		FlushBatchDraw();
	}
	EndBatchDraw();
	closegraph();
}