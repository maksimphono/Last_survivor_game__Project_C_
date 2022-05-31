#include "Project_header.h"
#define PLAYER_STEP 5
#define PLAYER_SPEED 3
#define PLANT_HP 100
#define PLAYER_STARVATION_SPEED 100

const int item_icon_size = 30;
const int default_attack_distance = 40;
int tick = 0;

typedef struct {
	Entity* parent;
	int phase;
	int time_phase_1;
	int time_phase_2;
	int health;
	LPCTSTR phase1_figure;
	LPCTSTR phase2_figure;
	int start_grow;
} Plant;

typedef enum {_ITEM, FOOD, HP, WEAPON, GUN}ItemType;

typedef struct Item;

struct Item {
	Entity* parent;
	ItemType type;
	union {
		int hunger_points; // if it is food
		int health_points; // if it is for HP
		struct {
			int damage; // if it is a weapon
			int distance;
		};
	};
	Item* (*use)(Item* self, int x, int y);
};

typedef struct {
	Entity* parent;
	Item* items[9];
	void (*increase_health)(int hp);
	void (*increase_fullness)(int fp);
	int item_in_hand;
	int fullness;
	int health;
	int hvstep;
	int dstep;
} Player;

Player* main_player;

const char* Plant_Grow_pahse1_Action(Entity* self, int tick);
const char* Reduce_Fullness_Action(Entity* self, int tick);
const char* Push_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);
const char* Stop_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);
const char* Player_Movement_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);
const char* Player_Loop_Action(Entity* self, int tick);
Item* increase_player_hp(Item* self, int x, int y);
Item* increase_player_fp(Item* self, int x, int y);

COLORREF _RGB(int lst[3]) {
	return RGB(lst[0], lst[1], lst[2]);
}

Plant* init_plant(int x, int y, int time, int height, LPCTSTR figure0_path, LPCTSTR figure1_path, LPCTSTR figure2_path) {
	static Plant* self;
	self = (Plant*)malloc(sizeof(Plant));
	self->parent = registerEntity(x, y, height, "Plant", figure0_path, NULL, NULL, "Prop:", "Rect", x + 40, y + 70, 20, 10);
	self->parent->child = (void*)self;
	self->parent->loop_action = Plant_Grow_pahse1_Action;
	self->parent->movable = false;
	self->time_phase_1 = time;
	self->time_phase_2 = time * 2;
	self->phase1_figure = figure1_path;
	self->phase2_figure = figure2_path;
	self->start_grow = tick;
	self->health = PLANT_HP;
	return self;
}

inline void increase_health(int hp) { main_player->health = min(main_player->health + hp, 100); }
inline void increase_fullness(int p) { main_player->fullness = min(main_player->fullness + p, 199); }

Player* init_player(int x, int y) {
	static Player* self;
	self = (Player*)malloc(sizeof(Player));
	int points[4][19][4] = {
		{{x, y, x + 50, y} },
		{{x, y + 50, x + 50, y + 50}},
		{{x, y, x, y + 50}},
		{{x + 50, y, x + 50, y + 50}}
	};
	self->parent = createByPoints(x, y, 50, "Player", player_png, Player_Loop_Action, Player_Movement_Action, points);
	player = self->parent;
	self->parent->child = (void*)self;
	self->fullness = 199;
	self->health = 100;
	self->dstep = PLAYER_STEP;
	self->hvstep = 1;
	memset(self->items, NULL, 9 * sizeof(Item*));
	self->item_in_hand = 1;
	main_player = self;
	self->increase_fullness = increase_fullness;
	self->increase_health = increase_health;
	return self;
}

Item* init_item(int x, int y, LPCTSTR path, ItemType type, int points) {
	static Item* self;
	workingGameField;
	self = (Item*)malloc(sizeof(Item));
	self->parent = registerEntity(x, y, item_icon_size, "Item", path, NULL, Stop_Action, "Prop:", "Square", x, y, item_icon_size);
	self->parent->child = (void*)self;
	self->type = type;
	switch (type) {
	case FOOD:
		self->hunger_points = points;
		self->use = increase_player_fp;
		break;
	case WEAPON:
		self->damage = points;
		self->distance = default_attack_distance;
		break;
	case GUN:
		self->damage = points;
		break;
	case HP:
		self->health_points = points;
		self->use = increase_player_hp;
		break;
	}

	return self;
}

void collect_item(Item* self) {
	Entity* parent = self->parent;
	setPosition(parent, 50, 560);
	parent->phis_model->nocollide = true;
	for (Item** item = main_player->items; item != main_player->items + 9; item++) {
		if (*item == NULL) {
			*item = self;
			break;
		}
	}
}

void drop_item_from_hand() {
	if (main_player->items[main_player->item_in_hand - 1] == NULL) return;
	main_player->items[main_player->item_in_hand - 1]->parent->phis_model->nocollide = false;
	setPosition(main_player->items[main_player->item_in_hand - 1]->parent, main_player->parent->center_x, main_player->parent->center_y + 50);
	main_player->items[main_player->item_in_hand - 1]->parent->lower_edge = main_player->items[main_player->item_in_hand - 1]->parent->Y + main_player->items[main_player->item_in_hand - 1]->parent->figure->height;
	main_player->items[main_player->item_in_hand - 1] = NULL;
}

void showPlayerInventory() {
	static int X = 50, Y = 560, rgb_frame[3] = {95, 95, 95}, rgb_selected[3] = { 15, 15, 15 };
	setlinecolor(_RGB(rgb_frame));

	for (int i = 0; i < 9; i++) {
		if (main_player->items[i] != NULL) {
			setPosition(main_player->items[i]->parent, X + i * item_icon_size + item_icon_size / 2, Y + item_icon_size / 2);
			main_player->items[i]->parent->lower_edge = SCREEN_HEIGHT + 100;
		}
	}
	rectangle(X, Y, X + item_icon_size * 9, Y + item_icon_size);
	setlinestyle(PS_SOLID, 4, NULL, 0);
	setlinecolor(_RGB(rgb_selected));
	rectangle(X + item_icon_size * (main_player->item_in_hand - 1), Y, X + item_icon_size * (main_player->item_in_hand - 1) + item_icon_size, Y + item_icon_size);
}

void showPlayerInfo() {
	static int X = 10, Y = 10;
	setlinestyle(PS_SOLID, 2, NULL, 0);
	outtextxy(X, Y, L"HP");
	setfillcolor(RED);
	fillrectangle(X + 30, Y, X + 30 + (*(Player*)player->child).health, Y + 10);
	outtextxy(X, Y + 20, L"FP");
	setfillcolor(BROWN);
	fillrectangle(X + 30, Y + 20, X + 30 + (*(Player*)player->child).fullness, Y + 30);
	showPlayerInventory();
}