#include "define_graphic_types.c"
#pragma warning (disable : 4996)


const int sleeptime = 1;
unsigned tick = 0;

void button_press(char btn, Entity* main_ch) {
	/*
		Manages control buttons('W', 'A', 'S', 'D')
	*/
	static int x = 0, y = 0;
	int step = 3;
	switch (btn) {
	case 'W':
		move(ENTITY, main_ch, 0, -step);
		break;
	case 'S':
		move(ENTITY, main_ch, 0, step);
		break;
	case 'A':
		move(ENTITY, main_ch, -step, 0);
		break;
	case 'D':
		move(ENTITY, main_ch, step, 0);
		break;
	case 'Q':
		move(ENTITY, entarray[1], 0, step);
		break;
	case 'R':
		renderAll("first");
		break;
	case 'C':
		setPosition(ENTITY, main_ch, 200, 200);
		break;
	}
}

void putcircle(int x, int y, bool pressed) {
	/*
	circle, that follows mouse pointer
	*/
	static int prev_x = 0, prev_y = 0;
	setlinecolor(WHITE);
	if ((x || y) && pressed) {
		circle(x, y, 50);
		prev_x = x;
		prev_y = y;
	}
	else {
		circle(prev_x, prev_y, 50);
	}
}

void event_manager(ExMessage* message, Entity* main_ch) { // function, that manage all events from mainloop
	static int prev_cords[2] = {};
	if (message->message == WM_KEYDOWN)
		button_press(message->vkcode, main_ch);

	if (message->vkcode == 128 && message->message == WM_MOUSEMOVE) //  user moves the mouse
		putcircle(message->x, message->y, 1);
}

void mainloop(const char* arg) {
	ExMessage message_mouse;
	ExMessage message_key;

	// creating entities
	
	registerEntity(20, 15, L"assets\\box1.png");
	registerEntity(200, 150, L"assets\\plant.png");
	registerEntity(400, 150, L"assets\\plant.png");
	registerEntity(270, 180, L"assets\\plant.png");
	registerEntity(310, 190, L"assets\\plant.png");
	registerEntity(20, 540, L"assets\\plant.png");
	registerEntity(100, 30, L"assets\\garage.png");
	Entity main_ch = *init_entity(100, 100, MAIN_CH_MODEL_PATH);
	addEnt(&main_ch);

	initgraph(SCREEN_WIDTH, SCREEN_HEIGHT); //		create main window and put image on it
	loadimage(&main_background, MAIN_BG_MODEL_PATH);
	bg = GetImageBuffer(&main_background);
	bg_src = GetImageBuffer(&main_background);
	Figure* main_bg_f = init_figure(0, 0, MAIN_BG_MODEL_PATH);
	
	BeginBatchDraw();
	SetWorkingImage();
	putimage(0, 0, &main_background);
	renderAll("first");
	for (;; tick++) {
		if (tick == MAX_INT) tick = 0;
		 //						 set working image as main window
		 //		 render background
		 // 	//setcliprgn(hrgn);// new
		
		move_transparent_image(0, 0, main_bg_f, WHITE);
		peekmessage(&message_key, EM_MOUSE | EM_KEY);//			get event
		event_manager(&message_key, &main_ch); //				manage event
		
		//move(FIGURE, (&main_ch)->figure, 0, 0);
		renderAll("first");
		FlushBatchDraw();
		//Sleep(sleeptime);
	}
	EndBatchDraw();
	closegraph();
}