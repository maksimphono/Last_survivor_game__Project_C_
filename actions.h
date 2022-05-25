// Actions must be initialized at this file

#include "Project_header.h"

const char* Stop_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	if (side == LEFT || side == RIGHT) *dx = 0;
	if (side == UP || side == DOWN) *dy = 0;
	return "Stop";
}

const char* Move_to_Target_Action(Entity* self, int tick) {
	//move_directly(self, self->target[_X], self->target[_Y], 5);
	if (!self->target[0] || !self->target[1]) return "";
	if (move_directly(self, self->target[_X], self->target[_Y], 5)) {
		self->target[_X] = 0;
		self->target[_Y] = 0;
		self->move_axis = 0;
	}
	return "Move directly";
}