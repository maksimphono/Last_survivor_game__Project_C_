#include "Project_header.h"
#define PLAYER_STEP 5
#define PLAYER_SPEED 3
#define PLAYER_STARVATION_SPEED 100
unsigned tick = 0;

typedef struct {
	Entity* parent;
	int phase;
	int time_phase_1;
	int time_phase_2;
	LPCTSTR phase1_figure;
	LPCTSTR phase2_figure;
	int start_grow;
} Plant;

typedef struct {
	Entity* parent;
	int fullness;
	int health;
	int hvstep;
	int dstep;
} Player;

typedef struct {
	Entity* parent;
};

const char* Plant_Grow_Action(Entity* self, int tick);
const char* Reduce_Fullness_Action(Entity* self, int tick);
const char* Push_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);
const char* Stop_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);

Plant* init_plant(int x, int y, int time, int height, LPCTSTR figure0_path, LPCTSTR figure1_path, LPCTSTR figure2_path) {
	static Plant* self;
	self = (Plant*)malloc(sizeof(Plant));
	self->parent = registerEntity(x, y, height, "Plant", figure0_path, NULL, NULL, "Rect", x + 20, y + 70, 20, 10);
	self->parent->child = (void*)self;
	self->parent->loop_action = Plant_Grow_Action;
	self->time_phase_1 = time;
	self->time_phase_2 = time * 2;
	self->phase1_figure = figure1_path;
	self->phase2_figure = figure2_path;
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
	self->parent = createByPoints(x, y, 50, "Player", player_png, Reduce_Fullness_Action, Stop_Action, points);
	player = self->parent;
	self->parent->child = (void*)self;
	self->fullness = 199;
	self->health = 100;
	self->dstep = PLAYER_STEP;
	self->hvstep = 1;
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