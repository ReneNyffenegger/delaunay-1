/*
    Test of various algorithms dealing with geometry

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
#include <time.h>
#include <sys/time.h>
#include "global.h"
#include "util.h"
#include "delaunay.h"
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_gfxPrimitives.h"

#define SCREEN_W 640
#define SCREEN_H 480

const char *WINDOW_TITLE = "Geometry";

void
apply_surface (int x, int y, SDL_Surface * source, SDL_Surface * destination)
{
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    SDL_BlitSurface (source, NULL, destination, &offset);
}

void display_triangle(triangle_t *t, SDL_Surface *screen)
{
	int i;
	for (i=0; i < 3; i++) {
		lineRGBA(screen, t->p[i]->x, t->p[i]->y, t->p[(i+1)%3]->x, t->p[(i+1)%3]->y,
		         255, 255, 255, SDL_ALPHA_OPAQUE);
	}
}

int is_duplicate (point_t *cloud, int n)
{
	int i;
	
	for (i=0; i < n; i++)
	{
		if ((cloud[i].x == cloud[n].x) && (cloud[i].y == cloud[n].y))
			return 1;
	}
	return 0;
}

void test_delaunay(SDL_Surface *screen, int n)
{
 	point_t *cloud;
	tl_elt *triangulation, *tmp;
	int i;

    SDL_FillRect (SDL_GetVideoSurface (), NULL, 0);

	if ( (cloud = (point_t*)malloc(n*sizeof(point_t))) == NULL) {
		fprintf(stderr, "Unable to get memory for point cloud.\n");
		exit(EXIT_FAILURE);
	}

	for (i=0; i<n; i++) {
		do {
			cloud[i].x = 10 + rand()%(screen->w-11);
			cloud[i].y = 10 + rand()%(screen->h-11);
		}
		while (is_duplicate(cloud, i));
	}

	triangulation = create_delaunay_triangulation(cloud, n, screen->w, screen->h);
	tmp = triangulation;
	while(tmp != NULL)
	{
		display_triangle(tmp->t, screen);
		tmp = tmp->n;
	}
	
	for (i=0; i<n; i++) {
		circleRGBA(screen, cloud[i].x, cloud[i].y, 1, 255, 0, 0, SDL_ALPHA_OPAQUE);
	}
	
	destroy_list(triangulation);
	free(cloud);
}

int main(int argc, char **argv)
{
    int i;
	SDL_Surface *screen;

    srand (time (NULL));
    SDL_Init (SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode (SCREEN_W, SCREEN_H, 0,
                                            SDL_HWSURFACE | SDL_DOUBLEBUF |
                                            SDL_RESIZABLE);
    SDL_WM_SetCaption (WINDOW_TITLE, 0);
	
	for (i=0; i < 32; i++) {
		test_delaunay(screen, 256);
		SDL_Flip (screen);
		SDL_Delay(100);
	}		
    
	SDL_Quit ();

	return EXIT_SUCCESS;
}
