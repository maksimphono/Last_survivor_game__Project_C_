#include "define_graphic_types_new.c"
#pragma warning (disable : 4996)

int step = 1;
const int sleeptime = 1;
unsigned tick = 0;

void button_press(char btn, Entity* main_ch) {
	/*
	Manages control buttons
	*/
	static int x = 500, y = 400;

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
	case 'C':
		setPosition(main_ch, 200, 200);
		
		//paint_over(main_ch->figure);
		break;
	case 'B':
		visiable_bones = !visiable_bones;
		break;
	case 'T':
		registerEntity(x, y, "Train", L"assets\\train.png", Move_Left_Action, Go_Back_Action, "Prop:",
			init_vectorarr(1, x, y, x + 100, y),
			init_vectorarr(1, x, y + 100, x + 100, y + 100),
			init_vectorarr(1, x, y, x, y + 100),
			init_vectorarr(1, x + 100, y, x + 100, y + 100));
		main_entity_list.tail->object->vision_radius = 100;
		break;
	case 'R':
		kill_entnode(main_entity_list.tail);
		break;
	case 'K':
		kill_all_nodes(&main_entity_list);
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
	if (message->message == WM_KEYDOWN)
		button_press(message->vkcode, main_ch);
	
	if (message->message == WM_LBUTTONDOWN) {
		int x = message->x - 25, y = message->y - 25;
		registerEntity(x, y, "Box", L"assets\\box3.png", NULL, Push_Action, "Prop:",
			init_vectorarr(1, x, y, x + 50, y),
			init_vectorarr(1, x, y + 50, x + 50, y + 50),
			init_vectorarr(1, x, y, x, y + 50),
			init_vectorarr(1, x + 50, y, x + 50, y + 50));
		message->message = NULL;
	}
	else if (message->message == WM_RBUTTONDOWN) {
		setTarget(main_ch, "Points", message->x, message->y);
		main_ch->loop_action = Move_to_Target_Action;
		//registerEntity(main_ch->target[_X] - 20, main_ch->target[_Y] - 20, "Pointer", L"assets\\pointer.png", NULL, NULL, "Not");
		//circle(main_ch->target[_X] - 25, main_ch->target[_Y] - 25, 50);
	}
}

void mainloop(const char* arg) {
	MOUSEMSG message_mouse;
	ExMessage message_key;
	bool started = false;

	// creating entities
	
	registerEntity(20, 15, "none", L"assets\\box1.png", NULL, NULL, "without prop");
	registerEntity(200, 150, "none", L"assets\\plant.png", NULL, NULL, "without prop");
	registerEntity(400, 150, "none", L"assets\\plantNULL.png", NULL, NULL, "without prop");
	registerEntity(270, 180, "none", L"assets\\plant.png", NULL, NULL, "without prop");
	registerEntity(310, 190, "none", L"assets\\plant.png", NULL, NULL, "without prop");
	registerEntity(20, 540, "none", L"assets\\plant.png", NULL, NULL, "without prop");
	registerEntity(100, 30, "none", L"assets\\garage.png", NULL, NULL, "without prop");
	registerEntity(0, 420, "Box", L"assets\\railway.png", NULL, NULL, "without prop");
	registerEntity(100, 500, "main", MAIN_CH_MODEL_PATH, NULL, Kill_Action, "not Prop");
		//init_varray(1, 120, 100), init_varray(1, 120, 140), init_varray(1, 100, 120), init_varray(1, 140, 120));
	Entity* main_ch = main_entity_list.tail->object;
	setProp(main_ch, init_prop(BONES, 
		*init_vectorarr(1, 100, 500, 145, 540),
		*init_vectorarr(1, 100, 550, 150, 555),
		*init_vectorarr(1, 100, 500, 95, 555),
		*init_vectorarr(1, 150, 505, 160, 555)));

	int x = 100, y = 440;
	registerEntity(x, y, "Box", L"assets\\box3.png", NULL, Push_Action, "Prop:",
		init_vectorarr(1, x, y, x + 50, y),
		init_vectorarr(1, x, y + 50, x + 50, y + 50),
		init_vectorarr(1, x, y, x, y + 50),
		init_vectorarr(1, x + 50, y, x + 50, y + 50));
	y = 380;
	x = 105;
	registerEntity(x, y, "Box", L"assets\\box3.png", NULL, Push_Action, "Prop:",
		init_vectorarr(1, x, y, x + 50, y),
		init_vectorarr(1, x, y + 50, x + 50, y + 50),
		init_vectorarr(1, x, y, x, y + 50),
		init_vectorarr(1, x + 50, y, x + 50, y + 50));

	
	initgraph(SCREEN_WIDTH, SCREEN_HEIGHT); //		create main window and put image on it
	loadimage(&main_background, MAIN_BG_MODEL_PATH);
	bg = GetImageBuffer(&main_background);
	bg_src = GetImageBuffer(&main_background);
	Figure* main_bg_f = init_figure(0, 0, MAIN_BG_MODEL_PATH);
	
	
	GameField gf = *init_gamefield(L"", MAIN_BG_MODEL_PATH);
	
	//setupGameField();
	BeginBatchDraw();
	SetWorkingImage();
	//putimage(0, 0, &main_background);
	renderAll(true);
	for (;; tick++) {
		if (tick == MAX_INT) tick = 0;
		 //						 set working image as main window
		 //		 render background
		 // 	//setcliprgn(hrgn);// new
		
		//message_key.vkcode = '0';
		
		
		move_transparent_image(0, 0, main_bg_f, WHITE, true);
		//message_key.lbutton = false;
		//if (!started)
			//started = peekmessage(&message_key, EM_KEY | EM_MOUSE);//			get event
		//else
			//peekmessage(&message_key, EM_KEY | EM_MOUSE);//			get event
		getmessage(&message_key, EM_KEY | EM_MOUSE);
		
		event_manager(&message_key, &message_mouse, main_ch); //				manage event
		if (message_key.vkcode == 'B' || message_key.vkcode == 'R') message_key.vkcode = 128;

		//move(FIGURE, (&main_ch)->figure, 0, 0);
		//if (started) 
		all_actions(tick);
		renderAll(true);
		FlushBatchDraw();
		//Sleep(sleeptime);
	}
	EndBatchDraw();
	closegraph();
}