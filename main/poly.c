#include <poly.h>
#include <util.h>

// make a new polynomial p(x) = sum_{n=0}^{4} a_n * x^n
poly poly_new(float a0, float a1, float a2, float a3, float a4, float a5) {
	poly p;

	p.a[0] = a0;
	p.a[1] = a1;
	p.a[2] = a2;
	p.a[3] = a3;
	p.a[4] = a4;
	p.a[5] = a5;

	for (p.order = POLY_MAX_ORDER; p.order >= 0; p.order--) {
		if (p.a[p.order] != 0) break;
	}

	return p;
}

// evaluate polynomial at x
float poly_eval(const poly const * const this, float x) {
	ASSERTNOTNULL(this);
	ASSERT(this->order <= POLY_MAX_ORDER, "order out of bounds %d", this->order);

	float rv = 0;
	float x2n = 1; // at^n

	for (int n = 0; n <= this->order; n++) {
		rv += this->a[n] * x2n;
		x2n *= x;
	}

	return rv;
}

// return the derivative of the polynomial
poly poly_derive(poly p) {
	ASSERT(p.order <= POLY_MAX_ORDER, "order out of bounds %d", p.order);

	for (int n = 1; n <= p.order; n++) {
		p.a[n - 1] = n * p.a[n];
	}
	p.a[p.order] = 0;
	p.order--;
	return p;
}
