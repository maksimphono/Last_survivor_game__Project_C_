// Actions must be initialized at this file

#include "objects.h"

void checkPlayer(int tick) {
	if ((*(Player*)(player->child)).health <= 0) {
		setlinestyle(PS_SOLID, 2, NULL, 0);
		outtextxy(300, 300, L"GAME OVER");
		Sleep(3000);
		exit(0);
	}
}

const char* Stop_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	if (side == LEFT || side == RIGHT) *dx = 0;
	if (side == UP || side == DOWN) *dy = 0;
	return "Stop";
}

const char* Move_to_Target_Action(Entity* self, int tick) {
	//move_directly(self, self->target[_X], self->target[_Y], 5);
	if (!self->target[0] || !self->target[1]) return "";
	if (move_directly(self, self->target[_X], self->target[_Y], 5)) {
		self->target[_X] = 0;
		self->target[_Y] = 0;
		self->move_axis = 0;
	}
	return "Move directly";
}

const char* Plant_Grow_Action(Entity* self, int tick) {
	int old_height = self->figure->height;
	if (tick - (*(Plant*)(self->child)).start_grow == (*(Plant*)(self->child)).time_phase_1) {
		setFigure(self, 100, L"Projects//Test_animations//assets//mushroom.png");
		move(FIGURE, self->figure, 0, old_height - 100);
		self->lower_edge += old_height - 100;
		return "Grown";
	} else if (tick - (*(Plant*)(self->child)).start_grow == (*(Plant*)(self->child)).time_phase_2) {
		setFigure(self, 100, L"Projects//Test_animations//assets//bigtree.png");
		move(FIGURE, self->figure, 0, old_height - 100);
		self->lower_edge += old_height - 100;
		self->loop_action = NULL;
		return "Grown";
	}
	return "Grow process";
}

const char* Push_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	if (*dx && !move_with_collision(obstacle, *dx, *dy)) {
		*dx = 0;
	}
	else if (*dy && !move_with_collision(obstacle, *dx, *dy)) {
		*dy = 0;
	}
	return "Push";
}

const char* Reduce_Fullness_Action(Entity* self, int tick) {
	if ((*(Player*)(self->child)).fullness) {
		if (tick % 100 == 0) (*(Player*)(self->child)).fullness--;
	}
	else
		if (tick % 100 == 0) (*(Player*)(self->child)).health--;

	return "Hunger";
}