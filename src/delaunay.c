/*

    Creates a Delaunay triangulation from a set of points
	
    Copyright (C) 2009 - Sebhz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "util.h"
#include "delaunay.h"

static point_t box[4];

void update_neighborhood(triangle_t *t1, triangle_t *t2)
{
	int cnt_t1 = 0, cnt_t2 = 0, cnt = 0, i, j;

	for (i=0; i<3; i++) {
		for (j=0; j <3; j++) {
			if (t1->p[i] == t2->p[j]) {
				cnt++;
				cnt_t1+=i;
				cnt_t2+=j;
			}
		}
	}

	if (cnt == 2) {
		t1->t[3-cnt_t1] = t2;
		t2->t[3-cnt_t2] = t1;
	}
}

void recompute_neighborhood(tl_elt *triangulation, triangle_t *t)
{
	tl_elt *tmp;
	
	if (t == NULL) return;

	tmp = triangulation;
	while (tmp != NULL) {
		if (tmp->t != t) update_neighborhood(tmp->t, t);
		tmp = tmp->n;
	}
}

triangle_t *create_triangle(point_t *p0, point_t *p1, point_t *p2)
{
	triangle_t *t;
	int i;
	double v = v_product(p0, p1, p2);
	
	if (v == 0) /* Not a true triangle */
		return NULL;
	
	if ((t = (triangle_t *)malloc(sizeof(triangle_t))) == NULL ) {
		fprintf(stderr, "Unable to allocate triangle\n");
		exit(EXIT_FAILURE);
	}
	
	/* Make sure the triangle is direct */
	t->p[0] = p0;
	if (v < 0) {
		t->p[1] = p1;
		t->p[2] = p2;
	}
	else {
		t->p[1] = p2;
		t->p[2] = p1;
	}
	
	set_circumcircle(t);
	for (i=0; i<3; i++) t->t[i] = NULL;

	return t;
}

void debug_triangle(triangle_t *t)
{
	int i;
	if (t==NULL) {
		fprintf(stderr, "NULL\n");
		return;
	}
	
	for (i=0; i <3; i++) {
		fprintf(stderr, "sommet (%f,%f) (0x%08x) -> 0x%08x\n", t->p[i]->x, t->p[i]->y, (int)t->p[i], (int)t->t[i]);
	}
	fprintf(stderr, "circumcircle center (%f,%f) - radius %.3f\n", t->o.x, t->o.y, t->r);
}

void debug_list(tl_elt *list)
{
	tl_elt *tmp = list;
	int i = 0;
	
	fprintf(stderr, "Going through list 0x%08x\n", (int)list);
	while (tmp != NULL) {
		fprintf(stderr, "Current: 0x%08x - Previous: 0x%08x - Next: 0x%08x - Triangle: 0x%08x\n",
						 (int)tmp, (int)tmp->p, (int)tmp->n, (int)tmp->t);
		debug_triangle(tmp->t);
		tmp = tmp->n;
		i++;
	}
	fprintf(stderr, "%d triangles in triangulation\n", i);
}

/* Adds a triangle to a triangle linked list (at the beginning)  - create the list if the list pointer is NULL */
tl_elt *add_triangle(tl_elt *list, triangle_t *t)
{
	tl_elt *new;
	
	if ((new = (tl_elt *)malloc(sizeof(tl_elt))) == NULL) {
		fprintf(stderr, "Out of memory when trying to add triangle to list.\n");
		exit(EXIT_FAILURE);
	}

	new->p = NULL;
	new->n = list;
	new->t = t;

	if (list != NULL) list->p = new;
	
	return new;
}

tl_elt *remove_elt_containing_triangle(tl_elt *list, triangle_t *t)
{
	tl_elt *tle = list, *tmp, *nxt = list->n;

	while (tle->t != t) {
		if (tle->n == NULL) {
			fprintf(stderr, "Cannot find triangle in the list when trying to remove it.\n");
			exit(EXIT_FAILURE);
		}
		tle = tle->n;
	}
	
	if (tle->n != NULL) {
		tle->n->p = tle->p;
	}
	if (tle->p != NULL) {
		tle->p->n = tle->n;
	}

	free(t);
	tmp = tle->p;
	free(tle);
	
	if (tmp == NULL) return nxt; /*We removed the first element of the list */
	return list;
}

void destroy_list(tl_elt *list)
{
	tl_elt *tle = list;
	
	if (list == NULL) return;
	
	while (tle->n != NULL) {
		tle = tle->n;
		free(tle->p->t);
		free(tle->p);
	} 
	free(tle->t);
	free(tle);
}

tl_elt *create_box(int w, int h) {
	triangle_t *t[2];
	tl_elt *list = NULL;
	double d;
	int i;

	for (i=0; i < 2; i++) {
		if ((t[i] = (triangle_t *)malloc(sizeof(triangle_t))) == NULL) {
			fprintf(stderr, "Out of memory when trying create initial box triangles.\n");
			exit(EXIT_FAILURE);
		}
	}

	box[0].x = 0; box[0].y = 0; box[1].x = w; box[1].y = 0; box[2].x = 0; box[2].y = h; box[3].x = w; box[3].y = h;
	t[0]->p[0] = &(box[0]); t[0]->p[1] = &(box[1]); t[0]->p[2] = &(box[2]);
	t[1]->p[0] = &(box[2]); t[1]->p[1] = &(box[1]); t[1]->p[2] = &(box[3]);
	t[0]->t[0] = t[1];      t[0]->t[1] = NULL;      t[0]->t[2] = NULL;
	t[1]->t[0] = NULL;      t[1]->t[1] = NULL;      t[1]->t[2] = t[0];
	t[0]->o.x = w/2; t[0]->o.y = h/2; t[1]->o.x = w/2; t[1]->o.y = h/2;
	d = (sqrt(w*w + h*h))/2; t[0]->r = d; t[1]->r = d;
	
	for (i=0; i<2; i++)
		list = add_triangle(list, t[i]);
	
	return list;
}

int find_opposite_side(triangle_t *src, triangle_t *dst)
{
	int i;
	
	for (i=0; i < 3; i++)
		if (src->t[i] == dst) return i;
	
	return -1;
}

void remove_neighborhood(triangle_t *t)
{
	int summit, i;
	triangle_t *t_tmp;
	
	for (i=0; i<3; i++) {
		t_tmp = t->t[i];
		if (t->t[i] != NULL) {
			summit = find_opposite_side(t_tmp, t);
			t_tmp->t[summit] = NULL;
		}
	}
}

triangle_t *get_triangle_containing_p(tl_elt *triangulation, point_t *p) {
	tl_elt *t_tmp = triangulation;
	triangle_t *t = NULL;
	
	while (t_tmp != NULL) {
		if (check_inclusion(p, t_tmp->t)) {
			t = t_tmp->t;
			break;
		}
		t_tmp = t_tmp->n;
	}

	return t;
}

int is_summit(point_t *p, triangle_t *t)
{
	int i, found = 0;

	if (t == NULL) return 0;
	
	for (i=0; i <3; i++)
		if (t->p[i] == p) {
			found = 1;
			break;
		}
	
	return found;
}

tl_elt *flip_graph(tl_elt *triangulation, triangle_t *t, point_t *p) {
	triangle_t *t1, *t2, *t_neighbor;
	int found = 0, summit_t;

	if (t == NULL) return triangulation;  /* On the edge of the graph */
	if (euclidian_distance(p, &(t->o)) >= t->r) return triangulation; /* Point is not in the circumcenter */
	
	/* OK p is inside t circumcenter. Find out which neighbor of t contains p */
	for(summit_t=0; summit_t<3; summit_t++) {
		if (is_summit(p, t->t[summit_t])) {
			found = 1;
			break;
		}
	}
	if (found == 0) {
		exit(EXIT_FAILURE);
	}
	
	t_neighbor = t->t[summit_t];
	
	/* OK, now we know which summits are P and opposite to P - populate both new triangles... */
	t1 = create_triangle(p, t->p[summit_t], t->p[(summit_t+1)%3]);
	t2 = create_triangle(p, t->p[summit_t], t->p[(summit_t+2)%3]);
	/* ...add them to the triangulation.. .*/
	if (t1 != NULL) triangulation = add_triangle(triangulation, t1);
	if (t2 != NULL) triangulation = add_triangle(triangulation, t2);

	/* ... remove old triangles... */
	remove_neighborhood(t);
	remove_neighborhood(t_neighbor);
	triangulation = remove_elt_containing_triangle(triangulation, t);
	triangulation = remove_elt_containing_triangle(triangulation, t_neighbor);

	/* ...recompute adjacence */
	recompute_neighborhood(triangulation, t1);
	recompute_neighborhood(triangulation, t2);

	/* ... and check the neighbors of the two new triangles (opposite to p) */
	if (t1 != NULL) triangulation = flip_graph(triangulation, t1->t[0], p);
	if (t2 != NULL) triangulation = flip_graph(triangulation, t2->t[0], p);

	return triangulation;
}

tl_elt *split_triangle(tl_elt *triangulation, triangle_t *t, point_t *p) {
	triangle_t *triangles[6] = { NULL, NULL, NULL, NULL, NULL, NULL }, *u = NULL;
	int i, c, cf = 0;
	
	for (i=0; i<3; i++) {
		triangles[i] = create_triangle(p, t->p[i], t->p[(i+1)%3]);
		if (triangles[i] != NULL) {
			triangulation = add_triangle(triangulation, triangles[i]);
		}
		else { /* Colinearity detected */
			c = (i+2)%3;
			u = t->t[c];
			cf = 1;
		}
	}

	remove_neighborhood(t);
	triangulation = remove_elt_containing_triangle(triangulation, t);

	if (cf) {
		if (u != NULL) {
			for (i=3; i < 6; i++) {
				triangles[i] = create_triangle(p, u->p[i-3], u->p[(i-2)%3]);
				if (triangles[i] != NULL) {
					triangulation = add_triangle(triangulation, triangles[i]);
				}
			}
		remove_neighborhood(u);
		triangulation = remove_elt_containing_triangle(triangulation, u);
		}
	}
	
	for (i=0; i < 6; i++)
		recompute_neighborhood(triangulation, triangles[i]);
	
	for (i=0; i < 6; i++) {
		if (triangles[i] != NULL)
			triangulation = flip_graph(triangulation, triangles[i]->t[0], p);
	}
		
	return triangulation;
}

void check_delaunay(tl_elt *triangulation, point_t *cloud, int n)
{
	tl_elt *tmp = triangulation;
	int i, res = 0;
	double x1, x2, df;

	while (tmp != NULL)
	{
		for (i=0; i<n; i++)
		{
			x1 = euclidian_distance(cloud+i, &(tmp->t->o));
			x2 = tmp->t->r;
			df = x2-x1;
			if (df > 0.00000001)
			{
				fprintf(stderr, "Error while checking delaunay triangulation. Point %d - distance %.12f - radius of current circle: %.12f - difference %.12f\n", i, x1, x2, df);
				res = 1;
			}
		}
		tmp = tmp->n;
	}
	if (res) {
		for (i=0; i < n; i++) {
			fprintf  (stderr, "Point %d: (%.3f, %.3f)\n", i, (cloud+i)->x, (cloud+i)->y);
		}
		debug_list(triangulation);
		exit(EXIT_FAILURE);
	}			
}

/* Return the  number of points of t in the enclosing box. s contains the number of the first summit 
	in direct direction */
int number_of_points_in_box(triangle_t *t, int w, int h, int *s) {
	int i, sum = 0, summit=0;
	double x, y;
	
	for (i=0; i<3; i++)
	{
		x = t->p[i]->x; y = t->p[i]->y;
		if ( ((x == 0) || (x == w)) && ((y == 0) || (y == h)) ) {
			sum++;
			summit = i;
		}
	}
	
	if (sum == 1) *s = summit;
	if (sum == 2) {
		x = t->p[1]->x; y = t->p[1]->y;
		if ( ((x == 0) || (x == w)) && ((y == 0) || (y == h)) ) {
			*s = summit-1;
		}
		else {
			*s = 2;
		}
	}
	return sum;
}

tl_elt *remove_box(tl_elt *triangulation, int w, int h, int n) {
	tl_elt     *t_tmp = NULL,    *to_remove_list = NULL, *t_tmpp = NULL;
	triangle_t *t_origin = NULL, *t_current = NULL;
	int  s, p;
	
	/* First find one triangle in the triangulation with only one point on the box */
	t_tmp = triangulation;
	while (t_tmp != NULL) {
		p = number_of_points_in_box(t_tmp->t, w, h, &s);
		if (p == 1) {
			t_origin = t_tmp->t;
			break;
		}
		t_tmp = t_tmp->n;
	}
	if (t_origin == NULL) {
		fprintf(stderr, "Unable to find triangle with one single point in box\n");
		exit(EXIT_FAILURE);
	}
	
	to_remove_list = add_triangle(to_remove_list, t_origin);
	t_current = t_origin->t[(s+1)%3];
	
	/* Next, iterate on all the triangles of the border */
	while (t_current != t_origin) {
		to_remove_list = add_triangle(to_remove_list, t_current);

		n = number_of_points_in_box(t_current, w, h, &s);
		t_current = t_current->t[(s+1)%3];
		
		if (t_current == NULL) {
			fprintf(stderr, "Internal error while removing triangle from box\n");
			debug_list(to_remove_list);
			exit(EXIT_FAILURE);
		}
	}
	
	/* Cleanup */
	t_tmp = to_remove_list;
	while (t_tmp != NULL) {
		remove_neighborhood(t_tmp->t);
		t_tmp = t_tmp->n;
	}

	t_tmp = to_remove_list;
	while (t_tmp != NULL) {
		triangulation = remove_elt_containing_triangle(triangulation, t_tmp->t);
		t_tmpp = t_tmp;
		t_tmp = t_tmp->n;
		free(t_tmpp);
	}

	return triangulation;
}

tl_elt *create_delaunay_triangulation(point_t *cloud, int n, int w, int h) {
	tl_elt *triangulation = NULL;
	triangle_t *t_tmp;
	point_t *p;
	int i;
	
	triangulation = create_box(w, h);
	for (i=0; i <n; i++) {
		p = cloud+i;
		t_tmp = get_triangle_containing_p(triangulation, p);
		if (t_tmp == NULL) {
			fprintf(stderr, "Unable to find a triangle in the triangulation containing p (0x%08x, %f, %f)\n", (int)p, p->x, p->y);
			exit(EXIT_FAILURE);
		}
		triangulation = split_triangle(triangulation, t_tmp, p);
	}
	check_delaunay(triangulation, cloud, n);
	return triangulation;
}
