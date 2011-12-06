#pragma once

#define POLY_MAX_ORDER 5

typedef struct _tag_poly {
	int order;
	float a[POLY_MAX_ORDER + 1];
} poly;

poly poly_new(float a0, float a1, float a2, float a3, float a4, float a5);
float poly_eval(const poly const * const this, float x);
poly poly_derive(poly p);
