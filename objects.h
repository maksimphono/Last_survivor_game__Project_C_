#include "Project_header.h"
#define PLAYER_STEP 5
#define PLAYER_SPEED 2
#define PLANT_HP 100
#define PLAYER_STARVATION_SPEED 100

const int item_icon_size = 30;
const int default_attack_distance = 40;
int tick = 0;

const int spawn_log_item_id = 4;
const int spawn_row_meat_item_id = 5;

void spawnById(int id, int x, int y);

typedef enum {_ITEM, FOOD, HP, WEAPON, GUN}ItemType;

typedef struct Item;

struct Item {
	Entity* parent;
	ItemType type;
	union {
		int hunger_points; // if it is food
		int health_points; // if it is for HP
		struct {
			int damage; // if it is a weapon or a gun
			int distance;
			int reload_time;
			int reload;
			int track[4];
		};
	};
	Item* (*use)(Item* self, int x, int y);
};

typedef struct {
	Entity* parent;
	int phase;
	int time_phase_1;
	int time_phase_2;
	int health;
	LPCTSTR phase1_figure;
	LPCTSTR phase2_figure;
	int start_grow;
	int drop_items_id[3];
	void (*death_action)(int* id_arr, int , int);
} Plant;

typedef struct {
	int speed;
	int damage;
	int dstep;
	Entity* parent;
} Bullet;

typedef struct Mob {
	Entity* parent;
	int damage;
	int health;
	int reload_time;
	int reload;
	int dstep;
	void (*attack)(Mob* self, Entity* target);
	void (*death_action)(int* id_arr, int, int);
	LPCTSTR move_figure;
	LPCTSTR stand_figure;
	LPCTSTR attack_figure;
	int drop_items_id[3];
} Mob;

typedef struct MobStar {
	const char* name;
	const int damage;
	const int speed;
	const int dstep;
	const int health;
	const int figure_height;
	LPCTSTR attack_figure;
	LPCTSTR walk_figure;
	LPCTSTR stand_figure;
} MobStar;

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
MobStar mobStar = {
	"Star", 30, 10, 2, 50, 50, mob_star_attack_png, mob_star_walk_png, mob_star_stand_png
};

const char* Plant_Grow_pahse1_Action(Entity* self, int tick);
const char* Reduce_Fullness_Action(Entity* self, int tick);
const char* Push_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);
const char* Stop_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);
const char* Player_Movement_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);
const char* Reduce_HP_Bullet_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);
const char* Attack_by_hit_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side);
const char* Player_Loop_Action(Entity* self, int tick);
const char* Move_to_Target_Action(Entity* self, int tick);
const char* Move_by_line_Action(Entity* self, int tick);
const char* Reload_Gun_Action(Entity* self, int tick);
const char* Mob_Loop_Action_1(Entity* self, int tick);
const char* Mob_check_self_Action(Entity* self, int tick);
void drop_items_death_action(int* id_arr, int, int);

Item* increase_player_hp(Item* self, int x, int y);
Item* increase_player_fp(Item* self, int x, int y);
Item* shoot_gun(Item* gun, int x, int y);
Item* shoot_rifle(Item* gun, int x, int y);
Item* hit_with_tool(Item* tool, int x, int y);

COLORREF _RGB(int lst[3]) {
	return RGB(lst[0], lst[1], lst[2]);
}

Item* init_item(int x, int y, LPCTSTR path, ItemType type, int points);

Plant* init_plant(int x, int y, int time, int height, LPCTSTR figure0_path, LPCTSTR figure1_path, LPCTSTR figure2_path, int collision_width, int collision_height) {
	static Plant* self;
	self = (Plant*)malloc(sizeof(Plant));
	self->parent = registerEntity(x, y, height, "Plant", figure0_path, NULL, NULL, "Prop:", "Rect", x + (int)(height * 0.4), y + (int)(height * 0.8), collision_width, collision_height);
	self->parent->child = (void*)self;
	self->parent->loop_action = Plant_Grow_pahse1_Action;
	self->parent->movable = false;
	self->time_phase_1 = time;
	self->time_phase_2 = time * 2;
	self->phase1_figure = figure1_path;
	self->phase2_figure = figure2_path;
	self->start_grow = tick;
	self->health = PLANT_HP;
	memset(self->drop_items_id, -1, sizeof(int) * 3);
	self->death_action = drop_items_death_action;
	return self;
}

void spawn_oak_tree_3(int x, int y) {
	Plant* tree = init_plant(x, y, 900, 100, oaktree4_1_png, oaktree4_2_png, oaktree4_3_png, 20, 10);
}

void spawn_oak_tree_2(int x, int y) {
	Plant* tree = init_plant(x, y, 1000, 100, oaktree3_1_png, oaktree3_2_png, oaktree3_3_png, 20, 10);
}

void spawn_oak_pinetree(int x, int y) {
	Plant* tree = init_plant(x, y, 1200, 125, pinetree_1_png, pinetree_2_png, pinetree_3_png, 20, 10);
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
	self->parent = registerEntity(x, y, 50, "Player", player_png, Player_Loop_Action, Player_Movement_Action, "Prop:", "Square", x + 10, y + 10, 30);
	player = self->parent;
	self->parent->child = (void*)self;
	self->fullness = 199;
	self->health = 100;
	self->dstep = PLAYER_STEP;
	self->hvstep = 2;
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
		self->use = hit_with_tool;
		break;
	case GUN:
		self->damage = points;
		self->parent->loop_action = Reload_Gun_Action;
		memset(self->track, 0, sizeof(int) * 4);
		break;
	case HP:
		self->health_points = points;
		self->use = increase_player_hp;
		break;
	default :
		self->use = NULL;
	}
	return self;
}

void spawn_item_apple(int x, int y) { Item* self = (Item*)malloc(sizeof(Item)); self = init_item(x, y, apple_png, FOOD, 25);}

void spawn_item_banana(int x, int y) { Item* self = (Item*)malloc(sizeof(Item)); self = init_item(x, y, banana_png, FOOD, 30); }

void spawn_item_aidkit(int x, int y) { Item* self = (Item*)malloc(sizeof(Item)); self = init_item(x, y, aidkit_png, HP, 40); }

void spawn_item_axe(int x, int y) { Item* self = (Item*)malloc(sizeof(Item)); self = init_item(x, y, axe_png, WEAPON, 25); }

void spawn_item_pickaxe(int x, int y) { Item* self = (Item*)malloc(sizeof(Item)); self = init_item(x, y, pickaxe_png, WEAPON, 35); }

void spawn_item_sword(int x, int y) { Item* self = (Item*)malloc(sizeof(Item)); self = init_item(x, y, sword_png, WEAPON, 35); }

void spawn_item_log(int x, int y) { Item* self = (Item*)malloc(sizeof(Item)); self = init_item(x, y, log_item_png, _ITEM, 0); }

void spawn_item_row_meat(int x, int y) { Item* self = (Item*)malloc(sizeof(Item)); self = init_item(x, y, row_meat_item_png, FOOD, 40); }

void spawn_item_stone(int x, int y) { Item* self = (Item*)malloc(sizeof(Item)); self = init_item(x, y, stone_item_png, _ITEM, 0); }

void spawn_small_gun(int x, int y) {
	static Item* self;
	self = (Item*)malloc(sizeof(Item));
	self = init_item(x, y, gun_png, GUN, 20);
	self->reload_time = 30;
	self->reload = 30;
	self->use = shoot_gun;
}

void spawn_rifle(int x, int y) {
	static Item* self;
	self = (Item*)malloc(sizeof(Item));
	self = init_item(x, y, rifle_png, GUN, 40);
	self->reload_time = 40;
	self->reload = 40;
	self->use = shoot_rifle;
	self->distance = 300;
}

Bullet* init_bullet(int center_x, int center_y, int target_x, int target_y, int damage) {
	static Bullet* self;
	self = (Bullet*)malloc(sizeof(Bullet));
	self->parent = registerEntity(center_x, center_y, 30, "Bullet", bullet_png, Move_by_line_Action, Reduce_HP_Bullet_Action, "Prop:", "Square", center_x, center_y, 30);
	setPosition(self->parent, center_x, center_y);
	self->parent->child = (void*)self;
	self->dstep = 6;
	self->damage = damage;
	setTarget(self->parent, "Points", target_x, target_y);
	return self;
}

Mob* init_mob(int x, int y, int height, LPCTSTR picture, const char* (*loop_action)(Entity*, int), const char* (*collision_action)(Entity*, Entity*, int*, int*, COLLISION_SIDE)) {
	static Mob* self;
	self = (Mob*)malloc(sizeof(Mob));
	self->parent = registerEntity(x, y, height, "Mob", picture, loop_action, collision_action, "Prop:", "Square", x + 15, y + 15, height - 30);
	self->parent->child = (void*)self;
	self->reload_time = 30;
	memset(self->drop_items_id, -1, sizeof(int) * 3);
	return self;
}

void spawn_stone(int x, int y) {
	Mob* stone = init_mob(x, y, 120, stone_png, Mob_check_self_Action, NULL);
	stone->parent->movable = false;
	stone->stand_figure = stone_png;
	stone->health = 100;
	stone->drop_items_id[0] = 12;
}

void spawn_mob_Star(int x, int y) {
	static Mob* self;
	self = (Mob*)malloc(sizeof(Mob));
	self = init_mob(x, y, mobStar.figure_height, mobStar.stand_figure, Mob_Loop_Action_1, Attack_by_hit_Action);
	self->health = mobStar.health;
	self->damage = mobStar.damage;
	self->dstep = mobStar.dstep;
	self->reload_time = 30;
	self->move_figure = mobStar.walk_figure;
	self->attack_figure = mobStar.attack_figure;
	self->stand_figure = mobStar.stand_figure;
	memset(self->drop_items_id, -1, sizeof(int) * 3);
	self->drop_items_id[0] = 4;
	self->death_action = drop_items_death_action;
}

bool collect_item(Item* self) {
	Entity* parent = self->parent;
	Item** item = main_player->items;
	for (; item != main_player->items + 9; item++) {
		if (*item == NULL) {
			*item = self;
			break;
		}
	}
	if (item != main_player->items + 9) {
		parent->phis_model->nocollide = true;
		return true;
	} 
	return false;
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