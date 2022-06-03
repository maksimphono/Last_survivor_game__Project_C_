#ifndef PHISICS_ENGINE

#define PHISICS_ENGINE "phisics_engine.c"
#include "constants.c"
//#include "standart_types.c"
#include "phisics_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <windows.h>
#include <assert.h>

#define _X 0
#define _Y 1

const double EPS = 0.001;
const double PI = 3.141592653589793;
typedef enum {NOCOLLIDE, UP, DOWN, LEFT, RIGHT} COLLISION_SIDE;

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

void del_vertex(Vertex* self) {
	/*
	Destructor for 'Vertex' structure
	*/
	self->X = NULL;
	self->Y = NULL;
	free(self);
}

Vector* init_vector(int x1, int y1, int x2, int y2) {
	/*
	Constructor for structure 'Vector'
	*/
	static Vector* self;
	self = (Vector*)malloc(sizeof(Vector));
	self->p1 = init_vertex(x1, y1);
	self->p2 = init_vertex(x2, y2);
	self->length = sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
	self->plain[_X] = x1 - x2;
	self->plain[_Y] = y1 - y2;
	self->next = NULL;
	self->prev = NULL;
	return self;
}

void del_vector(Vector* self) {
	self->length = 0;
	self->plain[_X] = 0;
	self->plain[_Y] = 0;
	del_vertex(self->p1);
	del_vertex(self->p2);
	
	self = (Vector*)realloc(self, 1);
	//self = (Vector*)malloc(sizeof(self));
	free(self);
}

void addVector(VectorArr* self, int x1, int y1, int x2, int y2);

VectorArr* init_vectorarr(int n_array[19][4], ...) {
	static VectorArr* self;
	self = (VectorArr*)malloc(sizeof(VectorArr));
	va_list arguments;
	int x1 = 0, x2 = 0, y1 = 0, y2 = 0, len, n;
	self->vectors = (Vector*)malloc(sizeof(Vector));
	self->length = 0;
	va_start(arguments, n_array);
	if ((long long)n_array != 0) {
		n = va_arg(arguments, int);
		for (int i = 0; i != n && n_array[i][0] != 0; i += 1) {
			x1 = n_array[i][0];
			x2 = n_array[i][2];
			y1 = n_array[i][1];
			y2 = n_array[i][3];
			addVector(self, x1, y1, x2, y2);
		}
	}
	else {
		n = va_arg(arguments, int);
		for (int i = 0; i < n; i++) {
			x1 = va_arg(arguments, int);
			y1 = va_arg(arguments, int);
			x2 = va_arg(arguments, int);
			y2 = va_arg(arguments, int);
			addVector(self, x1, y1, x2, y2);
		}
	}
	va_end(arguments);
	return self;
}

void del_vectorarr(VectorArr* self) {
	for (int i = 0; i < self->length; i++) {
		del_vector(&self->vectors[i]);
	}
	//self->vectors = NULL;//(Vector*)realloc(self->vectors, 1);
	self = (VectorArr*)realloc(self, 1);
	free(self);
}

void addVector(VectorArr* self, int x1, int x2, int y1, int y2) {
	Vector* vector = init_vector(x1, x2, y1, y2);
	self->vectors = (Vector*)realloc(self->vectors, sizeof(Vector) * (1 + self->length));
	self->vectors[self->length++] = *vector;
}

Vector* get(VectorArr* self, unsigned index) {
	return &self->vectors[index];
}

Vertex* find_central_vertex(Prop* self) {
	/*
	Finds central vertex for prop
	*/
	int len = 0;
	int sumX = 0;
	int sumY = 0;
	Vector vector;
	switch (self->collision_type) {
	case BONES:
		VectorArr arrays[4] = {self->upper , self->lower , self->left , self->right};
		for (int arr_n = 0; arr_n < 4; arr_n++) {
			for (int i = 0; i < arrays[arr_n].length; i++, len += 2) {
				vector = arrays[arr_n].vectors[i];
				sumX += vector.p1->X;
				sumY += vector.p1->Y;
				sumX += vector.p2->X;
				sumY += vector.p2->Y;
			}
		}
		return init_vertex(sumX / len, sumY / len);
		break;
	}
	return NULL;
}

Prop* init_prop(COLLISION_TYPE type, ...) {
	/*
	Constructor for structure 'Prop'
	*/
	static Prop* self;
	VectorArr upper, lower, left, right;
	va_list arguments;
	unsigned collide_radius = 0;
	int x, y;

	self = (Prop*)malloc(sizeof(Prop));
	va_start(arguments, type);
	self->collision_type = type;
	switch (type) {
	case RADIUS:
		x = va_arg(arguments, int);
		y = va_arg(arguments, int);
		self->collide_radius = va_arg(arguments, unsigned);
		self->center = init_vertex(x, y);
		//self = init_prop(RADIUS, collide_radius);
		break;
	case BONES:
		self->upper = va_arg(arguments, VectorArr);
		self->lower = va_arg(arguments, VectorArr);
		self->left = va_arg(arguments, VectorArr);
		self->right = va_arg(arguments, VectorArr);
		//self = init_prop(BONES, upper, lower, left, right);
		self->center = find_central_vertex(self);
		break;
	}
	self->nocollide = false;
	return self;
}

void setSideCollision(Prop* self, COLLISION_SIDE side, int points[MAX_VECTOR_NUM][4]) {
	VectorArr* varray = NULL;
	switch (side) {
	case UP:
		varray = &self->upper;
		break;
	case DOWN:
		varray = &self->lower;
		break;
	case LEFT:
		varray = &self->left;
		break;
	case RIGHT:
		varray = &self->right;
		break;
	default:
		return;
	}
	*varray = *init_vectorarr(points);
}

void del_prop(Prop* self) {
	if (self == NULL) return;
	switch (self->collision_type) {
	case BONES:
		del_vectorarr(&self->upper);
		del_vectorarr(&self->lower);
		del_vectorarr(&self->left);
		del_vectorarr(&self->right);
		free(self->center);
		break;
	}
	free(self);
}

void shift(PHISICS_TYPE type, int dx, int dy, ...) {
	/*
	Method, that moves phisical object by dx and dy coordinates
	*/
	va_list argument;
	va_start(argument, dy);
	union {
		Vertex* vertex;
		Vector* vector;
		VectorArr* vectorarr;
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
	case VECTOR:
		vector = va_arg(argument, Vector*);
		shift(VERTEX, dx, dy, vector->p1);
		shift(VERTEX, dx, dy, vector->p2);
		break;
	case VECTORARR:
		vectorarr = va_arg(argument, VectorArr*);
		for (Vector* vector = vectorarr->vectors; vector != vectorarr->vectors + vectorarr->length; vector++) {
			shift(VECTOR, dx, dy, vector);
		}
		break;
	case PROP:
		prop = va_arg(argument, Prop*);
		if (prop != NULL) {
			switch (prop->collision_type) {
			case BONES:
				shift(VECTORARR, dx, dy, prop->upper);
				shift(VECTORARR, dx, dy, prop->lower);
				shift(VECTORARR, dx, dy, prop->left);
				shift(VECTORARR, dx, dy, prop->right);
			}
			shift(VERTEX, dx, dy, prop->center);
		}
		break;
	}
	va_end(argument);
}

bool isBetween(int a_X, int a_Y, int b_X, int b_Y, double c_X, double c_Y) {
	/*
	Checks, whether vertex c lies between verteces a and b
	*/
	double dotproduct;
	double sqr_ba;
	double crossproduct = (c_Y - a_Y) * (b_X - a_X) - (c_X - a_X) * (b_Y - a_Y);
	if (fabs(crossproduct) >= EPS) return false;
	dotproduct = (c_X - a_X) * (b_X - a_X) + (c_Y - a_Y) * (b_Y - a_Y);
	if (dotproduct < 0) return false;
	sqr_ba = pow((b_X - a_X), 2) + pow((b_Y - a_Y), 2);
	if (dotproduct > sqr_ba) return false;
	return true;
}

double distance_between_verteces(Vertex* v1, Vertex* v2) {
	// find distance between two verteces
	double d = sqrt(pow((v2->X - v1->X), 2) + pow((v2->Y - v1->Y), 2));
	return d;
}

double distance_to_vector(Vertex* vertex, Vector* vector) {
	double a, b, d;
	double y1 = vector->p1->Y;
	double x1 = vector->p1->X;
	double x2 = vector->p2->X;
	double y2 = vector->p2->Y;
	if (fabs(x2 - x1) <= EPS) return fabs(vertex->X - vector->p1->X);

	a = (y1 / (x1 - x2) + y2 / (x2 - x1));
	b = -(y1 / (x1 - x2)) * x2 - (y2 / (x2 - x1)) * x1;

	d = fabs(a * vertex->X - vertex->Y + b) / sqrt(1 + pow(a, 2));
	return d;
}

double distance_between_vectors(Vector* v1, Vector* v2) {
	/*
	Finds distanse between parallel vectors with respect to axis, vectors tilting to
	*/
	double x1 = v1->p1->X;
	double y1 = v1->p1->Y;
	double x2 = v1->p2->X;
	double y2 = v1->p2->Y;

	double x3 = v2->p1->X;
	double y3 = v2->p1->Y;
	double x4 = v2->p2->X;
	double y4 = v2->p2->Y;
	double d, a1, b1, b2;

	if (fabs(x1 - x2) <= EPS) { // complete this !!!
		if (max(y1, y2) <= min(y3, y4) || min(y1, y2) >= max(y3, y4))
			d = min(min(distance_between_verteces(v1->p1, v2->p1), distance_between_verteces(v1->p2, v2->p1)),
				min(distance_between_verteces(v1->p1, v2->p2), distance_between_verteces(v1->p2, v2->p2)));
		else d = fabs(x1 - x3);
		return d;
	}
	else {
		a1 = (y1 / (x1 - x2) + y2 / (x2 - x1));
		b1 = -(y1 / (x1 - x2)) * x2 - (y2 / (x2 - x1)) * x1;
		b2 = -(y3 / (x3 - x4)) * x4 - (y4 / (x4 - x3)) * x3;
		d = fabs(b2 - b1) / sqrt(1 + pow(a1, 2));
	}

	if (fabs(a1) <= EPS) { // fixing bug with parallel gorizontal bones
		if (max(x1, x2) < min(x3, x4) || min(x1, x2) > max(x3, x4))
			d = min(min(distance_between_verteces(v1->p1, v2->p1), distance_between_verteces(v1->p2, v2->p1)),
				min(distance_between_verteces(v1->p1, v2->p2), distance_between_verteces(v1->p2, v2->p2)));
		else
			d = v1->p1->Y - v2->p1->Y;
	}
	else if (a1 > 1) { // if vector is tilting to OY axis
		if (max(x1, x2) <= min(x3, x4) || min(x1, x2) >= max(x3, x4))
			d = min(min(distance_between_verteces(v1->p1, v2->p1), distance_between_verteces(v1->p2, v2->p1)),
				min(distance_between_verteces(v1->p1, v2->p2), distance_between_verteces(v1->p2, v2->p2)));
		else
			d /= fabs(cos(atan(a1)));
	}
	else if (a1 < 1) { // if vector is tilting to OX axis
		if (max(y1, y2) < min(y3, y4) || min(y1, y2) > max(y3, y4))
			d = min(min(distance_between_verteces(v1->p1, v2->p1), distance_between_verteces(v1->p2, v2->p1)),
				min(distance_between_verteces(v1->p1, v2->p2), distance_between_verteces(v1->p2, v2->p2)));
		else
			d /= fabs(cos(0.5 * PI - atan(a1)));
	}
	return fabs(d);
}

double prop_centers_distance(Prop* object1, Prop* object2) {
	//printf("Object2 center: %d, %d", object2->center->X, )
	return sqrt(pow(object1->center->X - object2->center->X, 2) + pow(object1->center->Y - object2->center->Y, 2));
}

double crossProduct(double v[2], double w[2]) {
	return v[_X] * w[_Y] - v[_Y] * w[_X];
}

bool countClockwise(Vertex* A, Vertex* B, Vertex* C) {
	// Checks, whether point C lays clockwise from A and B
	return (C->Y - A->Y) * (B->X - A->X) > (B->Y - A->Y) * (C->X - A->X);
}

bool new_line_cross(Vector* v1, Vector* v2, int min_distance) {
	/*
	Checks, whether vectors cross each other. Used method: "Clockwise check".
	Argument 'min_distance' represent minimal allowable distance between vectors if they are parallel. If distance is less than 'step', vectors intersecs
	*/
	static Vector vI = *init_vector(0, 0, 1, 0), vJ = *init_vector(0, 0, 0, 1); // identity vectors vI = (1, 0) and vJ = (0, 1)
	Vertex* A = v1->p1;
	Vertex* B = v1->p2;
	Vertex* C = v2->p1;
	Vertex* D = v2->p2;

	double r[2] = { v1->p2->X - v1->p1->X, v1->p2->Y - v1->p1->Y };
	double s[2] = { v2->p2->X - v2->p1->X, v2->p2->Y - v2->p1->Y };
	double relative_tilt = crossProduct(r, s);
	double d;

	if (fabs(relative_tilt) <= EPS) { // vectors are collinear (parallel)
		d = distance_between_vectors(v1, v2);
		if (d <= min_distance) return true;
		return false;
	}

	return countClockwise(A, C, D) != countClockwise(B, C, D) && countClockwise(A, B, C) != countClockwise(A, B, D);
}

bool vector_circle_cross(Vector* v, Vertex* center, int radius) {
	double d = distance_to_vector(center, v), a, b, c;
	double sqr_b, ac4, x1 = v->p1->X, x2 = v->p2->X, y1 = v->p1->Y, y2 = v->p2->Y, h = center->X, k = center->Y, t1, t2;
	if (fabs(y1 - y2) <= EPS) return min(fabs(v->p1->Y - center->Y), fabs(center->Y - v->p2->Y)) <= radius;
	else if (fabs(x1 - x2) <= EPS) return min(fabs(v->p1->X - center->X), fabs(center->X - v->p2->X)) <= radius;
	a = (pow(x2 - x1, 2) + pow(y2 - y1, 2));
	b = 2*(x2-x1)*(x1-h)+2*(y2-y1)*(y1-k);
	sqr_b = pow(b, 2);
	c = pow((x1-h), 2) + pow((y1-k), 2) - pow(radius, 2);
	ac4 = 4 * a * c;
	if (sqr_b - ac4 <= 0) return false;
	return true;
}

const bool collide_side(Prop* self, Prop* prop, int min_distance, COLLISION_SIDE side) {
	/*
	Check collision between 2 objects by only one side
	*/
	Vector* pivot, * v1, * v2, *v3, *v4;
	VectorArr* self_varray = NULL;
	VectorArr* prop_varray = NULL;
	int distance;


	if (self == NULL || prop == NULL) return false;
	
	if (self->collision_type == BONES && prop->collision_type == BONES) {
		switch (side) {
		case UP:
			self_varray = &self->upper;
			prop_varray = &prop->lower;
			break;
		case DOWN:
			self_varray = &self->lower;
			prop_varray = &prop->upper;
			break;
		case LEFT:
			self_varray = &self->left;
			prop_varray = &prop->right;
			break;
		case RIGHT:
			self_varray = &self->right;
			prop_varray = &prop->left;
			break;
		default:
			return false;
		}
		for (Vector* v1 = self_varray->vectors; v1 != self_varray->vectors + self_varray->length; v1++) {
			for (Vector* v2 = prop_varray->vectors; v2 != prop_varray->vectors + prop_varray->length; v2++) {
				if (new_line_cross(v1, v2, min_distance)) return true;
			}
		}
	}
	else if (prop->collision_type == BONES && self->collision_type == RADIUS) {
		switch (side) {
		case UP:
			prop_varray = &prop->lower;
			break;
		case DOWN:
			prop_varray = &prop->upper;
			break;
		case LEFT:
			prop_varray = &prop->right;
			break;
		case RIGHT:
			prop_varray = &prop->left;
			break;
		default:
			return false;
		}
		for (Vector* v1 = prop_varray->vectors; v1 != prop_varray->vectors + prop_varray->length; v1++) {
			distance = min(min(distance_between_verteces(self->center, v1->p1), distance_between_verteces(self->center, v1->p2)), distance_to_vector(self->center, v1));
			if (distance < self->collide_radius) return true;
		}
	}
	else if (self->collision_type == BONES && prop->collision_type == RADIUS) {
		switch (side) {
		case DOWN:
			self_varray = &self->lower;
			break;
		case UP:
			self_varray = &self->upper;
			break;
		case RIGHT:
			self_varray = &self->right;
			break;
		case LEFT:
			self_varray = &self->left;
			break;
		default:
			return false;
		}
		for (Vector* v1 = self_varray->vectors; v1 != self_varray->vectors + self_varray->length; v1++) {
			//distance = min(min(distance_between_verteces(prop->center, v1->p1), distance_between_verteces(prop->center, v1->p2)), distance_to_vector(prop->center, v1));
			if (vector_circle_cross(v1, prop->center, prop->collide_radius)) return true;
		}
	}
		
	return false;
}

const bool collide_vector_side(Vector* vector, Prop* prop, int min_distance, COLLISION_SIDE side) {
	Vector* pivot, * v1, * v2, * v3, * v4;
	VectorArr* prop_varray = NULL;
	int distance;
	v1 = vector;

	if (vector == NULL || prop == NULL) return false;

	if (prop->collision_type == BONES) {
		switch (side) {
		case UP:
			prop_varray = &prop->lower;
			break;
		case DOWN:
			prop_varray = &prop->upper;
			break;
		case LEFT:
			prop_varray = &prop->right;
			break;
		case RIGHT:
			prop_varray = &prop->left;
			break;
		default:
			return false;
		}
		
		for (Vector* v2 = prop_varray->vectors; v2 != prop_varray->vectors + prop_varray->length; v2++) {
			if (new_line_cross(v1, v2, min_distance)) return true;
		}
	}
	return false;
}
#endif