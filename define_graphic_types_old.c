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
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

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

void render(GRAPHIC_TYPE type, ...) {
	/*
	function, that puts image on the canvas. 'type' argument represents abstract type of object, that have to be rendered
	can be 'FIGURE' or 'ENTITY'. After 'type' there must be only 1 argument - object, that must be rendered
	*/
	va_list argument;
	Entity* object = (Entity*)malloc(sizeof(Entity));

	va_start(argument, type);
	switch (type) {
	case FIGURE: //		if object is 'FIGURE'
		object->figure = va_arg(argument, Figure*);
		break;
	case ENTITY:
		object = va_arg(argument, Entity*);
		break;
	}
	put_transparent_picture(object->figure->X, object->figure->Y, &images[object->figure->img_index], BLACK);
	va_end(argument);
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
	va_start(arguments, type);
	Entity* object = (Entity*)malloc(sizeof(Entity));
	switch (type) {
	case FIGURE:
		object->figure = va_arg(arguments, Figure*);
		object->figure->X += va_arg(arguments, int);
		object->figure->Y += va_arg(arguments, int);
		break;
	case ENTITY:
		object = va_arg(arguments, Entity*);
		object->figure->X += va_arg(arguments, int);
		object->figure->Y += va_arg(arguments, int);
		break;
	}
	va_end(arguments);
}