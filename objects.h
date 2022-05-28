#include "Project_header.h"

unsigned tick = 0;

typedef struct {
	Entity* parent;
	int time_phase_1;
	int time_phase_2;
	int start_grow;
} Plant;

typedef struct {
	Entity* parent;
	int fullness;
	int health;
} Player;

const char* Plant_Grow_Action(Entity* self, int tick);
const char* Reduce_Fullness_Action(Entity* self, int tick);
const char* Push_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);

Plant* init_plant(int x, int y, int time) {
	static Plant* self;
	self = (Plant*)malloc(sizeof(Plant));
	self->parent = registerEntity(x, y, 70, "Plant", plant_png, NULL, NULL, "");
	self->parent->child = (void*)self;
	self->parent->loop_action = Plant_Grow_Action;
	self->time_phase_1 = time;
	self->time_phase_2 = time * 2;
	self->start_grow = tick;
	return self;
}

Player* init_player(int x, int y) {
	static Player* self;
	self = (Player*)malloc(sizeof(Player));
	int points[4][19][4] = {
		{{x, y, x + 50, y} },
		{{x, y + 50, x + 50, y + 50}},
		{{x, y, x, y + 50}},
		{{x + 50, y, x + 50, y + 50}}
	};
	self->parent = createByPoints(x, y, 50, "Player", player_png, Reduce_Fullness_Action, Push_Action, points);
	player = self->parent;
	self->parent->child = (void*)self;
	self->fullness = 199;
	self->health = 100;
	return self;
}

void showPlayerInfo() {
	setlinestyle(PS_SOLID, 2, NULL, 0);
	outtextxy(10, 10, L"HP");
	setfillcolor(RED);
	fillrectangle(40, 10, 40 + (*(Player*)player->child).health, 20);
	outtextxy(10, 30, L"FP");
	setfillcolor(BROWN);
	fillrectangle(40, 30, 40 + (*(Player*)player->child).fullness, 40);
}