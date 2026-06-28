#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Definitions_26.h"

Window wd;
Status st;
My_Polygon polygons[MAX_POLYGONS];

const float PALETTE[MAX_POLYGONS][3] = {
	{0.0f, 0.0f, 1.0f}, // 0: Blue
	{0.0f, 0.6f, 0.0f}, // 1: Dark Green
	{0.6f, 0.0f, 0.8f}, // 2: Purple
	{1.0f, 0.5f, 0.0f}, // 3: Orange
	{0.0f, 0.7f, 0.7f}, // 4: Teal
	{0.6f, 0.3f, 0.1f}, // 5: Brown
	{1.0f, 0.2f, 0.6f}, // 6: Deep Pink
	{0.0f, 0.0f, 0.5f}, // 7: Navy
	{0.4f, 0.5f, 0.1f}, // 8: Olive
	{0.3f, 0.3f, 0.3f}, // 9: Dark Gray
};

// GLUT callbacks

static float animation_scale_factor = 1.0f;
static int animation_scale_dir = -1;
void timer(int value) {
	if (st.current_mode == ANIMATION && st.selected_polygon_id != -1) {
		int current_id = st.selected_polygon_id;

		rotate_points_around_center_of_gravity(&polygons[current_id], -2.0f);

		if (animation_scale_dir == -1) {
			scale_points_around_center_of_gravity(&polygons[current_id], SCALE_DOWN_FACTOR);
			animation_scale_factor *= SCALE_DOWN_FACTOR;

			if (animation_scale_factor < 0.5f)
				animation_scale_dir = 1;
		}
		else {
			scale_points_around_center_of_gravity(&polygons[current_id], SCALE_UP_FACTOR);
			animation_scale_factor *= SCALE_UP_FACTOR;

			if (animation_scale_factor > 1.5f)
				animation_scale_dir = -1;
		}

		glutPostRedisplay();
		glutTimerFunc(ROTATION_STEP, timer, 0);
	}

	else if (st.current_mode == BOUNCE) {
		for (int i = 0; i < MAX_POLYGONS; i++) {
			if (polygons[i].is_active) {
				rotate_points_around_center_of_gravity(&polygons[i], polygons[i].rotation_dir);
				move_points(&polygons[i], polygons[i].vx, polygons[i].vy);
				check_window_boundary_collision(&polygons[i]);
			}
		}

		handle_dynamic_collisions(polygons);

		glutPostRedisplay();
		glutTimerFunc(ROTATION_STEP, timer, 0);
	}
}

void display(void) {
	switch (st.current_mode) {
		case IDLE:
			glClearColor(BACKGROUND_COLOR_IDLE, 1.0f);
			break;
		case CREATION:
			glClearColor(BACKGROUND_COLOR_CREATION, 1.0f);
			break;
		case SELECTION:
			glClearColor(BACKGROUND_COLOR_SELECTION, 1.0f);
			break;
		case ANIMATION:
			glClearColor(BACKGROUND_COLOR_ANIMATION, 1.0f);
			break;
		case BOUNCE:
			glClearColor(BACKGROUND_COLOR_BOUNCE, 1.0f);
			break;
	}
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < MAX_POLYGONS; i++) {
		if (polygons[i].is_active) {
			int is_selected = (st.current_mode == SELECTION || st.current_mode == ANIMATION)
				&& st.selected_polygon_id == i ? 1 : 0;
			int is_creating = st.current_mode == CREATION && st.selected_polygon_id == i ? 1 : 0;
			draw_lines_by_points(&polygons[i], is_selected, is_creating);
		}
	}
	glFlush();
}

static int duplicated_cnt = 1;
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 's':
		if (st.current_mode == IDLE) {
			int empty_slot = -1;
			for (int i = 0; i < MAX_POLYGONS; i++) {
				if (!polygons[i].is_active) {
					empty_slot = i;
					break;
				}
			}

			if (empty_slot != -1) {
				st.current_mode = CREATION;
				st.selected_polygon_id = empty_slot;

				polygons[empty_slot].is_active = 1;
				polygons[empty_slot].n_points = 0;
				polygons[empty_slot].color[0] = PALETTE[empty_slot][0];
				polygons[empty_slot].color[1] = PALETTE[empty_slot][1];
				polygons[empty_slot].color[2] = PALETTE[empty_slot][2];
			}
			else {
				fprintf(stderr, "*** Cannot create more polygons!\n");
			}
			glutPostRedisplay();
		}
		break;
	case 'e':
		if (st.current_mode == CREATION) {
			int current_id = st.selected_polygon_id;

			if (polygons[current_id].n_points >= 3) {
				update_center_of_gravity(&polygons[current_id]);
				st.current_mode = IDLE;
				st.selected_polygon_id = -1;
			}
			else {
				fprintf(stderr, "*** Polygons require at least 3 points!\n");
			}
			glutPostRedisplay();
		}
		break;
	case 'c':
		if (st.current_mode == SELECTION) {
			polygons[st.selected_polygon_id].is_active = 0;
			st.current_mode = IDLE;
			st.selected_polygon_id = -1;
			glutPostRedisplay();
		}
		break;
	case 'A':
		if (st.current_mode == SELECTION) {
			st.current_mode = ANIMATION;
			animation_scale_factor = 1.0f;
			animation_scale_dir = -1;

			glutTimerFunc(ROTATION_STEP, timer, 0);
			glutPostRedisplay();
		}
		else if (st.current_mode == ANIMATION) {
			st.current_mode = SELECTION;
			glutPostRedisplay();
		}
		break;
	case 'd':
		if (st.current_mode == SELECTION) {
			int current_id = st.selected_polygon_id;
			int empty_slot = -1;

			for (int i = 0; i < MAX_POLYGONS; i++) {
				if (!polygons[i].is_active) {
					empty_slot = i;
					break;
				}
			}

			if (empty_slot != -1) {
				polygons[empty_slot] = polygons[current_id];
				polygons[empty_slot].color[0] = PALETTE[empty_slot][0];
				polygons[empty_slot].color[1] = PALETTE[empty_slot][1];
				polygons[empty_slot].color[2] = PALETTE[empty_slot][2];

				move_points(&polygons[empty_slot], 0.05f * duplicated_cnt, -0.05f * duplicated_cnt);
				duplicated_cnt++;
			}
			else {
				fprintf(stderr, "*** Cannot create more polygons!\n");
			}

			glutPostRedisplay();
		}
		break;
	case 'B':
		if (st.current_mode == IDLE) {
			for (int i = 0; i < MAX_POLYGONS; i++) {
				if (polygons[i].is_active) {
					float angle = (rand() % 360) * 3.141592f / 180.0f;
					polygons[i].vx = cos(angle) * 0.01f;
					polygons[i].vy = sin(angle) * 0.01f;
					polygons[i].rotation_dir = rand() % 2 ? 1.0f : -1.0f;
				}
			}

			if (separate_overlapping_polygons(polygons)) {
				st.current_mode = BOUNCE;
				glutTimerFunc(ROTATION_STEP, timer, 0);
				glutPostRedisplay();
			}
		}
		else if (st.current_mode == BOUNCE) {
			st.current_mode = IDLE;
			glutPostRedisplay();
		}
		break;
	case 'f':
		glutLeaveMainLoop(); 
		break;
	}
}

void wheel(int wheel, int direction, int x, int y) {
	if (st.current_mode == SELECTION && st.selected_polygon_id != -1) {
		int current_id = st.selected_polygon_id;

		if (direction == -1) { // zoom out
			scale_points_around_center_of_gravity(&polygons[current_id], SCALE_DOWN_FACTOR);
		}
		else { // zoom in
			scale_points_around_center_of_gravity(&polygons[current_id], SCALE_UP_FACTOR);
		}
		glutPostRedisplay();
	}
}

static int prev_x, prev_y;
static int selected_vertex_id = -1;
void mousepress(int button, int state, int x, int y) {
	if (st.current_mode == CREATION) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			int key_state = glutGetModifiers();
			if (key_state & GLUT_ACTIVE_SHIFT) {
				int current_id = st.selected_polygon_id;
				if (polygons[current_id].n_points < MAX_POSITIONS) {
					add_point(&polygons[current_id], &wd, x, y);
					glutPostRedisplay();
				}
			}
		}
	}
	
	else if (st.current_mode == IDLE) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			float x_normalized = 2.0f * (float)x / wd.width - 1.0f;
			float y_normalized = 2.0f * ((float)wd.height - y) / wd.height - 1.0f;

			for (int i = MAX_POLYGONS - 1; i >= 0; i--) {
				if (polygons[i].is_active) {
					if (fabs(x_normalized - polygons[i].center_x) < CENTER_SELECTION_SENSITIVITY
						&& fabs(y_normalized - polygons[i].center_y) * (float)wd.width / (float)wd.height < CENTER_SELECTION_SENSITIVITY) {

						st.current_mode = SELECTION;
						st.selected_polygon_id = i;
						duplicated_cnt = 1;

						glutPostRedisplay();
						break;
					}
				}
			}
		}
	}

	else if (st.current_mode == SELECTION) {
		if (state == GLUT_DOWN) {
			if (button == GLUT_LEFT_BUTTON) {
				float x_normalized = 2.0f * (float)x / wd.width - 1.0f;
				float y_normalized = 2.0f * ((float)wd.height - y) / wd.height - 1.0f;

				int current_id = st.selected_polygon_id;

				if (fabs(x_normalized - polygons[current_id].center_x) < CENTER_SELECTION_SENSITIVITY
					&& fabs(y_normalized - polygons[current_id].center_y) * (float)wd.width / (float)wd.height < CENTER_SELECTION_SENSITIVITY) {

					st.current_mode = IDLE;
					st.selected_polygon_id = -1;
					glutPostRedisplay();
				}
				else {
					selected_vertex_id = -1;
					for (int i = 0; i < polygons[current_id].n_points; i++) {
						if (fabs(x_normalized - polygons[current_id].point[i][0]) < CENTER_SELECTION_SENSITIVITY
							&& fabs(y_normalized - polygons[current_id].point[i][1]) * (float)wd.width / (float)wd.height < CENTER_SELECTION_SENSITIVITY) {

							selected_vertex_id = i;
							break;
						}
					}

					st.leftbuttonpressed = 1;
					prev_x = x, prev_y = y;
				}
			}
			else if (button == GLUT_RIGHT_BUTTON) {
				st.rightbuttonpressed = 1;
				prev_x = x, prev_y = y;
			}
		}
		else if (state == GLUT_UP) {
			if (button == GLUT_LEFT_BUTTON) {
				st.leftbuttonpressed = 0;
				selected_vertex_id = -1;
			}
			if (button == GLUT_RIGHT_BUTTON)
				st.rightbuttonpressed = 0;
		}
	}
}

void mousemove(int x, int y) {
	if (st.current_mode == SELECTION) {
		int current_id = st.selected_polygon_id;

		if (st.leftbuttonpressed) {
			float delx = 2.0f * (float)(x - prev_x) / wd.width;
			float dely = 2.0f * (float)(prev_y - y) / wd.height;

			if (selected_vertex_id != -1) {
				polygons[current_id].point[selected_vertex_id][0] += delx;
				polygons[current_id].point[selected_vertex_id][1] += dely;

				update_center_of_gravity(&polygons[current_id]);
			}
			else {
				move_points(&polygons[current_id], delx, dely);
			}

			prev_x = x, prev_y = y;
			glutPostRedisplay();
		}
		else if (st.rightbuttonpressed) {
			float dx = (float)(x - prev_x);
			float angle = -dx * 0.5f;

			rotate_points_around_center_of_gravity(&polygons[current_id], angle);
			prev_x = x, prev_y = y;

			glutPostRedisplay();
		}
	}
}
	
void reshape(int width, int height) {
	fprintf(stdout, "### The new window size is %dx%d.\n", width, height);
	wd.width = width, wd.height = height;
	glViewport(0, 0, wd.width, wd.height);
}

void close(void) {
	fprintf(stdout, "\n^^^ The control is at the close callback function now.\n\n");
}
// End of GLUT callbacks

void initialize_polygon_editor(void) {
	wd.width = 800, wd.height = 600, wd.initial_anchor_x = 500, wd.initial_anchor_y = 200;
	
	st.current_mode = IDLE;
	st.leftbuttonpressed = 0;
	st.rightbuttonpressed = 0;
	st.selected_polygon_id = -1;

	for (int i = 0; i < MAX_POLYGONS; i++) {
		polygons[i].is_active = 0;
	}
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(wheel);
	glutMouseFunc(mousepress);
	glutMotionFunc(mousemove);
	glutReshapeFunc(reshape);
	glutCloseFunc(close);
}

void initialize_renderer(void) {
	register_callbacks();

	glPointSize(5.0);
	glClearColor(BACKGROUND_COLOR_IDLE, 1.0f);
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = TRUE;
	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 4
int main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 SimplefreeGLUTcode_Polygon_Editor";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 's', 'e', 'c', 'A', 'f'",
		"    - Special keys used: 'd', 'B'",
		"    - Mouse used: L-click, L-click and move, R-click and move",
		"    - Other operations: window reshape"
	};

	glutInit(&argc, argv);
	initialize_polygon_editor();

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE); // <-- Be sure to use this profile for this example code!
 //	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_RGBA);

	glutInitWindowSize(wd.width, wd.height);
	glutInitWindowPosition(wd.initial_anchor_x, wd.initial_anchor_y);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

   // glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_EXIT); // default
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	
	glutMainLoop();
	fprintf(stdout, "^^^ The control is at the end of main function now.\n\n");
	return 0;
}
