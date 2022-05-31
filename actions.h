// Actions must be initialized at this file

#include "objects.h"

void loadGameField_with_items(GameField* gf) {
	if (gf == NULL) return;
	EntityNode* prev_node = getWorkingField()->object_list->tail->prev;
	GameField* working_gf = getWorkingField();
	for (EntityNode* node = getWorkingField()->object_list->tail; node->object->lower_edge == SCREEN_HEIGHT + 100 && node != NULL;) {
		appendEntNode(gf->object_list, node->object);
		removeEntNode(working_gf->object_list, node);
		node = prev_node;
		prev_node = node->prev;
	}
	loadGameField(gf);
}

void checkPlayer(int tick) {
	if (main_player->health <= 0) {
		//setlinestyle(PS_SOLID, 4, NULL, 0);
		settextstyle(50, 20, L"");
		settextcolor(RED);
		//initgraph(400, 400);
		//registerEntity(300, 300, 60, "GameOver", star_png, NULL, NULL, "");
		outtextxy(250, 300, L"GameOver");
		FlushBatchDraw();
		Sleep(3000);
		//closegraph();
		exit(0);
	}
}

//				Items usages:

Item* increase_player_hp(Item* self, int x, int y) {
	if (self->type == HP) main_player->increase_health(self->health_points);
	return NULL;
}

Item* increase_player_fp(Item* self, int x, int y) {
	if (self->type == FOOD) main_player->increase_fullness(self->hunger_points);
	return NULL;
}

//				Collision actions:

const char* Stop_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
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

const char* Plant_Grow_phase2_Action(Entity* self, int tick) {
	int old_height = self->figure->height;
	Plant* plant = (Plant*)(self->child);
	if ((tick - plant->start_grow) % plant->time_phase_2 == 0) {
		setFigure(self, self->figure->height, plant->phase2_figure);
		plant->phase++;
		self->loop_action = NULL;
	}

	return "Grow process";
}

const char* Plant_Grow_pahse1_Action(Entity* self, int tick) {
	int old_height = self->figure->height;
	Plant* plant = (Plant*)(self->child);
	if ((tick - plant->start_grow) % plant->time_phase_1 == 0) {
		setFigure(self, self->figure->height, plant->phase1_figure);
		plant->phase++;
		self->loop_action = Plant_Grow_phase2_Action;
	}
	
	return "Grow process";
}


const char* Push_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	if (!obstacle->movable) { 
		return Stop_Action(self, obstacle, dx, dy, side);
	}
	if (*dx && !move_with_collision(obstacle, *dx, *dy)) {
		*dx = 0;
	}
	else if (*dy && !move_with_collision(obstacle, *dx, *dy)) {
		*dy = 0;
	}
	return "Push";
}

const char* Player_Movement_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	if (obstacle == NULL) {
		workingGameField;
		switch (side) {
		case UP:
			loadGameField_with_items(getWorkingField()->upper);
			setPosition(player, player->center_x, SCREEN_HEIGHT - player->center_y);
			return "Load GF";
		case DOWN:
			loadGameField_with_items(getWorkingField()->lower);
			setPosition(player, player->center_x, SCREEN_HEIGHT - player->center_y);
			return "Load GF";
		case LEFT:
			loadGameField_with_items(getWorkingField()->left);
			setPosition(player, SCREEN_WIDTH - player->center_x, player->center_y);
			return "Load GF";
		case RIGHT:
			loadGameField_with_items(getWorkingField()->right);
			setPosition(player, SCREEN_WIDTH - player->center_x, player->center_y);
			return "Load GF";
		}
	}
	else if (obstacle->name == "Item") {
		collect_item((Item*)(obstacle->child));
		return "Collect";
	}
	return Push_Action(self, obstacle, dx, dy, side);
}

const char* Player_Loop_Action(Entity* self, int tick) {
	if (self->distance_to_target >= main_player->dstep) {
		Move_to_Target_Action(self, tick);
	}
	
	return Reduce_Fullness_Action(self, tick);
}

const char* Reduce_Fullness_Action(Entity* self, int tick) {
	if ((*(Player*)(self->child)).fullness) {
		if (tick % PLAYER_STARVATION_SPEED == 0) (*(Player*)(self->child)).fullness--;
	}
	else
		if (tick % PLAYER_STARVATION_SPEED == 0) (*(Player*)(self->child)).health--;

	return "Hunger";
}