typedef struct {
	Entity* parent;
	int grow_time;
} Plant;

Plant* init_plant(int x, int y, int time) {
	static Plant* self;
	self = (Plant*)malloc(sizeof(Plant));
	self->parent = registerEntity(x, y, "Plant", plant_png, NULL, NULL, "");
	self->parent->child = (void*)self;
	self->grow_time = time;
	return self;
}