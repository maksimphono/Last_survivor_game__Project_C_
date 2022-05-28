#include "event_manager.c"

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
		
		peekmessage(&message, EM_MOUSE | EM_KEY);

		event_manager(&message); //				manage event

		all_actions(tick);
		renderAll(true);
		nextCadrAll(tick);
		FlushBatchDraw();
	}
	EndBatchDraw();
	closegraph();
}