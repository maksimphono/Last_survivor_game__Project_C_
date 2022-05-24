#ifndef GRAPHIC_TYPES
#define GRAPHIC_TYPES "graphic_types"
typedef unsigned long long ull;

LPCTSTR MAIN_BG_MODEL_PATH = L"assets\\main_bg_01.png";
LPCTSTR MAIN_CH_MODEL_PATH = L"assets\\test_transp.png";
LPCTSTR BONE_MODEL_PATH = L"assets\\bone.png";

typedef enum { FIGURE, IMG, ENTITY, ENTLIST, ENTNODE } GRAPHIC_TYPE;
typedef enum { ELLIPSE, LINE, RECTANGLE} PRIMITIVE_TYPE;
typedef enum { NONE, BONE, EFFECT, OBJECT, ITEM, MOB, WALL } ENTITY_TYPE;

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

typedef struct FigureArray {
	int length;
	int max_length;
	Figure** _;
}FigureArray;

typedef struct Entity {
	/*
	This is abstraction structure, that represents 'Entity'. It holds coordinates and figure, that figure will be rendered on canvas
	*/
	const char* name;
	ENTITY_TYPE type;
	Figure* figure;
	FigureArray bones;
	Prop* phis_model;
	int target[2];
	int vision_radius;
	int lower_edge;
	const char* (*loop_action)(Entity*, int);
	const char* (*collision_action)(Entity*, Entity*, int*, int*, COLLISION_SIDE);
	int center_x;
	int center_y;
	double move_axis;
	double distance_to_target;
	int X;
	int Y;
} Entity;

typedef struct EntArray {
	Entity* entities;
	int length;
} EntArray;

typedef struct EntityNode {
	ENTITY_TYPE type;
	Entity* object;
	EntityNode* prev;
	EntityNode* next;
	int lower_edge;
} EntityNode;

typedef struct EntityList {
	int length;
	EntityNode* head;
	EntityNode* tail;
} EntityList;

typedef struct GameField {
	Figure* background;
	Figure* background_source;
	LPCTSTR storage_file_path;
	DWORD* bg_code;
	DWORD* bg_code_src;
	GameField* upper;
	GameField* lower;
	GameField* left;
	GameField* right;
	EntityList* object_list;
} GameField;

GameField* workingGameField;

Entity* player;

GameField* getWorkingField() {
	return workingGameField;
}


EntityList* init_entlist();

EntityList killed_main_entity_list = *init_entlist();

EntityNode* appendEntNode(EntityList*, Entity*);
EntityNode* appendEntNode(EntityList*, EntityNode*);

void addEnt(Entity* ent) {
	appendEntNode(workingGameField->object_list, ent);
}

FigureArray* init_figarray(int length) {
	static FigureArray* self;
	self = (FigureArray*)malloc(sizeof(FigureArray));
	self->_ = (Figure**)malloc(sizeof(Figure*) * length);
	self->length = 0;
	self->max_length = length;
	return self;
}

Figure* popFigure(FigureArray* self) {
	if (self->length == 0) return NULL;
	Figure* figure = self->_[self->length - 1];
	self->_[--self->length] = NULL;
	return figure;
}

FigureArray killed_figure = *init_figarray(50);

Figure* init_figure(int x, int y, LPCTSTR path_to_image) {
	static Figure* self;
	if (killed_figure.length == 0) {
		//self = init_figure(x, y, path_to_image);
		self = (Figure*)malloc(sizeof(Figure));
	}
	else {
		self = popFigure(&killed_figure);
	}	
	IMAGE* img = setupImage(&self->img_index); // access to image in 'images' array
	if (img != NULL) {
		loadimage(img, path_to_image, 0, 0, true);
		self->width = img->getwidth();
		self->height = img->getheight();
		self->X = x;
		self->Y = y;
	}
	return self;
}

void kill_figure(Figure* self) {
	/*
	Destructor for 'Figure' structure
	*/
	empty_image_pos[empty_image_pos_len++] = self->img_index;
	killed_figure._[killed_figure.length++] = self;
}

int appendFigure(FigureArray* self, int x, int y, LPCTSTR path_to_image) {
	static Figure* new_fig;
	if (self->length >= self->max_length) {
		self->_ = (Figure**)realloc(self->_, sizeof(self->_) + sizeof(Figure*) * 10);
		self->max_length++;
	}
	new_fig = init_figure(x, y, path_to_image);
	self->_[self->length++] = new_fig;
	return self->length;
}

int clearFigureArr(FigureArray* self) {
	for (Figure** fig = self->_; fig != self->_ + self->length; fig++) {
		kill_figure(*fig);
	}
	self->length = 0;
	return 0;
}

Entity* init_entity(int x, int y, LPCTSTR path_to_image) {
	/*
	Constructor for 'Entity' structure
	*/
	static Entity* self;
	self = (Entity*)malloc(sizeof(Entity));
	self->figure = init_figure(x, y, path_to_image);
	self->bones = *init_figarray(8);
	//self->phis_model = *init_prop();
	self->type = OBJECT;
	self->X = x;
	self->Y = y;
	self->center_x = x + self->figure->width / 2;
	self->center_y = y + self->figure->height / 2;
	self->vision_radius = STANDART_VISION_RADIUS;
	self->lower_edge = y + self->figure->height;
	self->phis_model = NULL;
	self->loop_action = NULL;
	self->collision_action = NULL;
	return self;
}

void kill_entnode(EntityNode* self);

void kill_entity(Entity* self) {
	/*
	Removes EntityNode with that object
	*/
	for (EntityNode* node = workingGameField->object_list->head; node != NULL; node = node->next) {
		if (node->object == self) kill_entnode(node);
	}
}

Entity* reinit_entity(Entity* self, int x, int y, LPCTSTR path_to_image) {
	if (self == NULL) return NULL;
	self->name = "object";
	self->figure = init_figure(x, y, path_to_image);
	//self->phis_model = *init_prop();
	self->type = OBJECT;
	self->X = x;
	self->Y = y;
	self->center_x = x + self->figure->width / 2;
	self->center_y = y + self->figure->height / 2;
	self->vision_radius = STANDART_VISION_RADIUS;
	self->lower_edge = y + self->figure->height;
	self->collision_action = NULL;
	self->loop_action = NULL;
	self->phis_model = NULL;

	return self;
}

void del_entity(Entity* self) {
	/*
	Destructor for 'Entity' structure
	*/
	int len = 0;
	Entity** object = NULL;
	
	if (self->phis_model != NULL) del_prop(self->phis_model);
	kill_figure(self->figure);
	self = (Entity*)malloc(sizeof(Entity));

	free(self);
}

void setProp(Entity* self, Prop* prop) {
	/*
	Method, that creates new Prop, using given arrays and sets that prop as entity's phisics model
	*/
	if (self->phis_model != NULL) del_prop(self->phis_model);
	self->phis_model = prop;
}

void setTarget(Entity* self, const char* type, ...) {
	va_list argument;
	union {
		Entity* object;
		struct {
			int X;
			int Y;
		};
	};
	va_start(argument, type);
	if (type == "Points") {
		X = va_arg(argument, int);
		Y = va_arg(argument, int);
	}
	else if (type == "Object") {
		// rewrite !!
		object = va_arg(argument, Entity*);
		X = object->center_x;
		Y = object->center_y;
	}
	self->target[_X] = X;
	self->target[_Y] = Y;
	self->move_axis = (double)(self->target[_Y] - self->center_y) / (double)(self->target[_X] - self->center_x);
	self->distance_to_target = sqrt(pow(self->target[_Y] - self->center_y, 2) + pow(self->target[_X] - self->center_x, 2));

	va_end(argument);
}

void setAxis(Entity* self) {
	if (self->target[_X] < 0 || self->target[_Y] < 0) return;
}

EntityNode* animate_entnode();

EntityNode* init_entnode(Entity* object) {
	static EntityNode* self;
	self = (EntityNode*)malloc(sizeof(EntityNode));
	self->next = NULL;
	self->prev = NULL;
	self->type = object->type;
	self->lower_edge = object->lower_edge;
	self->object = object;
	return self;
}

EntityList* init_entlist() {
	static EntityList* self;
	self = (EntityList*)malloc(sizeof(EntityList));
	self->head = NULL;
	self->tail = self->head;
	self->length = 0;
	return self;
}

// GameField methods:

GameField* init_gamefield(LPCTSTR path_to_storage_file, LPCTSTR background_image_path) {
	static GameField* self;
	self = (GameField*)malloc(sizeof(GameField));
	self->object_list = init_entlist();
	self->background = init_figure(0, 0, background_image_path);
	self->background_source = init_figure(0, 0, background_image_path);
	self->storage_file_path = path_to_storage_file;
	self->bg_code = NULL;
	self->bg_code_src = NULL;
	self->upper = NULL;
	self->lower = NULL;
	self->left = NULL;
	self->right = NULL;
	return self;
}

void setNeighbours(GameField* self, GameField* upper, GameField* lower, GameField* left, GameField* right) {
	self->upper = upper;
	if (upper) upper->lower = self;
	self->lower = lower;
	if (lower) lower->upper = self;
	self->left = left;
	if (left) left->right = self;
	self->right = right;
	if (right) right->left = self;
}

void setupGameField(GameField* self) {
	workingGameField = self;
	self->bg_code = GetImageBuffer(&images[self->background->img_index]);
	self->bg_code_src = GetImageBuffer(&images[self->background->img_index]);

	//main_entity_list = *self->object_list;
}

// GameField methods /\

EntityNode* registerEntity(int x, int y, const char* name, LPCTSTR path_to_image, const char* (*action)(Entity*, int), const char* (*collision_action)(Entity*, Entity*, int*, int*, COLLISION_SIDE), const char* with_prop, ...) {
	/*
	Creates new entity, sets it on given cordinates, loads image by 'path_to_image'. If 'with_prop' is "Prop:", then
	takes 4 'VerexArr' objects, creates 'Prop' instance, using those arrays and sets created entity's 'phi_model'
	field as this new prop
	*/
	EntityNode* new_node;
	int X, Y, radius;

	if (killed_main_entity_list.length == 0) {
		new_node = init_entnode(init_entity(x, y, path_to_image));
		appendEntNode(workingGameField->object_list, new_node);
	}
	else {
		new_node = animate_entnode();
		reinit_entity(new_node->object, x, y, path_to_image);
		new_node->lower_edge = new_node->object->lower_edge;
	}
	new_node->object->loop_action = action;
	new_node->object->collision_action = collision_action;
	new_node->object->name = name;

	if (with_prop == "Prop:") {
		va_list vertex_arrs;
		va_start(vertex_arrs, with_prop);
		const char* type = va_arg(vertex_arrs, const char*);
		if (type == "BONES") {
			VectorArr up = va_arg(vertex_arrs, VectorArr);
			VectorArr down = va_arg(vertex_arrs, VectorArr);
			VectorArr left = va_arg(vertex_arrs, VectorArr);
			VectorArr right = va_arg(vertex_arrs, VectorArr);
			setProp(new_node->object, init_prop(BONES, up, down, left, right));
		} else if (type == "RADIUS"){
			radius = va_arg(vertex_arrs, unsigned);
			setProp(new_node->object, init_prop(RADIUS, x + new_node->object->figure->width / 2, y + new_node->object->figure->height / 2, radius));
			//X = va_arg(vertex_arrs, int);
			//Y = va_arg(vertex_arrs, int);
			
		}
		va_end(vertex_arrs);
		
	}
	else {
		new_node->object->phis_model = NULL;
	}
	return new_node;
}

EntityNode* popEntNode(EntityList* self, int index);

EntityNode* appendEntNode(EntityList* self, Entity* object) {
	EntityNode* new_node = NULL;
	new_node = init_entnode(object);
	if (self->length == 0) {
		self->head = new_node;
		self->head->prev = NULL;
	}
	else if (self->length == 1) {
		new_node->prev = self->head;
		self->head->next = new_node;
	}
	else {
		new_node->prev = self->tail;
		self->tail->next = new_node;
	}
	self->tail = new_node;
	self->tail->next = NULL;
	self->length++;
	return self->tail;
}

EntityNode* appendEntNode(EntityList* self, EntityNode* new_node) {
	if (self->length == 0) {
		self->head = new_node;
		self->head->prev = NULL;
	}
	else if (self->length == 1) {
		new_node->prev = self->head;
		self->head->next = new_node;
	}
	else {
		new_node->prev = self->tail;
		self->tail->next = new_node;
	}
	self->tail = new_node;
	self->tail->next = NULL;
	self->length++;
	return self->tail;
}

int insertEntNode(EntityList* self, EntityNode* node, int index) {
	/*
	Method, that insert element 'node' to list 'self' such way, that node's index will be 'index'
	Returns new list length
	*/
	node->prev = NULL;
	node->next = NULL;
	if (index > self->length) return -1;
	EntityNode* currentNode = self->head;
	if (index == 0) {
		node->next = self->head;
		self->head->prev = node;
		self->head = node;
	}
	else {
		index--;
		while (index-- > 0 && currentNode != NULL) currentNode = currentNode->next;
		node->next = currentNode->next;
		currentNode->next = node;
		node->prev = currentNode;
		if (node->next == NULL)
			self->tail = node;
		else
			node->next->prev = node;
	}
	return ++self->length;
}

void insertEntNode(EntityList* self, EntityNode* dest_node, EntityNode* new_node) {
	/*
	Method, that insert element 'node' to list 'self' such way, that node's index will be 'index'
	Returns new list length
	*/
	new_node->prev = NULL;
	new_node->next = NULL;
	if (dest_node == NULL) return;
	EntityNode* currentNode = self->head;
	if (dest_node->prev == NULL) {
		new_node->next = dest_node->next;
		dest_node->next = new_node;
		new_node->prev = dest_node;
		if (dest_node->next->next) dest_node->next->next->prev = new_node;
	}
	else {
		new_node->next = dest_node->next;
		dest_node->next = new_node;
		new_node->prev = dest_node;

		if (new_node->next == NULL)
			self->tail = new_node;
		else
			new_node->next->prev = new_node;
	}
	self->length++;
}

EntityNode* popEntNode(EntityList* self, int index) {
	/*
	Removes node on index 'index' from list 'self' and return it. If 'index' is -1, removes last node
	*/
	EntityNode* currentNode = NULL;
	EntityNode* previousNode = NULL;
	if (self->head == NULL) return NULL;
	if (index >= 0 and index >= self->length) return NULL;
	if (index == -1 || index == self->length - 1) {
		currentNode = self->tail;
		self->tail = self->tail->prev;
		if (self->tail) self->tail->next = NULL;
	}
	else if (index == 0) {
		currentNode = self->head;
		self->head = self->head->next;
		self->head->next = NULL;
		self->head->prev = NULL;
	}
	else {
		currentNode = self->head;
		while (index-- && currentNode != NULL) {
			previousNode = currentNode;
			currentNode = currentNode->next;
		}
		if (currentNode == self->head) self->head = self->head->next;
		if (previousNode->next == NULL || currentNode->next == NULL) return NULL;
		previousNode->next = currentNode->next;
		currentNode->next->prev = previousNode;
	}
	self->length--;
	return currentNode;
}

void removeEntNode(EntityList* self, EntityNode* node) {
	/*
	Removes node on index 'index' from list 'self' and return it. If 'index' is -1, removes last node
	*/
	EntityNode* currentNode = NULL;
	EntityNode* previousNode = NULL;
	if (self->head == NULL) return;
	if (self->length == 1) {
		self->head = self->tail = NULL;
	}
	else if (node->next == NULL) {
		currentNode = self->tail;
		self->tail = self->tail->prev;
		self->tail->next = NULL;
	}
	else if (node->prev == NULL) {
		currentNode = self->head;
		self->head = self->head->next;
		self->head->next = NULL;
		self->head->prev = NULL;
	}
	else {
		previousNode = node->prev;
		if (node == self->head) self->head = self->head->next;
		if (previousNode->next == NULL || node->next == NULL) return;
		previousNode->next = node->next;
		node->next->prev = previousNode;
	}
	self->length--;
}

void kill_entnode(EntityNode* self) {
	/*
	Removes node from active entity list and adds to killed entity list
	*/
	//del_prop(self->object->phis_model);
	self->object->phis_model = NULL;
	removeEntNode(workingGameField->object_list, self);
	appendEntNode(&killed_main_entity_list, self);
}

void kill_all_nodes(EntityList* list) {
	/*
	Kills all nodes in the list
	*/
	EntityNode* node = list->tail;
	EntityNode* prev_node = list->tail->prev;
	while (list->length > 0 && node != NULL) {
		prev_node = node->prev;
		kill_entnode(node);
		node = prev_node;
	}
}

EntityNode* animate_entnode() {
	/*
	Removes node from killed entity list and adds to active entity list
	*/
	return appendEntNode(workingGameField->object_list, popEntNode(&killed_main_entity_list, -1));
}

EntityNode* getEntNode(EntityList* self, int index) {
	EntityNode* node = self->head;
	if (index >= self->length || index < 0) return NULL;
	//index--;
	while (index--) node = node->next;
	return node;
}

void swapNode(EntityList* self, EntityNode* node_1, EntityNode* node_2) {
	EntityNode* prev_1 = node_1->prev, * prev_2 = node_2->prev, * next_1 = node_1->next, * next_2 = node_2->next, * temp1, * temp2;
	if (prev_1) prev_1->next = node_2;
	if (next_2) next_2->prev = node_1;
	if (prev_2) prev_2->next = node_1;
	if (next_1) next_1->prev = node_2;
	temp1 = node_1->next;
	temp2 = node_1->prev;

	node_1->next = node_2->next;
	node_1->prev = node_2->prev;
	node_2->next = temp1;
	node_2->prev = temp2;
	if (prev_1 == NULL)
		self->head = node_2;
	else if (prev_2 == NULL)
		self->head = node_1;
	if (next_1 == NULL)
		self->tail = node_2;
	else if (next_2 == NULL)
		self->tail = node_1;
}
#endif