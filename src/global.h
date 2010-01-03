typedef struct 
{
	double x;
	double y;
} point_t;

struct triangle_s {
	point_t *p[3];
	struct triangle_s *t[3];
	point_t o; /* Circumcircle center */
	double  r;
};
typedef struct triangle_s triangle_t;
