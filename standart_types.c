#include <stdlib.h>
#pragma warning(disable: 4996)

const int MAX_ARRAY_LEN = 99;

typedef char string[MAX_ARRAY_LEN];
typedef enum {INTEGER, CHARACTER, DBLE, STR} STANDART_TYPE;

typedef struct {
	union {
		int iarray[MAX_ARRAY_LEN];
		char carray[MAX_ARRAY_LEN];
		double darray[MAX_ARRAY_LEN];
		long larray[MAX_ARRAY_LEN];
		string sarray[MAX_ARRAY_LEN];
	};
	unsigned length;
	STANDART_TYPE type;
} Array;

Array* init_array(STANDART_TYPE type, unsigned len) {
	static Array* self;
	self = (Array*)malloc(sizeof(Array));
	self->length = 0;
	self->type = type;
	return self;
}

void append(Array* self, ...) {
	va_list argument;
	va_start(argument, self);
	switch (self->type) {
	case INTEGER:
		self->iarray[self->length++] = va_arg(argument, int);
		break;
	case CHARACTER:
		self->carray[self->length++] = va_arg(argument, char);
		break;
	case DBLE:
		self->darray[self->length++] = va_arg(argument, double);
		break;
	case STR:
		string new_str;
		strcpy(self->sarray[self->length++], va_arg(argument, string));
		break;
	}
	va_end(argument);
}