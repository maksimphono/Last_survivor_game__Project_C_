
typedef enum { VERTEX, VECTOR, VECTORARR, VARRAY, PROP } PHISICS_TYPE;
typedef enum { CIRCLE, FUNCTION, BONES } COLLISION_TYPE;

typedef struct {
	/*
	Abstract structure, that represents collision Vertex
	*/
	int X;
	int Y;
} Vertex;

typedef struct Vector {
	/*
	Abstract structure, that represents a geometric vector
	*/
	Vertex* p1;
	Vertex* p2;
	double length;
	double plain[2];
	Vector* next;
	Vector* prev;
} Vector;

typedef struct {
	/*
	Array of vectors
	*/
	Vector* vectors;
	unsigned length;
} VectorArr;

typedef struct Prop {
	/*
	Abstract structure, that represents phisics object-> It has 4 vector arrays, that represents
	upper, lower, left and right collision Vectors. 'center' represents the point at the center of the object->
	*/
	COLLISION_TYPE collision_type; // Prop's collision model can be represented in certain ways: 
	//	1) CIRCLE (most quick check, because i just check distance to the center of the target prop)
	//	2) FUNCTION (i just have to check, if collision model's graphic or lines cross target prop's graphic)
	//	3) LINES (most slow to check, must check if at least one line of pivot prop cross ta least 1 target prop's line)

	union {
		struct {
			VectorArr upper;
			VectorArr lower;
			VectorArr left;
			VectorArr right;
		};
		unsigned collide_radius;
	};
	Vertex* center;
	bool nocollide;
} Prop;

/*
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

	static Prop* self;
	self = (Prop*)malloc(sizeof(Prop));
	Vertex* vertexes = (Vertex*)malloc(sizeof(Vertex) * MAX_VERTEX_ARR_LEN);
	self->upper = *init_vectorarr(0);
	self->lower = *init_vectorarr(0);
	self->left = *init_vectorarr(0);
	self->right = *init_vectorarr(0);
	VectorArr* arrays[4] = {&self->upper, &self->lower, &self->left, &self->right};
	int* list_x[4] = { upper_x, lower_x, left_x, right_x };
	int* list_y[4] = { upper_y, lower_y, left_y, right_y };
	int lengths[4] = { upper_len, lower_len, left_len, right_len };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < lengths[i]; j++)
			addVector(arrays[i], init_vector(list_x[i][j], list_y[i][j]));
	}
	return self;
}
*/