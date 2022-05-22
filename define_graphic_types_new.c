#include "engine_v1.c"
#include "constants.c"
#include "graphic_types.h"

//#include <graphics.h>
#include <conio.h>
#include <time.h>
extern IMAGE main_background = *setupImage(&null_image);
extern bool visiable_bones = false;

void put_transparent_picture(int dstx, int dsty, IMAGE* img, COLORREF color) { // put transparent picture
	/*
	function, that renders image, considering to it's tratnsparency. 'color' argument represents, what color must
	be transparent in that image
	*/
	DWORD* src = GetImageBuffer(img);
	DWORD* dst = GetImageBuffer(GetWorkingImage());
	int image_width = img->getwidth(), image_height = img->getheight(), screen_width;
	if (GetWorkingImage() == NULL)
		screen_width = getwidth();
	else
		screen_width = GetWorkingImage()->getwidth();
	for (int i = 0; i < image_width; i++)
		for (int j = 0; j < image_height; j++) {
			if (src[j * image_width + i] != BITWHITE) {
				ull real_pixel_pos = (j + dsty) * screen_width + i + dstx;
				ull image_pixel_pos = j * image_width + i;
				if (dsty < 0)
					dsty = 0;
				else if (dsty + image_height >= SCREEN_WIDTH)
					dsty = SCREEN_WIDTH - image_height;
				dst[real_pixel_pos] = src[image_pixel_pos];
			}
		}
}

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
	int image_width = images[fig->img_index].getwidth(), image_height = images[fig->img_index].getheight(), screen_width;
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
		for (int j = 0; j < image_height; j++) {
			for (int p = 0; p < dx; p++) {
				ull real_pixel_pos = (j + dsty) * screen_width + dstx + p;
				dst[real_pixel_pos] = workingGameField->bg_code[real_pixel_pos];
			}
		}
		break;
	case 0: // dx < 0
		for (int j = 0; j < image_height; j++) {
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
		for (int j = 0; j < image_height; j++) {
			ull real_pixel_pos = (j + dsty + dy) * screen_width + i + dstx + dx;
			ull image_pixel_pos = j * image_width + i;
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
	//memmove(bg, dst, screen_width * SCREEN_HEIGHT * sizeof(DWORD));
}

void setPosition(Entity* self, int x, int y) {
	/*
	Sets object (second argument) of type 'type' X coordinate as value of 'third argument', Y coordinate as value of 'fourth' argument
	*/
	shift(PROP, x - self->phis_model->center->X, y - self->phis_model->center->Y, self->phis_model);
	self->X = x - self->figure->width / 2;
	self->Y = y - self->figure->height / 2;
	self->center_x = x;
	self->center_y = y;
	self->figure->X = x - self->figure->width / 2;
	self->figure->Y = y - self->figure->height / 2;
	self->lower_edge = self->Y + self->figure->height;
}
bool check_collision_with_all(Entity* obstacles[MAX_OBSTACLE_NUMBER], Entity* self, int step, COLLISION_SIDE side) {
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
		/*
		switch (side) {
		case UP:
			correct_direction = (self->Y > currentNode->object->Y);
			break;
		case DOWN:
			correct_direction = (self->Y < currentNode->object->Y);
			break;
		case RIGHT:
			correct_direction = (self->X < currentNode->object->X);
			break;
		case LEFT:
			correct_direction = (self->X > currentNode->object->X);
			break;
		}
		*/
		if (currentNode->object != self && node_model != NULL && correct_direction &&
			prop_centers_distance(node_model, self_model) <= self->vision_radius) {
			if (collide_side(self_model, node_model, step, side)) {
				obstacles[obstacle_num++] = currentNode->object;
			}
		}
	}
	return obstacles == NULL;
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
		if ((unsigned long long)entity == 0xFFFFFFFFFFFFFFCF) return false;
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		
		obstacles = (Entity**)calloc(MAX_OBSTACLE_NUMBER, sizeof(Entity*));
		
		if (dy) side = (dy < 0) ? UP : DOWN;
		check_collision_with_all(obstacles, entity, fabs(dy), side);
		if (entity->name == "Box" && obstacles[0] != NULL && obstacles[0]->name == "Box")
			puts("");
		for (int i = 0; obstacles[i] != NULL; i++) {
			if (entity->collision_action != NULL) entity->collision_action(entity, obstacles[i], &dx, &dy, side);
			else dy = 0;
		}
		memset(obstacles, NULL, MAX_OBSTACLE_NUMBER);
		if (dx) side = (dx < 0) ? LEFT : RIGHT;
		check_collision_with_all(obstacles, entity, fabs(dx), side);
		if (entity->name == "Box" && obstacles[0] != NULL && obstacles[0]->name == "Box")
			puts("");

		for (int i = 0; obstacles[i] != NULL; i++) {
			if (entity->collision_action != NULL) entity->collision_action(entity, obstacles[i], &dx, &dy, side);
			else dx = 0;
		}
		//free(obstacles);
		move(FIGURE, entity->figure, dx, dy);
		shift(PROP, dx, dy, entity->phis_model);
		
		entity->lower_edge += dy;
		entity->center_x += dx;
		entity->center_y += dy;
		entity->X += dx;
		entity->Y += dy;

		//entity->lower_edge += dy;
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

void move_directly(Entity* self, int X, int Y, int max_step_length){
	if (sqrt(pow(fabs(X - self->center_x), 2) + pow(fabs(Y - self->center_y), 2)) <= max_step_length) return;
	double dX = 0, dY = 0;
	double y2 = self->center_y;
	double x2 = self->center_x;
	double a = (Y / (max(X, x2) - min(X, x2)) + y2 / (min(X, x2) - max(X, x2)));
	int step_x, step_y;
	dY = (max_step_length * sin(atan(a)));
	dX = dY / a;
	step_y = (dY < 0) ? ceil(dY) : floor(dY);
	step_x = (dX < 0) ? ceil(dX) : floor(dX);
	move(ENTITY, self, step_x, step_y);
}

bool move_by_line(Entity* self, int X, int Y, int max_step_length) {
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

void show_bones(EntityNode* self, COLLISION_SIDE side) {
	/*
	Method, used to show on canvas all entity's bones (Verexes of prop)
	*/
	VectorArr* varrays[4] = {};
	Prop* model = self->object->phis_model;
	if (model != NULL) {
		switch (side) {
		case UP:
			varrays[0] = &model->upper;
			break;
		case DOWN:
			varrays[0] = &model->lower;
			break;
		case LEFT:
			varrays[0] = &model->left;
			break;
		case RIGHT:
			varrays[0] = &model->right;
			break;
		default:
			varrays[0] = &model->upper;
			varrays[1] = &model->lower;
			varrays[2] = &model->left;
			varrays[3] = &model->right;
		}
		for (VectorArr** varray = varrays; varray != varrays + 4 && *varray != NULL; varray++)
			for (int i = 0; i < (*varray)->length; i++) {
				appendFigure(&self->object->bones, (*varray)->vectors[i].p1->X, (*varray)->vectors[i].p1->Y, BONE_MODEL_PATH);
				appendFigure(&self->object->bones, (*varray)->vectors[i].p2->X, (*varray)->vectors[i].p2->Y, BONE_MODEL_PATH);
			}
	}
}

void show_hide_all_bones(COLLISION_SIDE side) {
	/*
	This function shows / hides all the bones of all objects.
	Shows bones: creating new 'Entity' and add it to array 'effectarray'. That array's items will be rendered after actual entities
	Hides bones: deleting entities from array 'effectarray'.
	*/
	static bool visiable = false;
	if (!visiable) { // add all bones to 'effectarray'
		for (EntityNode* node = workingGameField->object_list->head; node != NULL; node = node->next) {
			show_bones(node, NOCOLLIDE);
		}
		//for (Entity** ent = entarray; ent != entarray + entnum; ent++)
			//show_bones(*ent, side);
		visiable = true;
	}
	else { // delete all bones from 'effectarray'
		for (EntityNode* node = workingGameField->object_list->head; node != NULL; node = node->next) {
			if (node->object->phis_model) clearFigureArr(&node->object->bones);
		}
		visiable = false;
	}
}

// ACTIONS:

const char* Move_Left_Action(Entity* self, int tick) {
	move(ENTITY, self, -1, 0);
	return "Move left";
}

const char* Move_Right_Action(Entity* self, int tick) {
	move(ENTITY, self, 1, 0);
	return "Move left";
}

const char* Go_Back_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	if (side == LEFT) self->loop_action = Move_Right_Action;
	else if (side == RIGHT) self->loop_action = Move_Left_Action;
	*dx = 0;
	return "Collision!";
}

const char* Push_Action(Entity* self, Entity* obstacle, int* dx, int* dy, COLLISION_SIDE side) {
	//switch (side) {
	//case UP:
	move(ENTITY, obstacle, 1 * (*dx), 1 * (*dy));
	//}
	return "Push!";
}

const char* Move_to_Target_Action(Entity* self, int tick) {
	//move_directly(self, self->target[_X], self->target[_Y], 5);
	if (!self->target[0] || !self->target[1]) return "";
	if (move_by_line(self, self->target[_X], self->target[_Y], 5)) {
		self->target[_X] = 0;
		self->target[_Y] = 0;
		self->move_axis = 0;
	}
	return "Move directly";
}

const char* Kill_Action(Entity* self, Entity* obst, int* dx, int* dy, COLLISION_SIDE side) {
	kill_entity(obst);
	return "Kill";
}


void all_actions(int tick) {
	for (EntityNode* node = workingGameField->object_list->head; node != NULL; node = node->next) {
		if (node->object->loop_action != NULL) node->object->loop_action(node->object, tick);
	}
}

void renderBones(Entity* self) {
	if (!visiable_bones || self->phis_model == NULL) return;
	VectorArr v_arrs[4] = {self->phis_model->upper, self->phis_model->lower, self->phis_model->left, self->phis_model->right };
	for (VectorArr* arr = v_arrs; arr != v_arrs + 4; arr++) {
		for (Vector* vector = arr->vectors; vector != arr->vectors + arr->length; vector++) {
			setlinestyle(PS_SOLID, 4, NULL, 0);
			setlinecolor(RED);
			line(vector->p1->X, vector->p1->Y, vector->p2->X, vector->p2->Y);
			setlinecolor(GREEN);
			circle(self->center_x, self->center_y, self->vision_radius);
			circle(self->center_x, self->center_y, 2);
			setlinecolor(BLUE);
			circle(self->center_x, self->lower_edge, 2);
		}
	}
}

void renderBG() {
	move_transparent_image(0, 0, getWorkingField()->background_source, BLACK, false);
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