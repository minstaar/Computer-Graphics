#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp>

#include "GameMode.h"
#include "MyObjects.h"

extern glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;
extern glm::mat4 ModelViewMatrix, ModelViewProjectionMatrix;
extern GLint     loc_ModelViewProjectionMatrix, loc_primitive_color;
extern GLuint    h_ShaderProgram_simple;

extern void set_current_camera(int camera_num);
extern void set_ViewMatrix_from_camera_frame(void);
extern float distance_2D(float x1, float y1, float x2, float y2);

#define TO_RADIAN         0.01745329252f
#define PLAYER_SPEED      30.0f
#define PITCH_MAX         (60.0f * TO_RADIAN)
#define MOUSE_SENSITIVITY 0.001f

int   flag_game_active = 0;
float player_pos[3] = { 1200.0f, 4000.0f, 0.0f };
float player_angle = 0.0f;
float player_pitch = 0.0f;
float player_body_angle = 0.0f;

int key_w = 0, key_a = 0, key_s = 0, key_d = 0;
int game_camera_mode = 1;

float player_vel_z = 0.0f;
int   flag_jumping = 0;
#define JUMP_SPEED    30.0f
#define GRAVITY      -3.5f
#define GROUND_Z      0.0f

#define TIGER_CATCH_RADIUS 400.0f
#define ITEM_COLLECT_RADIUS 300.0f

int game_state = 0;
int items_collected = 0;

float item_pos[5][3] = {
	{ -1700.0f,  1000.0f, 0.0f   },
	{  3100.0f, -2700.0f, -40.0f },
	{   700.0f,  2800.0f, 0.0f   },
	{  1120.0f,  -960.0f, 0.0f   },
	{  -240.0f,  -300.0f, 0.0f   },
};
int item_collected[5] = { 0, 0, 0, 0, 0 };

void init_game_mode(void) {
	player_pitch = 0.0f;
	flag_game_active = 1;
	key_w = key_a = key_s = key_d = 0;

	camera_info[CAMERA_B].fovy = 70.0f * TO_RADIAN;
	camera_info[CAMERA_B].near_c = 1.0f;
	camera_info[CAMERA_B].far_c = 50000.0f;
	camera_info[CAMERA_B].aspect_ratio = (float)glutGet(GLUT_WINDOW_WIDTH) / glutGet(GLUT_WINDOW_HEIGHT);

	glutSetCursor(GLUT_CURSOR_NONE);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

void exit_game_mode(void) {
	flag_game_active = 0;
	key_w = key_a = key_s = key_d = 0;
	glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
}

void update_camera_b(void) {
	camera_info[CAMERA_B].aspect_ratio = (float)glutGet(GLUT_WINDOW_WIDTH) / glutGet(GLUT_WINDOW_HEIGHT);
	
	glm::vec3 forward = glm::normalize(glm::vec3(
		sinf(player_angle) * cosf(player_pitch),
		cosf(player_angle) * cosf(player_pitch),
		sinf(player_pitch)));

	glm::vec3 n = -forward;
	glm::vec3 world_up = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 u = glm::normalize(glm::cross(forward, world_up));
	glm::vec3 v = glm::cross(n, u);

	if (game_camera_mode == 1) {
		camera_info[CAMERA_B].pos[0] = player_pos[0] + sinf(player_angle) * 60.0f;
		camera_info[CAMERA_B].pos[1] = player_pos[1] + cosf(player_angle) * 60.0f;
		camera_info[CAMERA_B].pos[2] = player_pos[2] + 190.0f;
	}
	else {
		camera_info[CAMERA_B].pos[0] = player_pos[0] - forward.x * 300.0f;
		camera_info[CAMERA_B].pos[1] = player_pos[1] - forward.y * 300.0f;
		camera_info[CAMERA_B].pos[2] = player_pos[2] + 390.0f;
	}

	camera_info[CAMERA_B].naxis[0] = n.x; camera_info[CAMERA_B].naxis[1] = n.y; camera_info[CAMERA_B].naxis[2] = n.z;
	camera_info[CAMERA_B].uaxis[0] = u.x; camera_info[CAMERA_B].uaxis[1] = u.y; camera_info[CAMERA_B].uaxis[2] = u.z;
	camera_info[CAMERA_B].vaxis[0] = v.x; camera_info[CAMERA_B].vaxis[1] = v.y; camera_info[CAMERA_B].vaxis[2] = v.z;

	if (current_camera_index == CAMERA_B)
		set_current_camera(CAMERA_B);
}

void update_game_mode(void) {
	float fx = sinf(player_angle);
	float fy = cosf(player_angle);
	float rx = cosf(player_angle);
	float ry = -sinf(player_angle);

	float move_x = 0.0f, move_y = 0.0f;

	if (key_w) { move_x += fx; move_y += fy; }
	if (key_s) { move_x -= fx; move_y -= fy; }
	if (key_d) { move_x += rx; move_y += ry; }
	if (key_a) { move_x -= rx; move_y -= ry; }

	player_pos[0] += move_x * PLAYER_SPEED;
	player_pos[1] += move_y * PLAYER_SPEED;

	if (flag_jumping) {
		player_vel_z += GRAVITY;
		player_pos[2] += player_vel_z;

		if (player_pos[2] <= GROUND_Z) {
			player_pos[2] = GROUND_Z;
			player_vel_z = 0.0f;
			flag_jumping = 0;
		}
	}

	update_camera_b();

	float dist_tiger = distance_2D(player_pos[0], player_pos[1], tiger_position[0], tiger_position[1]);
	
	if (dist_tiger < TIGER_CATCH_RADIUS) {
		game_state = 0;
		items_collected = 0;
		memset(item_collected, 0, sizeof(item_collected));
		player_pos[0] = 1200.0f;
		player_pos[1] = 4000.0f;
		player_pos[2] = 0.0f;
		return;
	}

	for (int i = 0; i < 5; i++) {
		if (item_collected[i]) continue;
		float dist = distance_2D(player_pos[0], player_pos[1], item_pos[i][0], item_pos[i][1]);
		if (dist < ITEM_COLLECT_RADIUS) {
			item_collected[i] = 1;
			items_collected++;
		}
	}

	if (items_collected >= 5) {
		game_state = 0;
		items_collected = 0;
		memset(item_collected, 0, sizeof(item_collected));
		player_pos[0] = 1200.0f;
		player_pos[1] = 4000.0f;
		player_pos[2] = 0.0f;
	}
}

void draw_game_objects(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUseProgram(h_ShaderProgram_simple);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(player_pos[0], player_pos[1], player_pos[2]));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -player_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(300.0f, 300.0f, 300.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniform3f(loc_primitive_color, 0.0f, 1.0f, 1.0f);
	draw_ben();

	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void keyboard_game(unsigned char key, int x, int y) {
	switch (key) {
	case 'w': key_w = 1; break;
	case 's': key_s = 1; break;
	case 'a': key_a = 1; break;
	case 'd': key_d = 1; break;
	case '1': game_camera_mode = 1; break;
	case '3': game_camera_mode = 3; break;
	case ' ':
		if (flag_jumping < 2) {
			flag_jumping++;
			player_vel_z = JUMP_SPEED;
		}
		break;
	case 'r':
		game_state = 0;
		items_collected = 0;
		memset(item_collected, 0, sizeof(item_collected));
		player_pos[0] = 1200.0f;
		player_pos[1] = 4000.0f;
		player_pos[2] = 0.0f;
		break;
	case 27: glutLeaveMainLoop(); break;
	}
}

void keyboard_game_up(unsigned char key, int x, int y) {
	switch (key) {
	case 'w': key_w = 0; break;
	case 's': key_s = 0; break;
	case 'a': key_a = 0; break;
	case 'd': key_d = 0; break;
	}
}

void mousemove_game(int x, int y) {
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;

	if (x == cx && y == cy) return;

	int dx = x - cx;
	int dy = y - cy;

	player_angle += dx * MOUSE_SENSITIVITY;
	player_pitch -= dy * MOUSE_SENSITIVITY;

	if (player_pitch > PITCH_MAX) player_pitch = PITCH_MAX;
	if (player_pitch < -PITCH_MAX) player_pitch = -PITCH_MAX;

	glutWarpPointer(cx, cy);
	glutPostRedisplay();
}