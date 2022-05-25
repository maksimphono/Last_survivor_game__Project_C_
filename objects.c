#include "Project_header.h"
#include "objects.h"

void createObjects(GameField* gf) {

	setupGameField(gf);

	int x = 100, y = 440;
	int points[4][MAX_VECTOR_NUM][4] = {
		{{x, y, x + 50, y}},
		{{x, y + 50, x + 50, y + 50}},
		{{x, y, x, y + 50}},
		{{x + 50, y, x + 50, y + 50}}
	};
	
	Entity* box = createByPoints(x, y, "box", box3_png, NULL, Stop_Action, points);

	x = 250, y = 440;
	int _points[4][MAX_VECTOR_NUM][4] = {
		{{x, y, x + 50, y}},
		{{x, y + 50, x + 50, y + 50}},
		{{x, y, x, y + 50}},
		{{x + 50, y, x + 50, y + 50}}
	};

	Plant* pl = init_plant(240, 300, 900);

	player = createByPoints(x, y, "main", player_png, NULL, Stop_Action, _points);
}
