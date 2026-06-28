#pragma once
#include "SceneTypes.h"

extern Camera camera_info[];
extern Camera camera_info_default[];
extern Camera current_camera;
extern int current_camera_index;
extern SCENE_MODE current_mode;

extern float ben_position[3];
extern float ben_angle;
extern int cur_frame_ben;

extern float tiger_position[3];

extern int flag_game_active;
extern float player_pos[3];
extern float player_angle;
extern float player_pitch;
extern float player_body_angle;

extern int key_w, key_s, key_a, key_d;
extern int flag_jumping;
extern int item_collected[5];

void init_game_mode(void);
void exit_game_mode(void);
void update_game_mode(void);
void draw_game_objects(void);
void update_camera_b(void);

void keyboard_game(unsigned char key, int x, int y);
void keyboard_game_up(unsigned char key, int x, int y);
void mousemove_game(int x, int y);