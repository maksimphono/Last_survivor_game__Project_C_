#include "constants.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <windows.h>

typedef enum {VERTEX, PROP};

typedef struct {
	/*
	Abstraction structure, that represents collision Vertex
	*/
	int X;
	int Y;
} Vertex;

Vertex* init_vertex(int x, int y) {
	/*
	Constructor for 'Vertex' structure
	*/
	static Vertex* self;
	self = (Vertex*)malloc(sizeof(Vertex));
	self->X = x;
	self->Y = y;
	return self;
}

typedef struct VertexArr {
	/*
	Abstract structure, that represents array of vetrexes
	*/
	Vertex* vertexes;
	unsigned length;
} VertexArr;

VertexArr* init_varray() {
	/*
	Constructor for structure 'VertexArr'
	*/
	static VertexArr* self;
	self = (VertexArr*)malloc(sizeof(VertexArr));
	self->length = 0;
	self->vertexes = (Vertex*)malloc(MAX_VERTEX_ARR_LEN * sizeof(Vertex));
	return self;
}

void addVertex(VertexArr* self, int x, int y) {
	/*
	Creates and adds to array new vertex using these coordinates
	*/
	self->vertexes[self->length++] = *init_vertex(x, y);
}

void shift() {
	return;
}

typedef struct Prop {
	/*
	Abstract structure, that represents phisics object. It has 4 vertex arrays, that represents 
	upper, lower, left and right collision Vertexes. 'center' represents the point at the center of the object.
	*/
	VertexArr upper;
	VertexArr lower;
	VertexArr left;
	VertexArr right;
	Vertex center;
} Prop;

Prop* init_prop(int upper_x[], 
				int upper_y[],
				int upper_len,
				int lower_x[],
				int lower_y[],
				int lower_len,
				int left_x[],
				int left_y[],
				int left_len,
				int right_x[],
				int right_y[],
				int right_len) {
	/*
	Constructor for structure 'Prop'
	*/
	static Prop* self;
	self = (Prop*)malloc(sizeof(Prop));
	Vertex* vertexes = (Vertex*)malloc(sizeof(Vertex) * MAX_VERTEX_ARR_LEN);
	self->upper = *init_varray();
	self->lower = *init_varray();
	self->left = *init_varray();
	self->right = *init_varray();
	VertexArr* arrays[4] = {&self->upper, &self->lower, &self->left, &self->right};
	int* list_x[4] = { upper_x, lower_x, left_x, right_x };
	int* list_y[4] = { upper_y, lower_y, left_y, right_y };
	int lengths[4] = { upper_len, lower_len, left_len, right_len };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < lengths[i]; j++)
			addVertex(arrays[i], list_x[i][j], list_y[i][j]);
	}
	return self;
}