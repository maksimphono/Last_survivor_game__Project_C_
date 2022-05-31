//#include "objects.h"
#include "actions.h"

void createObjects00(GameField* gf) {

	setupGameField(gf);

	int x = 100, y = 440;
	int points[4][MAX_VECTOR_NUM][4] = {
		{{x, y, x + 50, y}},
		{{x, y + 50, x + 50, y + 50}},
		{{x, y, x, y + 50}},
		{{x + 50, y, x + 50, y + 50}}
	};
	
	//Entity* box = createByPoints(x, y, 50, "clock", clock_png, NULL, Stop_Action, points);

	//Entity* box = registerEntity(x, y, 50, "clock", clock_png, NULL, NULL, "");


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

	init_item(70, 330, axe_png, WEAPON, 20);
	init_item(70, 380, apple_png, FOOD, 20);
	init_item(70, 380, aidkit_png, HP, 30);
	init_item(110, 380, banana_png, FOOD, 50);
	init_item(150, 380, icecream_png, FOOD, 20);

	init_player(200, 100);

	//player = createByPoints(x, y, 50, "main", player_png, NULL, Push_Action, _points);
}

void createObjects01(GameField* gf) {
	int cords[3][7][2] = {
		{{200, 260}, {250, 270}, {300, 200}, {130, 260}, {100, 190}, {270, 390} },
		{{400, 260}, { 450, 370 }, { 370, 470 }, { 130, 500 }, { 300, 290 }, { 520, 400 }},
		{{200, 260}, { 250, 270 }, { 300, 200 }, { 130, 260 }, { 100, 190 }, { 270, 390 }}
	};
	const LPCTSTR models[4] = { oaktree1_png , oaktree2_png , oaktree3_png , oaktree4_3_png };
	int modes_height[4] = {150, 100, 100, 100};
	setupGameField(gf);

	registerEntity(500, 500, 30, "grass", smallgrass_png, NULL, NULL, "");
	registerEntity(60, 350, 30, "grass", smallgrass_png, NULL, NULL, "");
	registerEntity(560, 430, 30, "grass", smallgrass_png, NULL, NULL, "");
	registerEntity(300, 260, 30, "grass", smallgrass_png, NULL, NULL, "");

	init_plant(60, 70, 900, 101, oaktree4_1_png, oaktree4_2_png, oaktree4_3_png);

	for (int i = 0; i < 6; i++) {
		registerEntity(cords[0][i][_X], cords[0][i][_Y], 100, "tree", oaktree3_png, NULL, NULL, "static Prop:", "Rect", cords[0][i][_X] + 40, cords[0][i][_Y] + 70, 20, 10);
	}
	for (int i = 0; i < 6; i++) {
		registerEntity(cords[1][i][_X], cords[1][i][_Y], 100, "tree", oaktree4_3_png, NULL, NULL, "static Prop:", "Rect", cords[1][i][_X] + 40, cords[1][i][_Y] + 70, 20, 10);
	}

}