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
		settextstyle(50, 20, L"");
		settextcolor(RED);
		outtextxy(250, 300, L"GameOver");
		FlushBatchDraw();
		Sleep(3000);
		exit(0);
	}
}

void drop_items_death_action(int* id_arr, int x, int y) {
	for (int* item_id = id_arr; *item_id != -1; item_id++) {
		spawnById(*item_id, x, y);
	}
}

void kill_mob(Mob* self) {
	drop_items_death_action(self->drop_items_id, self->parent->center_x, self->parent->center_y);
	kill_entity(self->parent);
}

void kill_plant(Plant* self) {
	drop_items_death_action(self->drop_items_id, self->parent->center_x, self->parent->center_y);
	kill_entity(self->parent);
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

Item* shoot_gun(Item* gun, int x, int y) {
	if (!gun->reload) {
		gun->reload = gun->reload_time;
		init_bullet(main_player->parent->center_x, main_player->parent->center_y, x, y);
	}
	return gun;
}

//				Collision actions:

const char* Stop_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	if (side == LEFT || side == RIGHT) *dx = 0;
	if (side == UP || side == DOWN) *dy = 0;
	return "Stop";
}

const char* Reduce_HP_Bullet_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	union {
		Plant* plant;
		Mob* mob;
	};
	Bullet* bullet = (Bullet*)(self->child);
	if (obstacle != NULL && obstacle->name == "Plant") {
		plant = (Plant*)(obstacle->child);
		plant->health -= bullet->damage;
		if (plant->health <= 0) kill_plant(plant);
	}
	else if (obstacle != NULL && obstacle->name == "Mob") {
		mob = (Mob*)(obstacle->child);
		mob->health -= bullet->damage;
	}
	kill_entity(self);
	return "Reduse_Hp";
}

const char* Reload_Gun_Action(Entity* self, int tick) {
	Item* gun = (Item*)(self->child);
	if (gun->reload > 0) gun->reload--;
	return "Reload";
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

const char* Move_to_Target_Action(Entity* self, int tick) {
	union {
		Bullet* bullet;
	};
	int dstep = 2;
	if (player == self) {
		if (tick % PLAYER_SPEED) return "";
		dstep = main_player->dstep;
	}
	else if (self->name == "Mob") dstep = (*(Mob*)self->child).dstep;
	if (!self->target[0] || !self->target[1]) return "";
	if (move_directly(self, self->target[_X], self->target[_Y], dstep)) {
		self->target[_X] = 0;
		self->target[_Y] = 0;
		self->move_axis = 0;
	}
	return "Move directly";
}

const char* Move_by_line_Action(Entity* self, int tick) {
	if (!self->target[0] && !self->target[1]) return "";
	move_by_line(self, self->target[_X], self->target[_Y], (*(Bullet*)(self->child)).dstep);
	return "Move by line";
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
		if (collect_item((Item*)(obstacle->child))) return "Collect";
	}
	return Push_Action(self, obstacle, dx, dy, side);
}

const char* Player_Loop_Action(Entity* self, int tick) {
	if (self->distance_to_target >= main_player->dstep) {
		Move_to_Target_Action(self, tick);
	}
	return Reduce_Fullness_Action(self, tick);
}

//					Mob actions:

const char* Attack_by_hit_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	Mob* self_mob = (Mob*)self->child;
	if (self_mob->reload-- > 0) {
		*dx = *dy = 0;
		return "Reload";
	}
	self_mob->reload = self_mob->reload_time;
	if (obstacle == player) {
		setFigure(self_mob->parent, self_mob->parent->figure->height, self_mob->attack_figure);
		(*(Player*)(player->child)).health -= self_mob->damage;
	}
	*dy = 0;
	*dx = 0;
	return "Attack by hit";
}

const char* Mob_Loop_Action_1(Entity* self, int tick) {//I don't know how to use the tick
	int target[2] = {};
	int step = 3;
	Mob* self_mob = (Mob*)self->child;

	if (self_mob->health <= 0) kill_mob(self_mob);
	if (can_see(self, player, 200)) {
		setTarget(self, "Points", player->center_x, player->center_y);
	}
	else {
		if (self->target[0] == 0 && self->target[1] == 0) {
			target[0] = rand() % 50 - 25;
			target[1] = rand() % 50 - 25;
			setTarget(self, "Points", self->center_x - target[0], self->center_y - target[1]);
			setFigure(self, self->figure->height, (*(Mob*)(self->child)).move_figure);
		}
	}
	Move_to_Target_Action(self, tick);
	return "Walk process";
}

const char* Reduce_Fullness_Action(Entity* self, int tick) {
	if ((*(Player*)(self->child)).fullness) {
		if (tick % PLAYER_STARVATION_SPEED == 0) (*(Player*)(self->child)).fullness--;
	}
	else
		if (tick % PLAYER_STARVATION_SPEED == 0) (*(Player*)(self->child)).health--;

	return "Hunger";
}