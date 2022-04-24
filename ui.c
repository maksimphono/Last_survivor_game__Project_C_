#include "define_graphic_types.c"
#pragma warning (disable : 4996)

int step = 2;
const int sleeptime = 1;
unsigned tick = 0;

void button_press(char btn, Entity* main_ch) {
	/*
		Manages control buttons('W', 'A', 'S', 'D')
	*/
	static int x = 0, y = 0;

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
	case 'B':
		show_hide_all_bones();
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

void event_manager(ExMessage* message, MOUSEMSG* mouse, Entity* main_ch) { // function, that manage all events from mainloop
	static int prev_cords[2] = {};
	if (message->message == WM_KEYDOWN)
		button_press(message->vkcode, main_ch);

	if (mouse != NULL) {
		putcircle(mouse->x, mouse->y, 1);
	}
}

void mainloop(const char* arg) {
	MOUSEMSG message_mouse;
	ExMessage message_key;

	// creating entities
	
	registerEntity(20, 15, L"assets\\box1.png", "without prop");
	registerEntity(200, 150, L"assets\\plant.png", "without prop");
	registerEntity(400, 150, L"assets\\plant.png", "without prop");
	registerEntity(270, 180, L"assets\\plant.png", "without prop");
	registerEntity(310, 190, L"assets\\plant.png", "without prop");
	registerEntity(20, 540, L"assets\\plant.png", "without prop");
	registerEntity(100, 30, L"assets\\garage.png", "without prop");
	registerEntity(400, 300, L"assets\\mushroom.png", "Prop:", 
		init_varray(1, 400, 305), init_varray(1, 400, 325), init_varray(1, 360, 315), init_varray(1, 450, 315));
	Entity main_ch = *init_entity(100, 100, MAIN_CH_MODEL_PATH);
	setProp(&main_ch, *init_varray(1, 120, 100), *init_varray(1, 120, 140), *init_varray(1, 100, 120), *init_varray(1, 140, 120));
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
		
		//message_key.vkcode = '0';
		move_transparent_image(0, 0, main_bg_f, WHITE, true);
		
		peekmessage(&message_key, EM_KEY);//			get event
		
		event_manager(&message_key, &message_mouse, &main_ch); //				manage event
		if (message_key.vkcode == 'B') message_key.vkcode = 128;

		//move(FIGURE, (&main_ch)->figure, 0, 0);
		renderAll(true);
		FlushBatchDraw();
		//Sleep(sleeptime);
	}
	EndBatchDraw();
	closegraph();
}