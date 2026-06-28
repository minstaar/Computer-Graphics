#pragma once

#define BACKGROUND_COLOR_IDLE  245.0f / 255.0f, 245.0f / 255.0f, 240.0f / 255.0f
#define BACKGROUND_COLOR_CREATION  225.0f / 255.0f, 235.0f / 255.0f, 245.0f / 255.0f
#define BACKGROUND_COLOR_SELECTION  255.0f / 255.0f, 235.0f / 255.0f, 230.0f / 255.0f
#define BACKGROUND_COLOR_ANIMATION  253.0f / 255.0f, 253.0f / 255.0f, 225.0f / 255.0f
#define BACKGROUND_COLOR_BOUNCE  170.0f / 255.0f, 200.0f / 255.0f, 185.0f / 255.0f

#define POINT_COLOR  0.0f, 0.0f, 0.0f
#define CENTER_POINT_COLOR_UNSELECTED  0.0f, 0.0f, 0.0f
#define CENTER_POINT_COLOR_SELECTED  1.0f, 0.0f, 0.0f

#define ROTATION_STEP 16 // ms

#define SCALE_UP_FACTOR			1.02f
#define SCALE_DOWN_FACTOR   (1.0f / SCALE_UP_FACTOR)

#define CENTER_SELECTION_SENSITIVITY 0.03f

typedef struct {
	int width, height;
	int initial_anchor_x, initial_anchor_y;
} Window;

typedef enum {
	IDLE,
	CREATION,
	SELECTION,
	ANIMATION,
	BOUNCE
} Mode;

typedef struct {
	Mode current_mode;
	int leftbuttonpressed;
	int rightbuttonpressed;
	int selected_polygon_id;
} Status;

#define MAX_POSITIONS 128
#define MAX_POLYGONS 10
typedef struct {
	float point[MAX_POSITIONS][2];
	int n_points;
	float center_x, center_y;
	float color[3];
	int is_active;
	float vx, vy;
	float rotation_dir;
} My_Polygon;

// Functions for My_Polygon
void add_point(My_Polygon* pg, Window* wd, int x, int y);
void draw_lines_by_points(My_Polygon* pg, int is_selected, int is_creating);
void update_center_of_gravity(My_Polygon* pg);
void move_points(My_Polygon* pg, float del_x, float del_y);
void rotate_points_around_center_of_gravity(My_Polygon* pg, float angle_deg);
void scale_points_around_center_of_gravity(My_Polygon* pg, float scale_factor);
int separate_overlapping_polygons(My_Polygon polygons[]);
void handle_dynamic_collisions(My_Polygon polygons[]);
void check_window_boundary_collision(My_Polygon* pg);