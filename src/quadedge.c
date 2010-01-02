#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "quadedge.h"

/* Getters */
quadedge_t *onext(quadedge_t *q) {
	return(q->onext);
}

quadedge_t *rot(quadedge_t *q) {
	return(q->dual);
}

point_t *orig(quadedge_t *q) {
	return(q->orig);
}
 
/* Navigation */
/* Symetric (reverse) quadedge */
quadedge_t *sym(quadedge_t *q) {
	return rot(q->dual);
}

/* Other extremity */
point_t *dest(quadedge_t *q) {
	return orig(sym(q));
}

/* Symetric dual */
quadedge_t *rotsym(quadedge_t *q) {
	return sym(q->dual);
}

/* Previous quadedge (pointing to q->orig) */
quadedge_t *oprev(quadedge_t *q) {
	return rot(onext(q->dual));
} 

/* Previous quadedge starting from dest() */
quadedge_t *dprev(quadedge_t *q) {
	return rotsym(onext(rotsym(q)));
}

/* Next quadedge on left face */
quadedge_t *lnext(quadedge_t *q) {
	return rot(onext(rotsym(q)));
}

/* Precious quadedge on left face */
quadedge_t *lprev(quadedge_t *q) {
	return sym(onext(q));
}

/* Constructor */
quadedge_t *new_quadedge(quadedge_t *onext, quadedge_t *dual, point_t *orig) {
	quadedge_t *q;

	if ( (q = (quadedge_t *)malloc(sizeof(quadedge_t))) == NULL) {
		fprintf(stderr, "Unable to allocate memory for quadedge element\n");
		exit(EXIT_FAILURE);
	}

	q->orig = orig;
	q->dual = dual;
	q->onext = onext;

	return q;
}

quadedge_t *make_edge(point_t *orig, point_t *dest) {
	quadedge_t *q0, *q1, *q2, *q3;

	q0 = new_quadedge(NULL, NULL, orig);
	q1 = new_quadedge(NULL, NULL, NULL);
	q2 = new_quadedge(NULL, NULL, dest);
	q3 = new_quadedge(NULL, NULL, NULL);

	/* Create the segment */
	q0->onext = q0; q2->onext = q2; /* Single segment -> no next quadedge */
	q1->onext = q3; q3->onext = q1; /* in the dual space -> two adjacent faces */

	/* Dual switch */
	q0->dual = q1; q1->dual = q2; 
	q2->dual = q3; q3->dual = q0;

	return q0;
}

void splice(quadedge_t *a, quadedge_t *b) {
	quadedge_t *alpha = rot(onext(a));
	quadedge_t *beta  = rot(onext(b));

	quadedge_t *t1    = onext(b);
	quadedge_t *t2    = onext(a);
	quadedge_t *t3    = onext(beta);
	quadedge_t *t4    = onext(alpha);

	a->onext = t1;
	b->onext = t2;
	alpha->onext = t3;
	beta->onext  = t4;
} 

quadedge_t *connect_quadedge(quadedge_t *e1, quadedge_t *e2) {
	quadedge_t *q = make_edge(dest(e1), orig(e2));

	splice(q, lnext(e1));
	splice(sym(q), e2);
	
	return q;
}

void swap_edge(quadedge_t *e) {
	quadedge_t *a = oprev(e);
	quadedge_t *b = oprev(sym(e));
	quadedge_t *c;

	splice(e, a);
	splice(sym(e), b);
	splice(e, lnext(a));
	splice(sym(e), lnext(b));
	e->orig = dest(a);
	c = sym(e);
	c->orig = dest(b);
}

void delete_edge(quadedge_t *q) {
	splice(q, oprev(q));
	splice(sym(q), oprev(sym(q)));

	free(q);
}

int is_on_line(quadedge_t *e, point_t *p) {
	if ((p->x-e->orig->x)*(p->y-dest(e)->y) == (p->y-e->orig->y)*(p->x-dest(e)->x))
		return 1;
	return 0;
}

int is_counter_clockwise(point_t *a, point_t *b, point_t *c) {
	if ( (a->x - b->x)*(b->y - c->y) > (a->y - b->y)*(b->x - c->x) ) 
		return 1;
	return 0;
}

int is_at_right_of(quadedge_t *q, point_t *p) {
	return is_counter_clockwise(p, dest(q), q->orig);
}

static double det33(m0, m1, m2, m3, m4, m5, m6, m7, m8)
{
	double det33 = 0;

	det33 += m0 * (m4*m8 - m5*m7);
	det33 -= m1 * (m3*m8 - m5*m6);
	det33 += m2 * (m3*m7 - m4*m6);

	return det33;
}

/* Tests if point d is inside the circumcenter of triangle a, b, c
		 if "d" is strictly INSIDE the circle, then
 
		     |d² dx dy 1||
             |a² ax ay 1||
		 det |b² bx by 1|  <  0
		     |c² cx cy 1|
 
	There is probably a faster way to implement this test */
int incircle(point_t *a, point_t *b, point_t *c, point_t *d) {
	double a2 = a->x*a->x + a->y*a->y;
	double b2 = b->x*b->x + b->y*b->y;
	double c2 = c->x*c->x + c->y*c->y;
	double d2 = d->x*d->x + d->y*d->y;

	double det44 = 0;
	det44 += d2   * det33( a->x, a->y,    1, b->x, b->y,    1, c->x, c->y,    1);
	det44 -= d->x * det33( a2,   a->y,    1, b2,   b->y,    1, c2,   c->y,    1);
	det44 += d->y * det33( a2,   a->x,    1, b2,   b->x,    1, c2,   c->x,    1);
	det44 -= 1    * det33( a2,   a->x, a->y, b2,   b->x, b->y, c2,   c->y, c->y);

	if (det44 < 0) return 1;
	return 0;
}
