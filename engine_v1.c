#include "constants.c"
#include "standart_types.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <windows.h>
#include <assert.h>

#define _X 0
#define _Y 1

const double EPS = 0.01;
const double PI = 3.141592653589793;

typedef enum {VERTEX, VARRAY, LINE, PROP} PHISICS_TYPE;
typedef enum {CIRCLE, FUNCTION, LINES} COLLISION_TYPE;
typedef enum {NOCOLLIDE, UP, DOWN, LEFT, RIGHT} COLLISION_SIDE;


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

void del_vertex(Vertex* self) {
	/*
	Destructor for 'Vertex' structure
	*/
	self->X = NULL;
	self->Y = NULL;
	free(self);
}

typedef struct Vector {
	Vertex* p1;
	Vertex* p2;
	double length;
	double plain[2];
	Vector* next;
	Vector* prev;
} Vector;

Vector* init_vector(int x1, int y1, int x2, int y2) {
	/*
	Constructor for structure 'Vector'
	*/
	static Vector* self;
	self = (Vector*)malloc(sizeof(Vector));
	self->p1 = init_vertex(x1, y1);
	self->p2 = init_vertex(x2, y2);
	//self->length = (double*)malloc(sizeof(double));
	//self->plain[_X] = (double*)malloc(sizeof(double));
	//self->plain[_Y] = (double*)malloc(sizeof(double));
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
	
	//self = (Vector*)malloc(sizeof(self));
	//free(self);
}

typedef struct {
	Vector* vectors;
	unsigned length;
} VectorArr;

VectorArr* init_vectorarr() {
	static VectorArr* self;
	self = (VectorArr*)malloc(sizeof(VectorArr));
	self->vectors = (Vector*)malloc(sizeof(Vector));
	self->length = 0;
	return self;
}

void append(VectorArr* self, Vector* vector) {
	self->vectors = (Vector*)realloc(self->vectors, sizeof(Vector) * (1 + self->length));
	self->vectors[self->length++] = *vector;
}

typedef struct VertexArr {
	/*
	Abstract structure, that represents array of vetrexes
	*/
	Vertex* vertexes;
	Vertex** content_ptr;
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
	self->content_ptr = &self->vertexes;
	return self;
}

void del_varray(VertexArr* self) {
	for (Vertex* vertex = self->vertexes; vertex != self->vertexes + self->length; vertex++) {
		del_vertex(vertex);
	}
	static Vertex** vertex_arr = &self->vertexes;
	self->length = 0;
	self = (VertexArr*)malloc(sizeof(self));
	free(self);
}

Vertex* get(VertexArr* self, unsigned index) {
	return &self->vertexes[index];
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
	Vertex* center;
	bool nocollide;
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
	/*
	Finds central vertex for prop
	*/
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
	self->center = find_central_vertex(self);
	self->nocollide = false;
	return self;
}

void del_prop(Prop* self) {
	switch (self->collision_type) {
	case LINES:
		del_varray(&self->upper);
		del_varray(&self->lower);
		del_varray(&self->left);
		del_varray(&self->right);
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

double distance(PHISICS_TYPE type_1, PHISICS_TYPE type_2, ...) {
	va_list arguments;
	double a1, b1, a2, b2, x1, x2, y1, y2, X, Y, x3, y3, x4, y4;
	union {
		Vertex* v1, * v2, * v3, * v4;
		Prop* prop_1, * prop_2;
	};

	va_start(arguments, type_2);
	switch (type_1) {
	case VERTEX:
		v1 = va_arg(arguments, Vertex*);
		switch (type_2) {
		case VERTEX:
			v2 = va_arg(arguments, Vertex*);
			return sqrt(pow((v2->X - v1->X), 2) + pow((v2->Y - v1->Y), 2));
		case LINE:
			// I will count distance between vertex 'v1' and line with extreme vertexes (v2, v3)
			v2 = va_arg(arguments, Vertex*); // first extreme vertex of line
			v3 = va_arg(arguments, Vertex*); // second extreme vertex of line
			X = v1->X;
			x2 = v2->X;
			Y = v1->Y;
			y2 = v2->Y;
			x3 = v3->X;
			y3 = v3->Y;
			a1 = (y2 / (x2 - x3) + y3 / (x3 - x2));
			b1 = -(y2 / (x2 - x3)) * x3 - (y3 / (x3 - x2)) * x2;
			// if vertex 'v1' lies away of extreme vertexes, return distance to nearest extreme vertex
			if ((X > max(x3, x2) or X < min(x3, x2)) && (Y > max(y3, y2) or Y < min(y3, y2))) return min(distance(VERTEX, VERTEX, v2, v1), distance(VERTEX, VERTEX, v3, v1));
			return fabs(a1 * X - Y + b1) / sqrt(pow(a1, 2) + 1.0);
		}
	case LINE:
		v1 = va_arg(arguments, Vertex*);
		v2 = va_arg(arguments, Vertex*);
		switch (type_2) {
		case VERTEX:
			v3 = va_arg(arguments, Vertex*);
			return distance(VERTEX, LINE, v3, v1, v2);
		case LINE:
			// Warning!! Before count distance between two lines, make sure that these lines are parallel;  
			v3 = va_arg(arguments, Vertex*);
			v4 = va_arg(arguments, Vertex*);
			x1 = v1->X;
			y1 = v1->Y;
			x2 = v2->X;
			y2 = v2->Y;

			x3 = v3->X;
			y3 = v3->Y;
			x4 = v4->X;
			y4 = v4->Y;

			a1 = (y1 / (x1 - x2) + y2 / (x2 - x1));
			b1 = -(y1 / (x1 - x2)) * x2 - (y2 / (x2 - x1)) * x1;
			b2 = -(y3 / (x3 - x4)) * x4 - (y4 / (x4 - x3)) * x3;

			return fabs(b2 - b1) / sqrt(1 + pow(a1, 2));
		case PROP:
			prop_1 = va_arg(arguments, Prop*);
			prop_2 = va_arg(arguments, Prop*);
			return distance(VERTEX, VERTEX, prop_1->center, prop_2->center);
		}
	}
	va_end(arguments);
}


double distance(PHISICS_TYPE type_1, PHISICS_TYPE type_2, ...);

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

bool line_cross(Vertex* v1_1, Vertex* v1_2, Vertex* v2_1, Vertex* v2_2, char axis) {
	/*
	Determine, whether two lines are crossing each other
	*/
	double v1_1_X = v1_1->X;
	double v1_1_Y = v1_1->Y;
	double v1_2_X = v1_2->X;
	double v1_2_Y = v1_2->Y;
	double v2_1_X = v2_1->X;
	double v2_1_Y = v2_1->Y;
	double v2_2_X = v2_2->X;
	double v2_2_Y = v2_2->Y;
	double X;
	double Y;

	double a1 = (v1_1_Y / (v1_1_X - v1_2_X) + v1_2_Y / (v1_2_X - v1_1_X));
	double b1 = -(v1_1_Y / (v1_1_X - v1_2_X)) * v1_2_X - (v1_2_Y / (v1_2_X - v1_1_X)) * v1_1_X;
	double a2 = v2_1_Y / (v2_1_X - v2_2_X) + v2_2_Y / (v2_2_X - v2_1_X);
	double b2 = -(v2_1_Y / (v2_1_X - v2_2_X)) * v2_2_X - (v2_2_Y / (v2_2_X - v2_1_X)) * v2_1_X;

	if (fabs(a2 - a1) <= EPS) {
		puts("");
		b1 = distance(LINE, LINE, v1_1, v1_2, v2_1, v2_2);
		switch (axis) {
		case '|':
			if (max(v1_1_X, v1_2_X) < min(v2_1_X, v2_2_X) || min(v1_1_X, v1_2_X) > max(v2_1_X, v2_2_X)) {
				b1 = min(min(distance(VERTEX, VERTEX, v1_1, v2_1), distance(VERTEX, VERTEX, v1_2, v2_1)), min(distance(VERTEX, VERTEX, v1_1, v2_2), distance(VERTEX, VERTEX, v1_2, v2_2)));
				return (b1 <= EPS);
			}
			b1 = b1 / fabs(cos(atan(a1)));
			break;
		case '-':
			if (max(v1_1_Y, v1_2_Y) < min(v2_1_Y, v2_2_Y) || min(v1_1_Y, v1_2_Y) > max(v2_1_Y, v2_2_Y)) {
				b1 = min(min(distance(VERTEX, VERTEX, v1_1, v2_1), distance(VERTEX, VERTEX, v1_2, v2_1)), min(distance(VERTEX, VERTEX, v1_1, v2_2), distance(VERTEX, VERTEX, v1_2, v2_2)));
				return (b1 <= EPS);
			}
			b2 = atan(a1);
			b1 = b1 / fabs(cos(0.5 * PI - b2));
			break;
		}
		return (b1 <= 3.0);
		return false;
	}
	else {
		X = (b2 - b1) / (-a2 - (-a1));
		Y = b1 + (a1)*X;
	}

	if (isBetween(v1_1_X, v1_1_Y, v1_2_X, v1_2_Y, X, Y) && isBetween(v2_1_X, v2_1_Y, v2_2_X, v2_2_Y, X, Y)) return true;
	return false;
}
/*
double crossProduct(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4) {
	return (v2->X - v1->X) * (v4->Y - v3->Y) - (v2->Y - v1->Y) * (v4->X - v3->X);
}

bool line_cross_new(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4) {
	double s = v2
}
*/

const bool collide_side(Prop* self, Prop* prop, COLLISION_SIDE side) {
	/*
	Check collision between 2 objects by only one side
	*/
	Vertex* pivot, * v1, * v2, *v3, *v4;
	VertexArr* self_varray = NULL;
	VertexArr* prop_varray = NULL;
	char axis = '|';
	if (self == NULL || prop == NULL) return "";
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
		axis = '-';
		break;
	case RIGHT:
		self_varray = &self->right;
		prop_varray = &prop->left;
		axis = '-';
		break;
	}
	for (int x = 0, y = 1; y < self_varray->length; x++, y++) {
		v3 = get(self_varray, x);
		v4 = get(self_varray, y);
		for (int i = 0, j = 1; j < prop_varray->length; i++, j++) {
			v1 = get(prop_varray, i);
			v2 = get(prop_varray, j);
			if (line_cross(v1, v2, v3, v4, axis)) return true;
		}
	}
	return false;
}

COLLISION_SIDE collision(Prop* self, Prop* prop) {
	/*
	Check, wthether 2 props collides, return collision side as const char* . Can by "up", "down", "left", "right"
	*/
	//for (Vertex* vertex1 = prop1->upper.vertexes, *vertex2 = prop2->upper.vertexes + 1;)
	Vertex* pivot, *v1, *v2;
	bool collide = true;
	if (self == NULL || prop == NULL) return NOCOLLIDE;
	//char* sides 
	
	return NOCOLLIDE;
}
