struct triangle_list_elt_s {
	struct triangle_list_elt_s *p;
	struct triangle_list_elt_s *n;
	triangle_t *t;
};
typedef struct triangle_list_elt_s tl_elt;

tl_elt *create_triangulation(point_t *cloud, int n, int w, int h);
tl_elt *create_delaunay_triangulation(point_t *cloud, int n, int w, int h);
void destroy_list(tl_elt *list);

