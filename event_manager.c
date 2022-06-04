#include "objects.c"

const int sleeptime = 1;

void button_press(char btn, Entity* player) {
	/*
	Manages control buttons
	*/
	static int x = 500, y = 400;
	int step = (*(Player*)(player->child)).hvstep;

	if (isdigit(btn)) {
		main_player->item_in_hand = btn - '0';
		return;
	}
	switch (btn) {
	case 'W':
		move_with_collision(player, 0, -step);
		break;
	case 'S':
		move_with_collision(player, 0, step);
		break;
	case 'A':
		move_with_collision(player, -step, 0);
		break;
	case 'D':
		move_with_collision(player, step, 0);
		break;
	case 'C':
		setPosition(player, 200, 200);
		break;
	case 'B':
		visiable_bones = !visiable_bones; // when visiable_bones set as true, bones of objects will be rendered
		break;
	case 'K':
		saveGameField(getWorkingField(), project_name);
		break;
	case 'Q':
		drop_item_from_hand();
		break;
	}
}

void event_manager(ExMessage* message) { // function, that manage all events from mainloop
	static int prev_x = 0, prev_y = 0;

	LPCTSTR t = L"";

	setbkmode(TRANSPARENT);

	if (message->message == WM_KEYDOWN)
		button_press(message->vkcode, player);

	if (message->message == WM_LBUTTONDOWN) {
		int x = message->x, y = message->y;
		Item* item = main_player->items[main_player->item_in_hand - 1];
		if (item != NULL && (ull)item != 0xFFFFFFFFFFFFFFDF && item->use != NULL && (ull)item->use != 0xcdcdcdcdcdcdcdcd) {
			//main_player->items[main_player->item_in_hand - 1] = item->use(item, x, y);
			if (item->use(item, x, y) == NULL) {
				kill_entity(main_player->items[main_player->item_in_hand - 1]->parent);
				main_player->items[main_player->item_in_hand - 1] = NULL;
			}
		}
	}
	else if (message->message == WM_RBUTTONDOWN) {
		setTarget(player, "Points", message->x, message->y);
		//player->loop_action = Move_to_Target_Action;
	}
	/*
	if (message->message == WM_MOUSEMOVE) {
		setlinecolor(WHITE);
		circle(message->x, message->y, 50);
		prev_x = message->x;
		prev_y = message->y;
		outtextxy(prev_x, prev_y, t);
	}
	else {
		setlinecolor(WHITE);
		setlinestyle(PS_SOLID, 2, NULL, 0);
		circle(prev_x, prev_y, 50);
		outtextxy(prev_x, prev_y, t);
	}
	*/
}