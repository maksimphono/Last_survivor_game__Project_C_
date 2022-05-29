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
	if (self == player && obstacle == NULL) {
		workingGameField;
		switch (side) {
		case UP:
			loadGameField(getWorkingField()->upper);
			setPosition(player, player->center_x, SCREEN_WIDTH - player->center_y);
			return "Load GF";
		case DOWN:
			loadGameField(getWorkingField()->lower);
			setPosition(player, player->center_x, SCREEN_WIDTH - player->center_y);
			return "Load GF";
		case LEFT: loadGameField(getWorkingField()->left); return "Load GF";
		case RIGHT: loadGameField(getWorkingField()->right); return "Load GF";
		}
	}
	if (side == LEFT || side == RIGHT) *dx = 0;
	if (side == UP || side == DOWN) *dy = 0;
	return "Stop";
}

const char* Move_to_Target_Action(Entity* self, int tick) {
	//move_directly(self, self->target[_X], self->target[_Y], 5);
	if (player == self) {
		if (tick % PLAYER_SPEED) return "";
	}
	if (!self->target[0] || !self->target[1]) return "";
	if (move_directly(self, self->target[_X], self->target[_Y], (*(Player*)(self->child)).dstep)) {
		self->target[_X] = 0;
		self->target[_Y] = 0;
		self->move_axis = 0;
	}
	return "Move directly";
}

const char* Plant_Grow_Action(Entity* self, int tick) {
	int old_height = self->figure->height;
	Plant* plant = (Plant*)(self->child);
	if (tick - plant->start_grow == plant->time_phase_1) {
		setFigure(self, self->figure->height, plant->phase1_figure);
		plant->phase++;
	} else if (tick - plant->start_grow == plant->time_phase_2) {
		setFigure(self, self->figure->height, plant->phase2_figure);
		plant->phase++;
		self->loop_action = NULL;
	}
	
	return "Grow process";
}

const char* Push_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	if (self == player && obstacle == NULL) {
		workingGameField;
		switch (side) {
		case UP:
			loadGameField(getWorkingField()->upper);
			setPosition(player, player->center_x, SCREEN_HEIGHT - player->center_y);
			return "Load GF";
		case DOWN: 
			loadGameField(getWorkingField()->lower);
			setPosition(player, player->center_x, SCREEN_HEIGHT - player->center_y);
			return "Load GF";
		case LEFT:
			loadGameField(getWorkingField()->left);
			setPosition(player, SCREEN_WIDTH - player->center_x, player->center_y);
			return "Load GF";
		case RIGHT:
			loadGameField(getWorkingField()->right);
			setPosition(player, SCREEN_WIDTH - player->center_x, player->center_y);
			return "Load GF";
		}
	}
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
		if (tick % PLAYER_STARVATION_SPEED == 0) (*(Player*)(self->child)).fullness--;
	}
	else
		if (tick % PLAYER_STARVATION_SPEED == 0) (*(Player*)(self->child)).health--;

	return "Hunger";
}