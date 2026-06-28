#pragma once

void prepare_bus(void);
void prepare_tower(void);
void prepare_bike(void);
void prepare_ant(void);
void prepare_cat(void);
void prepare_tiger(void);
void prepare_ben(void);
void prepare_spider(void);

void draw_bus(void);
void draw_tower(void);
void draw_bike(void);
void draw_ant(void);
void draw_cat(void);
void draw_tiger(void);
void draw_ben(void);
void draw_spider(void);

extern int cur_frame_tiger;
extern int cur_frame_ben;
extern int cur_frame_spider;

extern GLuint tiger_VAO, tiger_VBO;
extern GLuint ben_VAO, ben_VBO;
extern GLuint spider_VAO, spider_VBO;
extern GLuint bus_VAO, bus_VBO;
extern GLuint tower_VAO, tower_VBO;
extern GLuint bike_VAO, bike_VBO;
extern GLuint ant_VAO, ant_VBO;
extern GLuint cat_VAO, cat_VBO;