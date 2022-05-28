//#include "objects.h"
#include "actions.h"

void createObjects(GameField* gf) {

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

	
	createByPoints(x, y, 50, "Box", box3_png, NULL, Stop_Action, _points);

	Plant* pl = init_plant(240, 300, 900);

	registerEntity(500, 500, 50, "clock", clock_png, NULL, NULL, "");
	Entity* box = createByPoints(x, y, 50, "clock", clock_png, NULL, Stop_Action, _points);

	init_player(300, 300);

	//player = createByPoints(x, y, 50, "main", player_png, NULL, Push_Action, _points);
}
