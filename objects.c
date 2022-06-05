//#include "objects.h"
#include "actions.h"
#define MAX_SPAWNER_NUMBER 29

void (*spawners[19])(int, int) = {
	spawn_mob_Star,
	spawn_oak_tree_3,
	spawn_oak_tree_2,
	spawn_item_log,
	spawn_item_row_meat,
	spawn_item_apple,
	spawn_small_gun,
	spawn_rifle,
	spawn_item_aidkit,
	spawn_item_axe,
	spawn_item_sword,
	spawn_item_banana,
	spawn_item_stone,
	spawn_item_pickaxe
};

void spawnById(int id, int x, int y) {
	if (id < 0 || id > MAX_SPAWNER_NUMBER) return;
	spawners[id](x, y);
}

void createObjects00(GameField* gf) {

	setupGameField(gf);

	int x = 100, y = 440;
	int points[4][MAX_VECTOR_NUM][4] = {
		{{x, y, x + 50, y}},
		{{x, y + 50, x + 50, y + 50}},
		{{x, y, x, y + 50}},
		{{x + 50, y, x + 50, y + 50}}
	};

	x = 250, y = 440;
	int _points[4][MAX_VECTOR_NUM][4] = {
		{{x, y, x + 50, y}},
		{{x, y + 50, x + 50, y + 50}},
		{{x, y, x, y + 50}},
		{{x + 50, y, x + 50, y + 50}}
	};
	
	x = 250, y = 200;
	registerEntity(0, 480, 120, "Sea", sea_png, NULL, NULL, "Prop:", "Rect", 0, 480, 600, 120);

	Entity* boat = registerEntity(x, y, 78, "Boat", brokenboat_png, NULL, NULL, "Prop:", "Rect", x, y + 40, 100, 30);
	boat->movable = false;

	registerEntity(50, 70, 19, "Seashell", seashell1_png, NULL, NULL, "");
	registerEntity(500, 440, 19, "Seashell", seashell1_png, NULL, NULL, "");

	registerEntity(150, 200, 70, "Plant", plant_png, NULL, NULL, "");
	registerEntity(210, 230, 70, "Plant", plant_png, NULL, NULL, "");
	registerEntity(350, 370, 70, "Plant", plant_png, NULL, NULL, "");


	init_item(70, 110, magicstone1_png, _ITEM, 0);

	init_item(70, 170, magicstone1_png, _ITEM, 0);

	init_item(70, 230, magicstone1_png, _ITEM, 0);

	spawn_item_axe(70, 330);
	spawn_item_sword(110, 330);
	spawn_rifle(150, 330);
	spawn_small_gun(310, 140);
	spawn_item_apple(70, 380);
	spawn_item_aidkit(70, 380);
	spawn_item_banana(110, 380);

	spawn_stone(350, 330);

	spawn_mob_Star(330, 130);

	init_player(200, 100);

	//player = createByPoints(x, y, 50, "main", player_png, NULL, Push_Action, _points);
}

void createObjects01(GameField* gf) {
	int cords[3][7][2] = {
		{{200, 260}, {250, 270}, {300, 200}, {130, 260}, {100, 190}, {270, 390} },
		{{400, 260}, { 450, 370 }, { 370, 470 }, { 130, 500 }, { 300, 290 }, { 520, 400 }},
		{{200, 260}, { 250, 270 }, { 300, 200 }, { 130, 260 }, { 100, 190 }, { 270, 390 }}
	};
	int modes_height[4] = {150, 100, 100, 100};
	setupGameField(gf);

	registerEntity(500, 500, 30, "grass", smallgrass_png, NULL, NULL, "");
	registerEntity(60, 350, 30, "grass", smallgrass_png, NULL, NULL, "");
	registerEntity(560, 430, 30, "grass", smallgrass_png, NULL, NULL, "");
	registerEntity(300, 260, 30, "grass", smallgrass_png, NULL, NULL, "");

	//init_plant(60, 70, 900, 101, oaktree4_1_png, oaktree4_2_png, oaktree4_3_png, 30, 20);

	spawn_oak_pinetree(60, 70);

	for (int i = 0; i < 6; i++) {
		//registerEntity(cords[0][i][_X], cords[0][i][_Y], 100, "tree", oaktree3_png, NULL, NULL, "static Prop:", "Rect", cords[0][i][_X] + 40, cords[0][i][_Y] + 70, 20, 10);
		spawnById(2, cords[0][i][_X], cords[0][i][_Y]);
	}

}