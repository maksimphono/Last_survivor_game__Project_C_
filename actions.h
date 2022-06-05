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
		init_bullet(main_player->parent->center_x, main_player->parent->center_y, x, y, gun->damage);
	}
	return gun;
}

Item* shoot_rifle(Item* gun, int x, int y) {
	Entity* prey;
	Vector* attack_vector;
	COLLISION_SIDE side;
	double axis, dY, dX, step_y, step_x;
	
	if (!gun->reload) {
		gun->reload = gun->reload_time;
		axis = (double)(y - player->center_y) / (double)(x - player->center_x);
		dY = ((gun->distance) * sin(atan(axis)));
		dX = dY / axis;
		step_y = (int)round(dY);
		step_x = (int)round(dX);

		if (x < player->center_x) {	// if point lies by left and up !!! I also have to realize when point lies by left and down !!!
			step_y = -step_y;
			step_x = -step_x;
		}
		attack_vector = init_vector(player->center_x, player->center_y, player->center_x + step_x, player->center_y + step_y);
		side = (player->center_x < x) ? RIGHT : LEFT;
		prey = check_collision_line_with_all(attack_vector, player, 3, side);
		gun->track[0] = player->center_x; gun->track[1] = player->center_y; gun->track[2] = attack_vector->p2->X; gun->track[3] = attack_vector->p2->Y;
		if (prey != NULL)
			if (prey->name == "Plant") {
				(*(Plant*)(prey->child)).health -= gun->damage;
				
				return gun;
			}
			else if (prey->name == "Mob") {
				(*(Mob*)(prey->child)).health -= gun->damage;
				return gun;
			}
		side = (player->center_y < y) ? DOWN : UP;
		prey = check_collision_line_with_all(attack_vector, player, 3, side);
		if (prey == NULL) {
			return gun;
		}
		if (prey->name == "Plant") {
			(*(Plant*)(prey->child)).health -= gun->damage;
			return gun;
		}
		else if (prey->name == "Mob") {
			(*(Mob*)(prey->child)).health -= gun->damage;
			return gun;
		}
	}

	return gun;
}

Item* hit_with_tool(Item* tool, int x, int y) {
	Vector* attack_vector;
	double axis = (double)(y - player->center_y) / (double)(x - player->center_x);
	double dY = ((tool->distance - 1) * sin(atan(axis)));
	double dX = dY / axis;
	int step_y = (int)round(dY);
	int step_x = (int)round(dX);

	if (x < player->center_x) {	// if point lies by left and up !!! I also have to realize when point lies by left and down !!!
		step_y = -step_y;
		step_x = -step_x;
	}
	attack_vector = init_vector(player->center_x, player->center_y, player->center_x + step_x, player->center_y + step_y);
	COLLISION_SIDE side = (player->center_x < x)?RIGHT:LEFT;
	Entity* prey = check_collision_line_with_all(attack_vector, player, 3, side);
	//line(attack_vector->p1->X, attack_vector->p1->Y, attack_vector->p2->X, attack_vector->p2->Y);
	if (prey != NULL)
		if (prey->name == "Plant") {
			(*(Plant*)(prey->child)).health -= tool->damage;
			return tool;
		}
		else if (prey->name == "Mob") {
			(*(Mob*)(prey->child)).health -= tool->damage;
			return tool;
		}
	side = (player->center_y < y) ? DOWN : UP;
	prey = check_collision_line_with_all(attack_vector, player, 3, side);
	if (prey == NULL) return tool;
	if (prey->name == "Plant") {
		(*(Plant*)(prey->child)).health -= tool->damage;
		return tool;
	}
	else if (prey->name == "Mob") {
		(*(Mob*)(prey->child)).health -= tool->damage;
		return tool;
	}
	
	return tool;
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
	if (gun->reload > 0) {
		if (gun->track[0] != 0) {
			setlinecolor(RGB(gun->reload * 30, 0, 0));
			line(gun->track[0], gun->track[1], gun->track[2], gun->track[3]);
		}
		gun->reload--;
	}
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
	if (plant->health <= 0) {
		kill_plant(plant);
		return "Dead";
	}
	if ((tick - plant->start_grow) % plant->time_phase_2 == 0) {
		setFigure(self, self->figure->height, plant->phase2_figure);
		plant->phase++;
		plant->drop_items_id[0] = 3;
		plant->drop_items_id[1] = (rand() % 5 > 3) ? 5 : -1;
		//self->loop_action = NULL;
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
	if (plant->health <= 0) {
		kill_plant(plant);
		return "Dead";
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
	if (can_see(self, player, 200) != NULL) {
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