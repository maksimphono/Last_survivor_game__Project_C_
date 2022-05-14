#include "engine_v1.c"
#include "constants.c"
#include "graphic_types.h"

//#include <graphics.h>
#include <conio.h>
#include <time.h>
extern IMAGE main_background = *setupImage(&null_image);
extern DWORD* bg = NULL;
extern const unsigned long* bg_src = NULL;

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

void sortEnt(int main_ch_index) {
	/*
	Sorts 'entarray' (array of 'Entity') to render all entities in order
	*/
	Entity* temp = (Entity*)malloc(sizeof(Entity));
	EntityNode* lower_node;
	EntityNode* pivot;
	EntityNode* currentNode_next;
	register int steps = 0;
	for (EntityNode* currentNode = entity_list.head->next; currentNode != NULL; currentNode = currentNode_next) {
		lower_node = currentNode->prev;
		pivot = currentNode;
		while (lower_node != NULL && lower_node->prev != NULL && lower_node->object.lower_edge > pivot->object.lower_edge) lower_node = lower_node->prev;
		currentNode_next = currentNode->next;
		if (lower_node != pivot->prev) {
			removeEntNode(&entity_list, pivot);
			insertEntNode(&entity_list, lower_node, pivot);
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
		sortEnt(0); // sort the entity array, because i need first render objects, those lays higher on the screen
	}

	renderAll(render);
	switch (dx > 0) {
	case 1: // dx > 0
		for (int j = 0; j < image_height; j++) {
			for (int p = 0; p < dx; p++) {
				ull real_pixel_pos = (j + dsty) * screen_width + dstx + p;
				dst[real_pixel_pos] = bg[real_pixel_pos];
			}
		}
		break;
	case 0: // dx < 0
		for (int j = 0; j < image_height; j++) {
			for (int p = 0; p < -dx; p++) {
				ull real_pixel_pos = (j + dsty) * screen_width + dstx + image_width - p;
				dst[real_pixel_pos] = bg[real_pixel_pos];
				dst[real_pixel_pos - 1] = bg[real_pixel_pos - 1];
			}
		}
	}
	switch (dy > 0) {
	case 1: // dy > 0
		for (int i = 0; i < image_width; i++) {
			for (int p = 0; p < dy; p++) {
				ull real_pixel_pos = (dsty + p) * screen_width + dstx + i;
				dst[real_pixel_pos] = bg[real_pixel_pos];
			}
		}
		break;
	case 0: // dy < 0
		for (int i = 0; i < image_width; i++) {
			for (int p = 0; p < -dy; p++) {
				ull real_pixel_pos = (dsty + image_height - p) * screen_width + dstx + i;
				dst[real_pixel_pos] = bg[real_pixel_pos];
				real_pixel_pos = (dsty + image_height - p - 1) * screen_width + dstx + i;
				dst[real_pixel_pos] = bg[real_pixel_pos];
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
				dst[real_pixel_pos] = bg[real_pixel_pos];
			}
		}
	}
	return dst;
	//memmove(bg, dst, screen_width * SCREEN_HEIGHT * sizeof(DWORD));
}

void setPosition(GRAPHIC_TYPE type, ...) {
	/*
	Sets object (second argument) of type 'type' X coordinate as value of 'third argument', Y coordinate as value of 'fourth' argument
	*/
	va_list arguments;
	va_start(arguments, type);
	Entity* object = (Entity*)malloc(sizeof(Entity));
	switch (type) {
	case FIGURE:
		object->figure = va_arg(arguments, Figure*);
		object->figure->X = va_arg(arguments, int);
		object->figure->Y = va_arg(arguments, int);
		break;
	case ENTITY:
		object = va_arg(arguments, Entity*);
		object->figure->X = va_arg(arguments, int);
		object->figure->Y = va_arg(arguments, int);
		break;
	}
	va_end(arguments);
}

Entity* check_collision_with_all(Entity* self, int step, COLLISION_SIDE side) {
	/*
	Checks collision of self and any other object on the field
	*/
	Prop* self_model = self->phis_model;
	Prop* node_model = NULL;
	//static Entity*
	if (self_model->nocollide) return NULL;
	for (EntityNode* currentNode = entity_list.head; currentNode != NULL; currentNode = currentNode->next){
		node_model = currentNode->object.phis_model;
		if (&currentNode->object != self && node_model != NULL && collide_side(self_model, node_model, step, side)) return &currentNode->object;
	}
	return NULL;
}

void move(GRAPHIC_TYPE type, ...) {
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
	Entity* obstacle1;
	va_start(arguments, type);
	
	switch (type) {
	case FIGURE:
		figure = va_arg(arguments, Figure*);
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		move_transparent_image(dx, dy, figure, BLACK, true);
		break;
	case ENTITY:
		entity = va_arg(arguments, Entity*);
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		if (dy) side = (dy < 0) ? UP : DOWN;
		if (entity->name == "box")// && obstacle1 != NULL)
			puts("");
		obstacle1 = check_collision_with_all(entity, fabs(dy), side);
		
		if (obstacle1 != NULL){// || (dy > 0 && (obstacle1 = check_collision_with_all(entity, fabs(dy), DOWN)) != NULL)) {
			if ((unsigned long long)entity != 0xFFFFFFFFFFFFFFCF && entity->collision_action != NULL) entity->collision_action(entity, obstacle1, &dx, &dy, side);
			else dy = 0;
		}
		if (dx) side = (dx < 0) ? LEFT : RIGHT;
		obstacle1 = check_collision_with_all(entity, fabs(dy), side);
		if (obstacle1 != NULL){//(dx < 0 && (obstacle1 = check_collision_with_all(entity, fabs(dx), LEFT)) != NULL) || (dx > 0 && (obstacle1 = check_collision_with_all(entity, fabs(dx), RIGHT)) != NULL)) {
			if ((unsigned long long)entity != 0xFFFFFFFFFFFFFFCF && entity->collision_action != NULL) entity->collision_action(entity, obstacle1, &dx, &dy, side);
			else dx = 0;
		}
		move(FIGURE, entity->figure, dx, dy);
		shift(PROP, dx, dy, entity->phis_model);
		for (Figure** bone = entity->bones._; bone != entity->bones._ + entity->bones.length; bone++) {
			move(FIGURE, *bone, dx, dy);
		}
		entity->lower_edge += dy;
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
}

void show_bones(EntityNode* self, COLLISION_SIDE side) {
	/*
	Method, used to show on canvas all entity's bones (Verexes of prop)
	*/
	VectorArr* varrays[4] = {};
	Prop* model = self->object.phis_model;
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
				appendFigure(&self->object.bones, (*varray)->vectors[i].p1->X, (*varray)->vectors[i].p1->Y, BONE_MODEL_PATH);
				appendFigure(&self->object.bones, (*varray)->vectors[i].p2->X, (*varray)->vectors[i].p2->Y, BONE_MODEL_PATH);
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
		for (EntityNode* node = entity_list.head; node != NULL; node = node->next) {
			show_bones(node, NOCOLLIDE);
		}
		//for (Entity** ent = entarray; ent != entarray + entnum; ent++)
			//show_bones(*ent, side);
		visiable = true;
	}
	else { // delete all bones from 'effectarray'
		for (EntityNode* node = entity_list.head; node != NULL; node = node->next) {
			if (node->object.phis_model) clearFigureArr(&node->object.bones);
		}
		visiable = false;
	}
}

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

void all_actions(int tick) {
	for (EntityNode* node = entity_list.head; node != NULL; node = node->next) {
		if (node->object.loop_action != NULL) node->object.loop_action(&node->object, tick);
	}
}

void renderAll(bool flag) {
	/*
	Clears canvas and puts all objects on it.
	*/
	DWORD* dst;
	if (flag) {
		memmove(bg, bg_src, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
		for (EntityNode* node = entity_list.head; node != NULL; node = node->next) {
			dst = move_transparent_image(0, 0, node->object.figure, BLACK, false);
			memmove(bg, dst, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
		}
		for (EntityNode* node = entity_list.head; node != NULL; node = node->next) {
			for (Figure** bone = node->object.bones._; bone != node->object.bones._ + node->object.bones.length; bone++) {
				dst = move_transparent_image(0, 0, *bone, BLACK, false);
				memmove(bg, dst, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
			}
		}
		/*
		for (Entity** object = entarray; object != entarray + entnum; object++) {
			dst = move_transparent_image(0, 0, (*object)->figure, BLACK, false);
			memmove(bg, dst, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));

		}
		*/
		/*
		for (Entity** object = effectarray; object != effectarray + effectnum; object++) {
			dst = move_transparent_image(0, 0, (*object)->figure, BLACK, false);
			memmove(bg, dst, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
		}
		*/
	}
}