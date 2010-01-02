struct quadedge_s {
	struct quadedge_s *onext; /* next (direct order) quadedge */
	struct quadedge_s *dual;  /* dual quadedge (face graph <-> edge graph) */
	point_t    *orig;  /* Origin point of the edge/face */
};

typedef struct quadedge_s quadedge_t;

/* Getters */
quadedge_t *onext(quadedge_t *q);
quadedge_t *rot(quadedge_t *q);
point_t *orig(quadedge_t *q);
 
/* Navigation */
/* Symetric (reverse) quadedge */
quadedge_t *sym(quadedge_t *q);

/* Other extremity */
point_t *dest(quadedge_t *q);

/* Symetric dual */
quadedge_t *rotsym(quadedge_t *q);

/* Previous quadedge (pointing to q->orig) */
quadedge_t *oprev(quadedge_t *q);

/* Previous quadedge starting from dest() */
quadedge_t *dprev(quadedge_t *q);

/* Next quadedge on left face */
quadedge_t *lnext(quadedge_t *q);

/* Previous quadedge on left face */
quadedge_t *lprev(quadedge_t *q);

/* Constructor */
quadedge_t *new_quadedge(quadedge_t *onext, quadedge_t *dual, point_t *orig);

quadedge_t *make_edge(point_t *orig, point_t *dest);

void splice(quadedge_t *a, quadedge_t *b);

quadedge_t *connect_quadedge(quadedge_t *e1, quadedge_t *e2);

void swap_edge(quadedge_t *e);

void delete_edge(quadedge_t *q);

int is_on_line(quadedge_t *e, point_t *p);

int is_counter_clockwise(point_t *a, point_t *b, point_t *c);

int incircle(point_t *a, point_t *b, point_t *c, point_t *d);

int is_at_right_of(quadedge_t *q, point_t *p);
