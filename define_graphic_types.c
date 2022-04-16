#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <windows.h>

#include <graphics.h>
#include <conio.h>
#include <time.h>

#define MAX_INT 4294967296
#define MAX_IMAGE_NUM 19
#define MAX_ENTITY_NUM 19
#define MAX_FIGURE_NUM 19
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define BITBLACK 4278190080
#define BITWHITE 4294967295

LPCTSTR MAIN_BG_MODEL_PATH = L"assets\\main_bg_01.png";
//LPCTSTR MAIN_CH_MODEL_PATH = L"assets\\main_character_transparent_3.png";
LPCTSTR MAIN_CH_MODEL_PATH = L"assets\\test_transp.png";

typedef unsigned long long ull;
typedef char string[50];
typedef enum { FIGURE, IMG, ENTITY } GRAPHIC_TYPE;

extern IMAGE images[MAX_IMAGE_NUM] = {}; // array, that contains all images, that will be rendered on canvas

IMAGE* setupImage(unsigned* img_index) {
	/*
	takes IMAGE object from 'images' array, return pointer to this image and write index of the image to 'img_index'
	*/
	static unsigned images_len = 0;
	if (images_len > MAX_IMAGE_NUM) return NULL;
	*img_index = images_len;
	return &images[images_len++];
}

unsigned null_image;
extern IMAGE main_background = *setupImage(&null_image);
extern DWORD* bg = NULL;

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
	Figure* figure;
	int center_x;
	int center_y;
	int X;
	int Y;
} Entity;

typedef struct EntArray {
	Entity* entities;
	int length;
} EntArray;

extern Entity* entarray[MAX_ENTITY_NUM] = {};
extern int entnum = 0;

void addEnt(Entity* ent) {
	entarray[entnum++] = ent;
}

Figure* init_figure(int x, int y, LPCTSTR path_to_image) {
	/*
	constructor for Figure struct
	*/
	static Figure* self;
	self = (Figure*)malloc(sizeof(Figure));
	IMAGE* img = setupImage(&self->img_index); // access to image in 'images' array
	loadimage(img, path_to_image);
	self->X = x;
	self->Y = y;
	self->width = img->getwidth();
	self->height = img->getheight();
	return self;
}

Entity* init_entity(int x, int y, LPCTSTR path_to_image) {
	/*
	Constructor for 'Entity' structure
	*/
	static Entity* self;
	self = (Entity*)malloc(sizeof(Entity));
	self->figure = init_figure(x, y, path_to_image);
	self->X = x;
	self->Y = y;
	return self;
}

void registerEntity(int x, int y, LPCTSTR path_to_image) {
	Entity* object = init_entity(x, y, path_to_image);
	addEnt(object);
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
			if (src[j * image_width + i] != color) {
				ull real_pixel_pos = (j + dsty) * screen_width + i + dstx;
				ull image_pixel_pos = j * image_width + i;
				if (dsty < 0)
					dsty = 0;
				else if (dsty + image_height >= SCREEN_WIDTH)
					dsty = SCREEN_WIDTH - image_height;
				dst[real_pixel_pos] = src[image_pixel_pos];
			}
		}
	if (!GetWorkingImage())
		FlushBatchDraw();
}

void move_transparent_image(int dx, int dy, Figure* fig, COLORREF color) {
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
				//dst[real_pixel_pos - 2] = bg[real_pixel_pos - 2];
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

void move(GRAPHIC_TYPE type, ...) {
	/*
	Shifts object (second argument) of type 'type' by X axis by value 'third argument', by Y axis by 'fourth' argument
	*/
	va_list arguments;
	int dx;
	int dy;
	va_start(arguments, type);
	Figure* figure = (Figure*)malloc(sizeof(Figure));
	switch (type) {
	case FIGURE:
		figure = va_arg(arguments, Figure*);
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		move_transparent_image(dx, dy, figure, BLACK);
		break;
	case ENTITY:
		Entity* ent = va_arg(arguments, Entity*);
		dx = va_arg(arguments, int);
		dy = va_arg(arguments, int);
		move(FIGURE, ent->figure, dx, dy);
		break;
	}
	va_end(arguments);
}

void renderAll() {
	putimage(0, 0, &main_background);
	for (Entity** object = entarray; object != entarray + entnum; object++) {
		move(ENTITY, *object, 0, 0);
	}
}