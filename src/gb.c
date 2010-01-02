#include <stdlib.h>
#include <stdio.h>
#include "global.h"
#include "quadedge.h"

static quadedge_t *starting_edge = NULL;
static quadedge_t *delaunay_list = NULL;

typedef struct {
	int minx;
	int miny;
	int maxx;
	int maxy;
	point_t *a; /* Lower left */
	point_t *b; /* Lower right */
	point_t *c; /* Upper right */
	point_t *d; /* Upper left */
} bounding_box_t; 

static bounding_box_t bbox;

point_t *new_point(int x, int y) {
	point_t *p;

	if ( (p = (point_t *)malloc(sizeof(point_t))) == NULL ) {
		fprintf(stderr, "Unable to allocate point.\n");
		exit(EXIT_FAILURE);
	}

	p->x = x;
	p->y = y;
	
	return p;
}

#define MAX_VALUE 10000
#define MIN_VALUE -10000

void init_bounding_box(void) {
	bbox.minx = MAX_VALUE;
	bbox.maxx = MIN_VALUE;
	bbox.miny = MAX_VALUE;
	bbox.maxy = MIN_VALUE;

	bbox.a = NULL;
	bbox.b = NULL;
	bbox.c = NULL;
	bbox.d = NULL;
}


void init_delaunay(void) {
	init_bounding_box();
	
	quadedge_t *ab = make_edge(bbox.a, bbox.b);
	quadedge_t *bc = make_edge(bbox.b, bbox.c);
	quadedge_t *cd = make_edge(bbox.c, bbox.d);
	quadedge_t *da = make_edge(bbox.d, bbox.a);
	splice(sym(ab), bc);
	splice(sym(bc), cd);
	splice(sym(cd), da);
	splice(sym(da), ab);

	starting_edge = ab;
}

void set_bounding_box(int minx, int miny, int maxx, int maxy) {
	bbox.minx = minx; bbox.maxx = maxx;
	bbox.miny = miny; bbox.maxy = maxy;

	int centerx = (minx+maxx)/2;
	int centery = (miny+maxy)/2;
	int x_min = (int)((minx-centerx-1)*10+centerx);
	int x_max = (int)((maxx-centerx+1)*10+centerx);
	int y_min = (int)((miny-centery-1)*10+centery);
	int y_max = (int)((maxy-centery+1)*10+centery);

	bbox.a->x = x_min; bbox.a->y = y_min;
	bbox.b->x = x_max; bbox.b->y = y_min;
	bbox.c->x = x_max; bbox.c->y = y_max;
	bbox.d->x = x_min; bbox.d->y = y_max;
}

#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)

void update_bounding_box(point_t *p) {
	int minx = min(bbox.minx, p->x);
	int maxx = max(bbox.maxx, p->x);
	int miny = min(bbox.miny, p->y);
	int maxy = max(bbox.maxy, p->y);
	set_bounding_box(minx, miny, maxx, maxy);
	fprintf(stderr, "Resized bounding box: %d %d %d %d\n", minx, miny, maxx, maxy);
}

quadedge_t *locate(point_t *p) {
	if (p->x < bbox.minx || p->x > bbox.maxx || p->y < bbox.miny || p->y > bbox.maxy) {
		update_bounding_box(p);
	}
	
	quadedge_t *e = starting_edge;
	point_t *d    = dest(e);

	while(1) {
		/* Duplicate point ? */
		if ( (p->x == e->orig->x) && (p->y == e->orig->y) ) return e;
		if ( (p->x == d->x)       && (p->y == d->y) )       return e;
		
		if (is_at_right_of(e, p))
			e = sym(e);
		else if (!is_at_right_of(onext(e), p))
			e = onext(e);
		else if (!is_at_right_of(dprev(e), p))
			e = dprev(e);
		else
			return e;
	}
}

void remove_quadedge(quadedge_t *q) {
	/* Remove quadedge from the linked list */
}

void add_quadedge(quadedge_t *q) {
	/* Add quadedge to the linked list */
}

void insert_point (point_t *p) {
	quadedge_t *e = locate(p);
	point_t *d    = dest(e);

	if ( (p->x == e->orig->x) && (p->y == e->orig->y) ) return;
	if ( (p->x == d->x)       && (p->y == d->y) )       return;
	
	/* Point is on an existing edge -> remove the edge */
	if (is_on_line(e, p)) {
		e = oprev(e);
		remove_quadedge(sym(onext(e))); /* Find the definition of quadedge_remove */
		remove_quadedge(onext(e));
		delete_edge(onext(e));
	}

	/* Connect the new point to the vertices of the containing triangle
	   (or quadrilateral in case the point is on an existing edge */
	quadedge_t *base = make_edge(e->orig, p);
	add_quadedge(base);

	splice(base, e);
	starting_edge = base;

	do {
		quadedge_t *t = oprev(e);

		if (is_at_right_of(e, dest(t)) && incircle(e->orig, dest(t), dest(e), p) ) {
			swap_edge(e);
			e = oprev(e);
		}
		else if (onext(e) == starting_edge)
			return;
		else {
			quadedge_t *f = onext(e);
			e = lprev(f);
		}
	} while (1);
}

