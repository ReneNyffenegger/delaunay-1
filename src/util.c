/*
    Various util functions dealing with geometry and triangles
    Copyright (C) 2009 Sebhz

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

double euclidian_distance(point_t *A, point_t *B)
{
	return sqrt( (B->x - A->x)*(B->x - A->x) + (B->y - A->y)*(B->y - A->y) );
}

static point_t get_circumcircle_center(triangle_t *t)
{
	double Xb, Xc, Yb, Yc, beta, gamma;
	point_t O;
	
	Xc = t->p[1]->x - t->p[0]->x; Yc = t->p[1]->y - t->p[0]->y;
	Xb = t->p[2]->x - t->p[0]->x; Yb = t->p[2]->y - t->p[0]->y;
	beta  = -Xb*(t->p[0]->x+t->p[2]->x)/2 - Yb*(t->p[0]->y+t->p[2]->y)/2;
	gamma = -Xc*(t->p[0]->x+t->p[1]->x)/2 - Yc*(t->p[0]->y+t->p[1]->y)/2;
	
	O.y = (beta*Xc-gamma*Xb)/(Yc*Xb-Xc*Yb);
	O.x = (gamma*Yb-beta*Yc)/(Yc*Xb-Xc*Yb);

	return O;
}

void set_circumcircle(triangle_t *t)
{
	point_t o = get_circumcircle_center(t);
	t->o.x = o.x;
	t->o.y = o.y;
	t->r = euclidian_distance(t->p[0], &o);
}

double v_product(point_t *p1, point_t *p2, point_t *p3)
{
	double v = (p3->x - p1->x)*(p2->y - p1->y) - (p2->x - p1->x)*(p3->y - p1->y);
	return v;
}

int direct_direction(point_t *p1, point_t *p2, point_t *p3)
{
	double v = v_product(p1, p2, p3); 

	if (v < 0) return 1;
	return 0;
}

int is_inside_segment (point_t *p0, point_t *p1, point_t *p2)
{
	if (v_product(p0, p1, p2) != 0) return 0;
	if ((p1->x < p2->x) && (p0->x >= p1->x) && (p0->x <= p2->x)) return 1;
	if ((p1->x > p2->x) && (p0->x <= p1->x) && (p0->x >= p2->x)) return 1;
	if ((p1->y < p2->y) && (p0->y >= p1->y) && (p0->y <= p2->y)) return 1;
	if ((p1->y > p2->y) && (p0->y <= p1->y) && (p0->y >= p2->y)) return 1;

    return 0;
}

int check_inclusion(point_t *p, triangle_t *t)
{
	int d, i;
	
	/* Check for pathological cases */
	for (i=0; i<3; i++)
		if ( is_inside_segment (p, t->p[i], t->p[(i+1)%3]))
			return 1;
	
	d = direct_direction(t->p[0], t->p[1], p);
	if ((d == direct_direction(t->p[0], p, t->p[2])) &&
		(d == direct_direction(t->p[1], t->p[2], p)) )
		return 1;
	
	return 0;
}

