#pragma once

typedef enum {
	CAMERA_U,
	CAMERA_I,
	CAMERA_O,
	CAMERA_P,
	CAMERA_A,
	CAMERA_T,
	CAMERA_G,
	CAMERA_B,
	NUM_CAMERAS
} CAMERA_INDEX;

typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move, rotation_axis;
} Camera;

typedef enum {
	MODE_NORMAL,
	MODE_CCTV,
	MODE_GAME
} SCENE_MODE;