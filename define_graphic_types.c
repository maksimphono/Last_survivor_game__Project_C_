#include "engine_v1.c"
#include "constants.c"

//#include <graphics.h>
#include <conio.h>
#include <time.h>

LPCTSTR MAIN_BG_MODEL_PATH = L"assets\\main_bg_01.png";
LPCTSTR MAIN_CH_MODEL_PATH = L"assets\\test_transp.png";

typedef unsigned long long ull;
typedef enum { FIGURE, IMG, ENTITY } GRAPHIC_TYPE;
typedef enum { NONE, BONE, EFFECT, OBJECT, ITEM } ENTITY_TYPE;

extern IMAGE images[MAX_IMAGE_NUM] = {}; // array, that contains all images, that will be rendered on canvas
extern unsigned images_len = 0;
unsigned empty_image_pos[MAX_IMAGE_NUM] = {};
int empty_image_pos_len = 0;

IMAGE* setupImage(unsigned* img_index) {
	/*
	takes IMAGE object from 'images' array, return pointer to this image and write index of the image to 'img_index'
	*/
	assert(images_len < MAX_IMAGE_NUM);
	//if (images_len >= MAX_IMAGE_NUM) return NULL;
	if (empty_image_pos_len) {
		*img_index = empty_image_pos[--empty_image_pos_len];
	}
	else {
		*img_index = images_len++;
	}
	
	return &images[*img_index];
}

unsigned null_image;
extern IMAGE main_background = *setupImage(&null_image);
extern DWORD* bg = NULL;
extern const unsigned long* bg_src = NULL;

typedef struct Figure {
	/*
	This abstraction structure holds image parameters. 'img_imdex' field represens index of image in 'images' array
	*/
	int X;
	int Y;
	unsigned width;
	unsigned height;
	unsigned img_index;
} Figure;

typedef struct Entity {
	/*
	This is abstraction structure, that represents 'Entity'. It holds coordinates and figure, that figure will be rendered on canvas
	*/
	int lower_edge;
	ENTITY_TYPE type;
	Figure* figure;
	Prop* phis_model;
	int center_x;
	int center_y;
	int X;
	int Y;
} Entity;

typedef struct EntArray {
	Entity* entities;
	int length;
} EntArray;

extern Entity* entarray[MAX_ENTITY_NUM] = {}; // all 'Entity' object will be stored at this array
extern int entnum = 0;

extern Entity* effectarray[MAX_ENTITY_NUM] = {}; // all effects will be stored at this array. Effect is entity without phisics model. It's rendered after actual objects
extern int effectnum = 0;

void addEnt(Entity* ent) {
	entarray[entnum++] = ent;
}

void addEffect(Entity* ent) {
	effectarray[effectnum++] = ent;
}

Figure* init_figure(int x, int y, LPCTSTR path_to_image) {
	/*
	constructor for 'Figure' struct
	*/
	static Figure* self;
	self = (Figure*)malloc(sizeof(Figure));
	IMAGE* img = setupImage(&self->img_index); // access to image in 'images' array
	if (img != NULL) {
		loadimage(img, path_to_image, 0, 0, true);
		self->width = img->getwidth();
		self->height = img->getheight();
	}
	self->X = x;
	self->Y = y;
	
	return self;
}

void del_figure(Figure* self) {
	/*
	Destructor for 'Figure' structure
	*/
	//images[self->img_index] = NULL;
	//free(&images[self->img_index]);
	empty_image_pos[empty_image_pos_len++] = self->img_index;
	//images[self->img_index];
	//images_len--;
	free(self);
}

Entity* init_entity(int x, int y, LPCTSTR path_to_image) {
	/*
	Constructor for 'Entity' structure
	*/
	static Entity* self;
	self = (Entity*)malloc(sizeof(Entity));
	self->figure = init_figure(x, y, path_to_image);
	//self->phis_model = *init_prop();
	self->type = OBJECT;
	self->X = x;
	self->Y = y;
	self->center_x = x + self->figure->width / 2;
	self->center_x = x + self->figure->height / 2;
	self->lower_edge = y + self->figure->height;
	self->phis_model = NULL;
	return self;
}

void del_entity(Entity* self) {
	/*
	Destructor for 'Entity' structure
	*/
	int len = 0;
	Entity** object = NULL;
	switch (self->type) {
	case OBJECT:
		len = entnum;
		object = entarray;
		for (; *object != self; object++);
		for (; *object != *(entarray + len); object++) {
			*object = *(object + 1);
		}
		entnum--;
		break;
		
	case BONE:
		len = effectnum;
		object = effectarray;
		for (; *object != self; object++);
		for (; *object != *(effectarray + len); object++) {
			*object = *(object + 1);
		}
		effectnum--;
		break;
	}
	if (self->phis_model != NULL) del_prop(self->phis_model);
	del_figure(self->figure);
	self = (Entity*)malloc(sizeof(Entity));
	
	free(self);
}

void setProp(Entity* self, VectorArr upper, VectorArr lower, VectorArr left, VectorArr right) {
	/*
	Method, that creates new Prop, using given arrays and sets that prop as entity's phisics model
	*/
	//static Prop* new_prop;
	//new_prop = init_prop(LINES, upper, lower, left, right);
	if (self->phis_model == NULL) {
		self->phis_model = init_prop(BONES, upper, lower, left, right);
	}
	else {
		del_prop(self->phis_model);
		self->phis_model = init_prop(BONES, upper, lower, left, right);
	}
	
}

Entity* registerEntity(int x, int y, LPCTSTR path_to_image, const char* with_prop, ...) {
	/*
	Creates new entity, sets it on given cordinates, loads image by 'path_to_image'. If 'with_prop' is "Prop:", then 
	takes 4 'VerexArr' objects, creates 'Prop' instance, using those arrays and sets created entity's 'phi_model'
	field as this new prop
	*/
	Entity* object = init_entity(x, y, path_to_image);
	addEnt(object);
	if (with_prop == "Prop:") {
		va_list vertex_arrs;
		va_start(vertex_arrs, with_prop);
		VectorArr up = va_arg(vertex_arrs, VectorArr);
		VectorArr down = va_arg(vertex_arrs, VectorArr);
		VectorArr left = va_arg(vertex_arrs, VectorArr);
		VectorArr right = va_arg(vertex_arrs, VectorArr);
		va_end(vertex_arrs);
		setProp(object, up, down, left, right);
	}
	return object;
}

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
	register int steps = 0;
	for (int i = 1; i < entnum; i++) {
		if (steps) i += steps - 1;
		steps = 0;
		while (i && entarray[i]->lower_edge < entarray[i - 1]->lower_edge) {
			temp = entarray[i];
			entarray[i] = entarray[i - 1];
			entarray[i - 1] = temp;
			//if (*object == &main_ch)
			i--;
			steps++;
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

COLLISION_SIDE check_collision_with_all(Entity* self, int step, COLLISION_SIDE side) {
	/*
	Checks collision of self and any other object on the field
	*/
	Prop* self_model = self->phis_model;
	if (self->phis_model->nocollide) return NOCOLLIDE;
	for (Entity** object = entarray; object != entarray + entnum; object++)
		if (*object != self && (*object)->phis_model != NULL && collide_side(self_model, (*object)->phis_model, step, side)) {
			return side;
		}
	return NOCOLLIDE;
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
		Entity* ent;
	};
	va_start(arguments, type);
	
	switch (type) {
	case FIGURE:
		figure = va_arg(arguments, Figure*);
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		move_transparent_image(dx, dy, figure, BLACK, true);
		break;
	case ENTITY:
		ent = va_arg(arguments, Entity*);
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		if ((dy < 0 && check_collision_with_all(ent, fabs(dy), UP)) || (dy > 0 && check_collision_with_all(ent, fabs(dy), DOWN))) {
			dy = 0;
		}
		if ((dx < 0 && check_collision_with_all(ent, fabs(dx), LEFT)) || (dx > 0 && check_collision_with_all(ent, fabs(dx), RIGHT))) {
			dx = 0;
		}
		move(FIGURE, ent->figure, dx, dy);
		shift(PROP, dx, dy, ent->phis_model);
		ent->lower_edge += dy;
		
		break;
	}
	va_end(arguments);
}

void show_bones(Entity* self, COLLISION_SIDE side) {
	/*
	Method, used to show on canvas all entity's bones (Verexes of prop)
	*/
	VectorArr* varrays[4] = {};
	if (self->phis_model != NULL) {
		switch (side) {
		case UP:
			varrays[0] = &self->phis_model->upper;
			break;
		case DOWN:
			varrays[0] = &self->phis_model->lower;
			break;
		case LEFT:
			varrays[0] = &self->phis_model->left;
			break;
		case RIGHT:
			varrays[0] = &self->phis_model->right;
			break;
		default:
			varrays[0] = &self->phis_model->upper;
			varrays[1] = &self->phis_model->lower;
			varrays[2] = &self->phis_model->left;
			varrays[3] = &self->phis_model->right;
		}
		for (VectorArr** varray = varrays; varray != varrays + 4 && *varray != NULL; varray++)
			for (int i = 0; i < (*varray)->length; i++) {
				addEffect(init_entity((*varray)->vectors[i].p1->X, (*varray)->vectors[i].p1->Y, L"assets\\bone.png"));
				addEffect(init_entity((*varray)->vectors[i].p2->X, (*varray)->vectors[i].p2->Y, L"assets\\bone.png"));
				effectarray[effectnum - 1]->type = BONE;
				effectarray[effectnum - 2]->type = BONE;
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
		for (Entity** ent = entarray; ent != entarray + entnum; ent++)
			show_bones(*ent, side);
		visiable = true;
	}
	else { // delete all bones from 'effectarray'
		int len = effectnum;
		for (Entity** effect = effectarray; effect != effectarray + len && effectarray[0] != NULL; effect++) {
			if ((*effect)->type == BONE) {
				del_entity(*effect);
				effect--;
			}
		}
		visiable = false;
	}
}

void renderAll(bool flag) {
	/*
	Clears canvas and puts all objects on it.
	*/
	DWORD* dst;
	if (flag) {
		memmove(bg, bg_src, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
		for (Entity** object = entarray; object != entarray + entnum; object++) {
			dst = move_transparent_image(0, 0, (*object)->figure, BLACK, false);
			memmove(bg, dst, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
		}
		
		for (Entity** object = effectarray; object != effectarray + effectnum; object++) {
			dst = move_transparent_image(0, 0, (*object)->figure, BLACK, false);
			memmove(bg, dst, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(DWORD));
		}
	}
}