#ifndef GRAPHIC_ENGINE

#define GRAPHIC_ENGINE "graphic_engine.c"

#include "phisics_engine.c"
#include "graphic_types.h"

#include <conio.h>
#include <time.h>
extern IMAGE main_background = *setupImage(&null_image);
extern bool visiable_bones = false;

void renderAll(bool);

void sortEnt(int player_index) {
	/*
	Sorts 'entarray' (array of 'Entity') to render all entities in order
	*/
	Entity* temp = (Entity*)malloc(sizeof(Entity));
	EntityNode* lower_node;
	EntityNode* pivot;
	EntityNode* currentNode_next;
	register int steps = 0;
	for (EntityNode* currentNode = workingGameField->object_list->head->next; currentNode != NULL; currentNode = currentNode_next) {
		lower_node = currentNode->prev;
		pivot = currentNode;
		while (lower_node != NULL && lower_node->prev != NULL && lower_node->object->lower_edge > pivot->object->lower_edge) lower_node = lower_node->prev;
		currentNode_next = currentNode->next;
		if (lower_node != pivot->prev) {
			removeEntNode(workingGameField->object_list, pivot);
			insertEntNode(workingGameField->object_list, lower_node, pivot);
		}
	}
}

DWORD* move_transparent_image(int dx, int dy, Figure* fig, COLORREF color, bool render) {
	/*
	Main method to move 'Figure' by cords dx, dy. It also renders all entities in sequence. 'render' argument
	used to determine, whether all eneitities must be rendered, if 'true' then yes, must,
	used to avoid infinity recursion calls.
	*/
	static bool sorted = false;
	//if (&images[fig->img_index] == NULL) return NULL;
	DWORD* src = GetImageBuffer(&images[fig->img_index]);
	DWORD* dst = GetImageBuffer(GetWorkingImage());
	int dstx = fig->X, dsty = fig->Y;
	int image_width = images[fig->img_index].getwidth(), image_height = fig->height, screen_width;
	fig->X += dx;
	fig->Y += dy;

	if (GetWorkingImage() == NULL)
		screen_width = getwidth();
	else
		screen_width = GetWorkingImage()->getwidth();

	if (dy || dx) { // if object actually moves
		sortEnt(0); // sort the entii.ty array, because i need first render objects, those lays higher on the screen
	}

	renderAll(render);
	switch (dx > 0) {
	case 1: // dx > 0
		for (int j = 0; j < image_height * 1; j++) {
			for (int p = 0; p < dx; p++) {
				ull real_pixel_pos = (j + dsty) * screen_width + dstx + p;
				dst[real_pixel_pos] = workingGameField->bg_code[real_pixel_pos];
			}
		}
		break;
	case 0: // dx < 0
		for (int j = 0; j < image_height * 1; j++) {
			for (int p = 0; p < -dx; p++) {
				ull real_pixel_pos = (j + dsty) * screen_width + dstx + image_width - p;
				dst[real_pixel_pos] = workingGameField->bg_code[real_pixel_pos];
				dst[real_pixel_pos - 1] = workingGameField->bg_code[real_pixel_pos - 1];
			}
		}
	}
	switch (dy > 0) {
	case 1: // dy > 0
		for (int i = 0; i < image_width; i++) {
			for (int p = 0; p < dy; p++) {
				ull real_pixel_pos = (dsty + p) * screen_width + dstx + i;
				dst[real_pixel_pos] = workingGameField->bg_code[real_pixel_pos];
			}
		}
		break;
	case 0: // dy < 0
		for (int i = 0; i < image_width; i++) {
			for (int p = 0; p < -dy; p++) {
				ull real_pixel_pos = (dsty + image_height - p) * screen_width + dstx + i;
				dst[real_pixel_pos] = workingGameField->bg_code[real_pixel_pos];
				real_pixel_pos = (dsty + image_height - p - 1) * screen_width + dstx + i;
				dst[real_pixel_pos] = workingGameField->bg_code[real_pixel_pos];
			}
		}
	}
	
	for (int i = 0; i < image_width; i++) {
		for (int j = 0; j < image_height * 1; j++) {
			ull real_pixel_pos = (j + dsty + dy) * screen_width + i + dstx + dx;
			ull image_pixel_pos = (j + image_height * fig->current_cadr) * image_width + i;// * fig->current_cadr;
			if (src[image_pixel_pos] != BITWHITE) { // if the pivot pixel has not transparent color, i paint it with image's pixel
				if (dsty < 0)
					dsty = 0;
				else if (dsty + image_height >= SCREEN_WIDTH)
					dsty = SCREEN_WIDTH - image_height;
				dst[real_pixel_pos] = src[image_pixel_pos];
			}
			else { // if pivot pixel has transparent color, i paint it with background's pixel
				dst[real_pixel_pos] = workingGameField->bg_code[real_pixel_pos];
			}
		}
	}
	return dst;
	//membg, dst, screen_width * SCREEN_HEIGHT * sizeof(DWORD));
}

void setPosition(Entity* self, int x, int y) {
	/*
	Sets object (second argument) of type 'type' X coordinate as value of 'third argument', Y coordinate as value of 'fourth' argument
	*/
	if (self->phis_model == NULL) return;
	shift(PROP, x - self->phis_model->center->X, y - self->phis_model->center->Y, self->phis_model);
	self->X = x - self->figure->width / 2;
	self->Y = y - self->figure->height / 2;
	self->center_x = x;
	self->center_y = y;
	self->figure->X = x - self->figure->width / 2;
	self->figure->Y = y - self->figure->height / 2;
	self->lower_edge = self->Y + self->figure->height;
}
bool check_collision_with_all(Entity* obstacles[MAX_OBSTACLE_NUMBER], Entity* self, int step, COLLISION_SIDE side, PHISICS_TYPE type = PROP) {
	/*
	Checks collision of self and any other object on the field
	*/
	Prop* self_model = self->phis_model;
	Prop* node_model = NULL;
	bool correct_direction = true;
	int obstacle_num = 0;
	if (self->phis_model == NULL) return false;
	if (self_model->nocollide) return false;
	for (EntityNode* currentNode = workingGameField->object_list->head; currentNode != NULL; currentNode = currentNode->next) {
		node_model = currentNode->object->phis_model;
		if (currentNode->object != self && node_model != NULL && !node_model->nocollide && correct_direction &&
			prop_centers_distance(node_model, self_model) <= self->vision_radius) {
			if (collide_side(self_model, node_model, step, side)) {
				obstacles[obstacle_num++] = currentNode->object;
			}
		}
	}
	return obstacles == NULL;
}

bool check_collision_line_with_all(Vector* line, Entity* object, int step, COLLISION_SIDE side) {
	Prop* node_model = NULL;
	if (object->phis_model == NULL) return false;
	//if (object->phis_model->nocollide) return false;
	for (EntityNode* currentNode = workingGameField->object_list->head; currentNode != NULL; currentNode = currentNode->next) {
		node_model = currentNode->object->phis_model;
		if (currentNode->object != object && node_model != NULL && !node_model->nocollide &&
			distance_to_vector(node_model->center, line) <= object->vision_radius) {
			if (collide_vector_side(line, node_model, step, side)) {
				return true;
			}
		}
	}
	return false;
}

bool can_see(Entity* self, Entity* object) {
	COLLISION_SIDE side;
	Entity** obstacles = (Entity**)malloc(MAX_OBSTACLE_NUMBER * sizeof(Entity*));
	int points[19][4] = { {self->center_x, self->center_y, object->center_x, object->center_y} };
	//Prop ray = *init_prop(BONES, init_vectorarr(points), init_vectorarr(points), init_vectorarr(points), init_vectorarr(points));
	Vector* line = init_vector(self->center_x, self->center_y, object->center_x, object->center_y);

	if (line->length >= self->vision_radius) return false;

	if (object->center_x <= self->center_x) {
		side = LEFT;
	}
	else {
		side = RIGHT;
	}
	if (check_collision_line_with_all(line, object, 1, side)) return false;
	if (object->center_y <= self->center_y) {
		side = UP;
	}
	else {
		side = DOWN;
	}
	return !check_collision_line_with_all(line, object, 1, side);
}

bool setPhisModel(Entity* self, int points[4][MAX_VECTOR_NUM][4]) {
	setSideCollision(self->phis_model, UP, points[0]);
	setSideCollision(self->phis_model, DOWN, points[1]);
	setSideCollision(self->phis_model, LEFT, points[2]);
	setSideCollision(self->phis_model, RIGHT, points[3]);
	return true;
}

void setFigure(Entity* self, int height, LPCTSTR picture) {
	if (self->figure != NULL) kill_figure(self->figure);
	self->figure = init_figure(self->X, self->Y, height, picture);
	self->lower_edge = self->Y + self->figure->height;
}

bool connectEntity(Entity* self, Entity* object) {
	self->connected[0] = object;
	return true;
}

bool move(GRAPHIC_TYPE type, ...) {
	/*
	Shifts object (second argument) of type 'type' by X axis by value 'third argument', by Y axis by 'fourth' argument
	Also checks object's collision
	*/
	va_list arguments;
	int dx;
	int dy;
	COLLISION_SIDE side = NOCOLLIDE;
	union {
		Figure* figure = (Figure*)malloc(sizeof(Figure));
		EntityNode* entNode;
		struct {
			EntityList* entList;
			EntityNode* next_node;
		};
		Entity* entity;
	};
	Entity** obstacles;
	va_start(arguments, type);
	
	switch (type) {
	case FIGURE:
		figure = va_arg(arguments, Figure*);
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		move_transparent_image(dx, dy, figure, BLACK, true);
		break;
	case ENTITY: // if object is 'Entity', i will check collision with all objects
		entity = va_arg(arguments, Entity*);
		if (!entity->movable || (unsigned long long)entity == 0xFFFFFFFFFFFFFFCF || entity == NULL) return false;
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		
		obstacles = (Entity**)calloc(MAX_OBSTACLE_NUMBER, sizeof(Entity*));
		
		if (dy) {
			side = (dy < 0) ? UP : DOWN;

			if (entity->Y + entity->figure->height + dy >= SCREEN_HEIGHT || entity->Y + dy <= 0) { // <-- complete
				if (entity->collision_action != NULL) entity->collision_action(entity, NULL, &dx, &dy, side);
				return false;
			}
		}

		check_collision_with_all(obstacles, entity, fabs(dy), side);

		for (int i = 0; obstacles[i] != NULL; i++) {
			if (entity->collision_action != NULL) entity->collision_action(entity, obstacles[i], &dx, &dy, side);
			//else dy = 0;
		}
		memset(obstacles, NULL, MAX_OBSTACLE_NUMBER);
		if (dx) {
			side = (dx < 0) ? LEFT : RIGHT;
		
			if (entity->X + entity->figure->width + dx >= SCREEN_WIDTH || entity->X + dx <= 0) {
				if (entity->collision_action != NULL) entity->collision_action(entity, NULL, &dx, &dy, side);
				return false;
			}

		}
		
		check_collision_with_all(obstacles, entity, fabs(dx), side);
		if (entity->name == "Box" && obstacles[0] != NULL && obstacles[0]->name == "Box")
			puts("");

		for (int i = 0; obstacles[i] != NULL; i++) {
			if (entity->collision_action != NULL) entity->collision_action(entity, obstacles[i], &dx, &dy, side);
			//else dx = 0;
		}
		//free(obstacles);
		move(FIGURE, entity->figure, dx, dy);
		shift(PROP, dx, dy, entity->phis_model);
		
		entity->lower_edge += dy;
		entity->center_x += dx;
		entity->center_y += dy;
		entity->X += dx;
		entity->Y += dy;
		break;
	case ENTLIST:
		entList = va_arg(arguments, EntityList*);
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		for (EntityNode* node = entList->head; node != NULL; node = next_node) {
			next_node = node->next;
			move(ENTITY, node, dx, dy);
		}
		break;
	}
	va_end(arguments);
	return (dx || dy);
}

double entity_distance(Entity* self, Entity* object) {
	return sqrt(pow(self->center_x - object->center_x, 2) + pow(self->center_y - object->center_y, 2));
}

bool move_with_collision(Entity* self, int dx, int dy) {
	return move(ENTITY, self, dx, dy);
}

void move_by_line(Entity* self, int X, int Y, int max_step_length){
	 
	double dY = ((max_step_length - 1) * sin(atan(self->move_axis)));
	double dX = dY / self->move_axis;
	int step_y = (int)round(dY);
	int step_x = (int)round(dX);
	
	if (X < self->center_x){	// if point lies by left and up !!! I also have to realize when point lies by left and down !!!
		step_y = -step_y;
		step_x = -step_x;
	}

	self->target[_X] += step_x;
	self->target[_Y] += step_y;

	move(ENTITY, self, step_x, step_y);;
}

bool move_directly(Entity* self, int X, int Y, int max_step_length) {
	if (self->distance_to_target <= max_step_length) 
		return true;
	double dY = ((max_step_length - 1) * sin(atan(self->move_axis)));
	double dX = dY / self->move_axis;
	int step_y = (int)round(dY);
	int step_x = (int)round(dX);

	if (X < self->center_x){	// if point lies by left and up !!! I also have to realize when point lies by left and down !!!
		step_y = -step_y;
		step_x = -step_x;	
	}
	move(ENTITY, self, step_x, step_y);
	self->distance_to_target -= sqrt(pow(step_y, 2) + pow(step_x, 2));
	return false;
}

bool follow_target(Entity* self, Entity* target, int step) {
	setTarget(self, "Object", target);
	return move_directly(self, self->target[_X], self->target[_Y], step);
}

void all_actions(int tick) {
	for (EntityNode* node = workingGameField->object_list->head; node != NULL; node = node->next) {
		if (node->object->loop_action != NULL) node->object->loop_action(node->object, tick);
	}
}

void renderBones(Entity* self) {
	if (!visiable_bones || self->phis_model == NULL) return;
	VectorArr v_arrs[4] = {self->phis_model->upper, self->phis_model->lower, self->phis_model->left, self->phis_model->right };
	switch (self->phis_model->collision_type) {
	case BONES:
		for (VectorArr* arr = v_arrs; arr != v_arrs + 4; arr++) {
			for (Vector* vector = arr->vectors; vector != arr->vectors + arr->length; vector++) {
				setlinestyle(PS_SOLID, 4, NULL, 0);
				setlinecolor(RED);
				line(vector->p1->X, vector->p1->Y, vector->p2->X, vector->p2->Y);
			}
		}
		break;
	case RADIUS:
		setlinestyle(PS_SOLID, 4, NULL, 0);
		setlinecolor(RED);
		circle(self->phis_model->center->X, self->phis_model->center->Y, self->phis_model->collide_radius);
		break;
	}
	setlinecolor(GREEN);
	circle(self->center_x, self->center_y, self->vision_radius);
	circle(self->center_x, self->center_y, 2);
	setlinecolor(BLUE);
	circle(self->center_x, self->lower_edge, 2);
}

void renderBG() {
	move_transparent_image(0, 0, getWorkingField()->background_source, BLACK, false);
}

void nextCadrAll(int tick) {
	for (EntityNode* node = workingGameField->object_list->head; node != NULL; node = node->next) {
		if (tick % node->object->figure->speed == 0) node->object->figure->current_cadr = next_cadr(node->object->figure);
	}
}

void renderList(EntityList* list) {
	DWORD* dst;
	VectorArr** vector_arrs;
	memmove(workingGameField->bg_code, workingGameField->bg_code_src, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
	for (EntityNode* node = workingGameField->object_list->head; node != NULL; node = node->next) {
		dst = move_transparent_image(0, 0, node->object->figure, BLACK, false);
		memmove(workingGameField->bg_code, dst, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
	}
}

void renderAll(bool flag) {
	/*
	Clears canvas and puts all objects on it.
	*/
	DWORD* dst;
	VectorArr** vector_arrs;
	if (flag) {
		memmove(workingGameField->bg_code, workingGameField->bg_code_src, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
		for (EntityNode* node = workingGameField->object_list->head; node != NULL; node = node->next) {
			dst = move_transparent_image(0, 0, node->object->figure, BLACK, false);
			memmove(workingGameField->bg_code, dst, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
		}

		if (visiable_bones) {
			for (EntityNode* node = workingGameField->object_list->head; node != NULL; node = node->next) {
				renderBones(node->object);
			}
		}
	}
}
#endif