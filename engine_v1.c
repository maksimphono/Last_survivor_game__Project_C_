#include "constants.c"
#include "standart_types.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <windows.h>



typedef enum {VERTEX, VARRAY, PROP} PHISICS_TYPE;
typedef enum {CIRCLE, FUNCTION, LINES} COLLISION_TYPE;

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

void addVertex(VertexArr* self, int x, int y) {
	/*
	Creates and adds new vertex to array using these coordinates
	*/
	self->vertexes[self->length++] = *init_vertex(x, y);
}

VertexArr* init_varray(int n, ...) {
	/*
	Constructor for structure 'VertexArr'
	*/
	static VertexArr* self;
	self = (VertexArr*)malloc(sizeof(VertexArr));
	self->length = 0;
	self->vertexes = (Vertex*)malloc(MAX_VERTEX_ARR_LEN * sizeof(Vertex));
	if (n) {
		va_list verteces;
		va_start(verteces, n);
		for (int i = 0; i < n; i++) {
			int X = va_arg(verteces, int);
			int Y = va_arg(verteces, int);
			addVertex(self, X, Y);
		}
		va_end(verteces);
	}
	return self;
}

typedef struct Prop {
	/*
	Abstract structure, that represents phisics object. It has 4 vertex arrays, that represents 
	upper, lower, left and right collision Vertexes. 'center' represents the point at the center of the object.
	*/
	COLLISION_TYPE collision_type; // Prop's collision model can be represented certain ways: 
	//	1) CIRCLE (most quick check, because i just check distance to the center of the target prop)
	//	2) FUNCTION (i just have to check, if collision model's graphic or lines cross target prop's graphic)
	//	3) LINES (most slow to check, must check if at least one line of pivot prop cross ta least 1 target prop's line)

	union { 
		struct {
			VertexArr upper;
			VertexArr lower;
			VertexArr left;
			VertexArr right;
		};
		
	};
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
	self->upper = *init_varray(0);
	self->lower = *init_varray(0);
	self->left = *init_varray(0);
	self->right = *init_varray(0);
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

Vertex* find_central_vertex(Prop* self) {
	int len = 0;
	int sumX = 0;
	int sumY = 0;
	switch (self->collision_type) {
	case LINES:
		for (int i = 0; i < self->upper.length; i++, len++) {
			Vertex vertex = self->upper.vertexes[i];
			sumX += vertex.X;
			sumY += vertex.Y;
		}
		for (int i = 0; i < self->lower.length; i++, len++) {
			Vertex vertex = self->lower.vertexes[i];
			sumX += vertex.X;
			sumY += vertex.Y;
		}
		for (int i = 0; i < self->left.length; i++, len++) {
			Vertex vertex = self->left.vertexes[i];
			sumX += vertex.X;
			sumY += vertex.Y;
		}
		for (int i = 0; i < self->right.length; i++, len++) {
			Vertex vertex = self->right.vertexes[i];
			sumX += vertex.X;
			sumY += vertex.Y;
		}
		break;
	}

	return init_vertex(sumX / len, sumY / len);
}

Prop* init_prop(COLLISION_TYPE type, VertexArr upper, VertexArr lower, VertexArr left, VertexArr right) {
	/*
	Constructor for structure 'Prop'
	*/
	static Prop* self;
	self = (Prop*)malloc(sizeof(Prop));
	self->upper = upper;
	self->lower = lower;
	self->left = left;
	self->right = right;
	self->collision_type = type;
	self->center = *find_central_vertex(self);
	return self;
}

void shift(PHISICS_TYPE type, int dx, int dy, ...) {
	/*
	Method, that moves phisical object by dx and dy coordinates
	*/
	va_list argument;
	va_start(argument, dy);
	union {
		Vertex* vertex;
		VertexArr* varray;
		Prop* prop;
	};
	switch (type) {
	case VERTEX:
		vertex = va_arg(argument, Vertex*);
		if (vertex != NULL) {
			vertex->X += dx;
			vertex->Y += dy;
		}
		break;
	case VARRAY:
		varray = va_arg(argument, VertexArr*);
		for (Vertex* vertex = varray->vertexes; vertex != varray->vertexes + varray->length; vertex++) {
			shift(VERTEX, dx, dy, vertex);
		}
		break;
	case PROP:
		prop = va_arg(argument, Prop*);
		if (prop != NULL) {
			switch (prop->collision_type) {
			case LINES:
				shift(VARRAY, dx, dy, prop->upper);
				shift(VARRAY, dx, dy, prop->lower);
				shift(VARRAY, dx, dy, prop->left);
				shift(VARRAY, dx, dy, prop->right);
			}
			shift(VERTEX, dx, dy, &prop->center);
		}
		break;
	}
	va_end(argument);
}


