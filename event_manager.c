#include "Project_header.h"

int step = 1;
const int sleeptime = 1;
unsigned tick = 0;

void button_press(char btn, Entity* player) {
	/*
	Manages control buttons
	*/
	static int x = 500, y = 400;

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
	}
}

void event_manager(ExMessage* message) { // function, that manage all events from mainloop
	static int prev_x = 0, prev_y = 0;

	LPCTSTR t = L"Hello";

	setbkmode(TRANSPARENT);

	if (message->message == WM_KEYDOWN)
		button_press(message->vkcode, player);

	if (message->message == WM_LBUTTONDOWN) {
		int x = message->x - 25, y = message->y - 25;

		int points[4][MAX_VECTOR_NUM][4] = {
			{{x, y, x + 50, y}},
			{{ x, y + 50, x + 50, y + 50}},
			{{x, y, x, y + 50}},
			{{x + 50, y, x + 50, y + 50}}
		};

		createByPoints(x, y, "Box", box3_png, NULL, Stop_Action, points);
		message->message = NULL;
	}
	else if (message->message == WM_RBUTTONDOWN) {
		setTarget(player, "Points", message->x, message->y);
		player->loop_action = Move_to_Target_Action;
	}
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
}