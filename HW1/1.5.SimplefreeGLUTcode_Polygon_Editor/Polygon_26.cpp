#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Definitions_26.h"

void add_point(My_Polygon *pg, Window *wd, int x, int y) {
	pg->point[pg->n_points][0] = 2.0f * ((float)x) / wd->width - 1.0f;
	pg->point[pg->n_points][1] = 2.0f * ((float)wd->height - y) / wd->height - 1.0f;
	pg->n_points++; 
}

void draw_lines_by_points(My_Polygon* pg, int is_selected, int is_creating) {
	if (pg->n_points == 0) return;

	glColor3f(POINT_COLOR);
	for (int i = 0; i < pg->n_points; i++) {
		glBegin(GL_POINTS);
		glVertex2f(pg->point[i][0], pg->point[i][1]);
		glEnd();
	}

	glColor3f(pg->color[0], pg->color[1], pg->color[2]);
	if (pg->n_points == 2) {
		glBegin(GL_LINES);
		glVertex2f(pg->point[0][0], pg->point[0][1]);
		glVertex2f(pg->point[1][0], pg->point[1][1]);
		glEnd();
	}
	else if (pg->n_points >= 3) {
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < pg->n_points; i++)
			glVertex2f(pg->point[i][0], pg->point[i][1]);
		glEnd();
	}

	if (pg->n_points >= 3 && !is_creating) {
		if (is_selected)
			glColor3f(CENTER_POINT_COLOR_SELECTED);
		else
			glColor3f(CENTER_POINT_COLOR_UNSELECTED);

		glBegin(GL_POINTS);
		glVertex2f(pg->center_x, pg->center_y);
		glEnd();
	}
}

void update_center_of_gravity(My_Polygon* pg) {
	pg->center_x = pg->center_y = 0.0f;
	if (pg->n_points == 0) return;
	for (int i = 0; i < pg->n_points; i++) {
		pg->center_x += pg->point[i][0], pg->center_y += pg->point[i][1];
	}
	pg->center_x /= (float)pg->n_points, pg->center_y /= (float)pg->n_points;
}

void move_points(My_Polygon* pg, float del_x, float del_y) {
	for (int i = 0; i < pg->n_points; i++) {
		pg->point[i][0] += del_x, pg->point[i][1] += del_y;
	}
	pg->center_x += del_x, pg->center_y += del_y;
}

void rotate_points_around_center_of_gravity(My_Polygon* pg, float angle_deg) {
	float radian = angle_deg * 3.141592f / 180.0f;
	float cos_a = cos(radian);
	float sin_a = sin(radian);

	for (int i = 0; i < pg->n_points; i++) {
		float x = pg->point[i][0] - pg->center_x;
		float y = pg->point[i][1] - pg->center_y;
		pg->point[i][0] = x * cos_a - y * sin_a;
		pg->point[i][1] = x * sin_a + y * cos_a;
		pg->point[i][0] += pg->center_x;
		pg->point[i][1] += pg->center_y;
	}
}

void scale_points_around_center_of_gravity(My_Polygon* pg, float scale_factor) {
	for (int i = 0; i < pg->n_points; i++) {
		float x, y;
		x = scale_factor * (pg->point[i][0] - pg->center_x)
			 + pg->center_x;
		y = scale_factor * (pg->point[i][1] - pg->center_y)
			+ pg->center_y;
		pg->point[i][0] = x, pg->point[i][1] = y;
	}
}

int check_collision(My_Polygon* a, My_Polygon* b, int is_dynamic) {
	float minX_A = 2.0f, maxX_A = -2.0f, minY_A = 2.0f, maxY_A = -2.0f;
	for (int i = 0; i < a->n_points; i++) {
		if (a->point[i][0] < minX_A) minX_A = a->point[i][0];
		if (a->point[i][0] > maxX_A) maxX_A = a->point[i][0];
		if (a->point[i][1] < minY_A) minY_A = a->point[i][1];
		if (a->point[i][1] > maxY_A) maxY_A = a->point[i][1];
	}

	float minX_B = 2.0f, maxX_B = -2.0f, minY_B = 2.0f, maxY_B = -2.0f;
	for (int i = 0; i < b->n_points; i++) {
		if (b->point[i][0] < minX_B) minX_B = b->point[i][0];
		if (b->point[i][0] > maxX_B) maxX_B = b->point[i][0];
		if (b->point[i][1] < minY_B) minY_B = b->point[i][1];
		if (b->point[i][1] > maxY_B) maxY_B = b->point[i][1];
	}

	if (maxX_A > minX_B && minX_A < maxX_B && maxY_A > minY_B && minY_A < maxY_B) {
		float overlap_x1 = maxX_A - minX_B;
		float overlap_x2 = maxX_B - minX_A;
		float overlap_x = (overlap_x1 < overlap_x2) ? overlap_x1 : overlap_x2;

		float overlap_y1 = maxY_A - minY_B;
		float overlap_y2 = maxY_B - minY_A;
		float overlap_y = (overlap_y1 < overlap_y2) ? overlap_y1 : overlap_y2;

		if (overlap_x < overlap_y) {
			if (minX_A < minX_B) {
				move_points(a, -overlap_x / 2.0f - 0.001f, 0.0f);
				move_points(b, overlap_x / 2.0f + 0.001f, 0.0f);
			}
			else {
				move_points(a, overlap_x / 2.0f + 0.001f, 0.0f);
				move_points(b, -overlap_x / 2.0f - 0.001f, 0.0f);
			}

			if (is_dynamic) {
				float t_vx = a->vx;
				a->vx = b->vx;
				b->vx = t_vx;
				a->rotation_dir *= -1.0f;
				b->rotation_dir *= -1.0f;
			}
		}
		else {
			if (minY_A < minY_B) {
				move_points(a, 0.0f, -overlap_y / 2.0f - 0.001f);
				move_points(b, 0.0f, overlap_y / 2.0f + 0.001f);
			}
			else {
				move_points(a, 0.0f, overlap_y / 2.0f + 0.001f);
				move_points(b, 0.0f, -overlap_y / 2.0f - 0.001f);
			}

			if (is_dynamic) {
				float t_vy = a->vy;
				a->vy = b->vy;
				b->vy = t_vy;
				a->rotation_dir *= -1.0f;
				b->rotation_dir *= -1.0f;
			}
		}
		return 1;
	}
	return 0;
}

int separate_overlapping_polygons(My_Polygon polygons[]) {
	int max_iteration = 20;
	int iter = 0;
	int is_resolved;

	do {
		is_resolved = 1;

		for (int i = 0; i < MAX_POLYGONS; i++) {
			if (!polygons[i].is_active) continue;

			for (int j = i + 1; j < MAX_POLYGONS; j++) {
				if (!polygons[j].is_active) continue;

				if (check_collision(&polygons[i], &polygons[j], 0))
					is_resolved = 0;
			}
		}
		iter++;
	} while (is_resolved == 0 && iter < max_iteration);

	return is_resolved;
}

void check_window_boundary_collision(My_Polygon* pg) {
	if (pg->n_points == 0) return;

	float minX = 2.0f, maxX = -2.0f, minY = 2.0f, maxY = -2.0f;
	for (int i = 0; i < pg->n_points; i++) {
		if (pg->point[i][0] < minX) minX = pg->point[i][0];
		if (pg->point[i][0] > maxX) maxX = pg->point[i][0];
		if (pg->point[i][1] < minY) minY = pg->point[i][1];
		if (pg->point[i][1] > maxY) maxY = pg->point[i][1];
	}

	if (maxX > 1.0f) pg->vx = -fabs(pg->vx), pg->rotation_dir *= -1.0f;
	else if (minX < -1.0f) pg->vx = fabs(pg->vx), pg->rotation_dir *= -1.0f;
	if (maxY > 1.0f) pg->vy = -fabs(pg->vy), pg->rotation_dir *= -1.0f;
	else if (minY < -1.0f) pg->vy = fabs(pg->vy), pg->rotation_dir *= -1.0f;
}

void handle_dynamic_collisions(My_Polygon polygons[]) {
	for (int i = 0; i < MAX_POLYGONS; i++) {
		if (!polygons[i].is_active) continue;

		for (int j = i + 1; j < MAX_POLYGONS; j++) {
			if (!polygons[j].is_active) continue;

			check_collision(&polygons[i], &polygons[j], 1);
		}
	}
}