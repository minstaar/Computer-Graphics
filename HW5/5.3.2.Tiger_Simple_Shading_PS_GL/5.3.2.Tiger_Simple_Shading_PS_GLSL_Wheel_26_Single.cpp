#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Shaders/LoadShaders.h"
#include <FreeImage/FreeImage.h>
 
#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LIGHT_IN_EC 0
#define LIGHT_IN_WC 1
#define LIGHT_IN_MC 2

#define LOC_POSITION 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

#define FLOOR_SIZE 3000.0f	
#define WALL_HEIGHT FLOOR_SIZE/3.0f
#define INITIAL_CAMERA_PRP FLOOR_SIZE/2.0f, FLOOR_SIZE/3.0f, FLOOR_SIZE/2.0f
#define INITIAL_CAMERA_VRP -0.0f, 0.0f, -0.0f
#define LIGHT_SPOT_CUTOFF_ANGLE 30.0f
#define LIGHT_SPOT_EXPONENT 10.0f
#define LIGHT_POS_IN_MC_MOVING_TIGER 0.0f, 0.0f, 15.0f
#define LIGHT_ATTENUATION_FACTORS_MC 1.0f, 0.0f, 0.00001f
#define LIGHT_SPOT_DIRECTION_IN_TIGER_MC 0.0f, 0.0f, -1.0f
#define LIGHT_POS_IN_WC -150.0f, 700.0f, -300.0f
#define LIGHT_SPOT_DIRECTION_IN_WC 0.0f, -1.0f, 0.0f
#define WC_LIGHT_REVOLUTION_STEP 2
#define LIGHT_POS_IN_EC -50.0f, 50.0f, -800.0f
#define LIGHT_SPOT_DIRECTION_IN_EC 0.0f, 0.0f, -1.0f
#define DIRECT_LIGHT_L_IN_MC 0.9f, 1.0f, 0.2f
#define DIRECT_LIGHT_L_IN_WC 0.9f, 0.9f, 0.9f
#define DIRECT_LIGHT_L_IN_EC 0.9f, 0.1f, 0.9f

GLuint h_ShaderProgram_simple, h_ShaderProgram_PS; // handles to shader programs

// for simple shaders
GLint loc_ModelViewProjectionMatrix_simple, loc_primitive_color_simple;

// for Simple Phone Shading shaders
GLint loc_ModelViewProjectionMatrix_PS, loc_ModelViewMatrix_PS, loc_ModelViewMatrixInvTrans_PS;
GLint loc_Light_Position, loc_Light_La, loc_Light_L;
GLint loc_Light_Spot_direction, loc_Light_Spot_cutoff_angle, loc_Light_Spot_exponent;
GLint loc_Light_Attenuation_factors;
GLint loc_Material_Ka, loc_Material_Kd, loc_Material_Ks, loc_Material_Shininess;
GLint loc_use_halfway_vector, loc_apply_spot_light, loc_apply_attenuation;
GLint loc_u_flag_texture_mapping, loc_u_base_texture, loc_u_alpha;
GLint loc_u_flag_reflection, loc_u_reflection_texture, loc_u_window_size;
GLint loc_u_flag_shadow, loc_u_shadow_map, loc_u_shadow_matrix, loc_u_caster_alpha;

GLuint reflection_FBO, reflection_texture, reflection_depth_RBO;
int window_width = 1280, window_height = 1024;
int reflection_pass_active = 0;
int shadow_pass_active = 0;

#define SHADOW_MAP_SIZE 2048
GLuint shadow_FBO, shadow_map;

GLuint texture_tiger, texture_cat, texture_ant, texture_spider, texture_wolf;
GLuint texture_floor, texture_wall;

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.
glm::mat4 ModelViewProjectionMatrix, ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;
glm::mat4 ViewMatrix, ProjectionMatrix;
glm::mat4 tiger_ModelViewMatrix;

glm::vec3 camera_PRP, camera_VRP;
int left_button_pressed = 0;
int mouse_prev_x, mouse_prev_y;
#define CAMERA_VRP_MOVE_SPEED 3.0f

struct States {
	int cur_frame_tiger{ 0 };
	int rotation_angle_tiger{ 0 };
	int animation_mode{ 1 };
	int use_halfway_vector{ 0 };
	int polygon_fill_mode{ 1 };
	int light_geometry_space{ LIGHT_IN_EC };
	int use_spot_light_for_EC_light{ 0 };
	int use_spot_light_for_WC_light { 0 };
	int use_spot_light_for_circling_tiger_MC{ 0 };
	int use_attenuation_for_MC_light{ 0 };
	int use_circular_motion_for_WC_light{ 0 };
	int revolution_angle_WC{ 0 };
	int cur_frame_spider{ 0 };
	int cur_frame_wolf{ 0 };
	int anim_tick{ 0 };
	int use_texture_objects{ 0 };
	int use_texture_floor_wall{ 0 };
	int use_blending{ 0 };
	float tiger_opacity{ 0.5f };
	int use_reflection{ 0 };
	int use_shadow{ 0 };
};
States states;

// axes object
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) { // draw coordinate axes
	// initialize vertex buffer object
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// initialize vertex array object
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

 void draw_axes(void) {
	 // assume ShaderProgram_simple is used
	 glBindVertexArray(axes_VAO);
	 glUniform3fv(loc_primitive_color_simple, 1, axes_color[0]);
	 glDrawArrays(GL_LINES, 0, 2);
	 glUniform3fv(loc_primitive_color_simple, 1, axes_color[1]);
	 glDrawArrays(GL_LINES, 2, 2);
	 glUniform3fv(loc_primitive_color_simple, 1, axes_color[2]);
	 glDrawArrays(GL_LINES, 4, 2);
	 glBindVertexArray(0);
 }

 // floor object
 GLuint rectangle_VBO, rectangle_VAO;
 GLfloat rectangle_vertices[6][8] = {
	 { 0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f },
	 { 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  4.0f, 0.0f },
	 { 1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  4.0f, 4.0f },
	 { 0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f },
	 { 1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  4.0f, 4.0f },
	 { 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 4.0f }
 };

  void prepare_floor(void) { // Draw coordinate axes.
	 // Initialize vertex buffer object.
	 glGenBuffers(1, &rectangle_VBO);

	 glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), &rectangle_vertices[0][0], GL_STATIC_DRAW);

	 // Initialize vertex array object.
	 glGenVertexArrays(1, &rectangle_VAO);
	 glBindVertexArray(rectangle_VAO);

	 glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	 glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
	 glEnableVertexAttribArray(0);
	 glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	 glEnableVertexAttribArray(1);
	 glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
	 glEnableVertexAttribArray(LOC_TEXCOORD);

	 glBindBuffer(GL_ARRAY_BUFFER, 0);
	 glBindVertexArray(0);
 }

 void set_material_floor(void) {
	 glUniform3f(loc_Material_Ka, 0.19225f, 0.19225f, 0.19225f);
	 glUniform3f(loc_Material_Kd, 0.50754f, 0.50754f, 0.50754f);
	 glUniform3f(loc_Material_Ks, 0.508273f, 0.508273f, 0.508273f);
	 glUniform1f(loc_Material_Shininess, 128.2f); // [0.0, 128.0]
 }

 void draw_floor(void) {
	 glFrontFace(GL_CCW);

	 glBindVertexArray(rectangle_VAO);
	 glDrawArrays(GL_TRIANGLES, 0, 6);
	 glBindVertexArray(0);
 }

 // tiger object
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat *tiger_vertices[N_TIGER_FRAMES];

int read_geometry(GLfloat **object, int bytes_per_primitive, const char *filename) {
	int n_triangles;
	FILE *fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL){
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float *)malloc(n_triangles*bytes_per_primitive);
	if (*object == NULL){
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void My_glTexImage2D_from_file(const char *filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP *tx_pixmap, *tx_pixmap_32;
	int width, height;
	GLvoid *data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);
	FreeImage_FlipVertical(tx_pixmap);

	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);
}

void prepare_texture(GLuint *texture_name, const char *filename) {
	glGenTextures(1, texture_name);
	glBindTexture(GL_TEXTURE_2D, *texture_name);
	My_glTexImage2D_from_file(filename);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void prepare_textures(void) {
	prepare_texture(&texture_tiger, "Data/tiger_tex.jpg");
	prepare_texture(&texture_cat, "Data/static_objects/cat_diff.tga");
	prepare_texture(&texture_ant, "Data/static_objects/antTexture.jpg");
	prepare_texture(&texture_spider, "Data/dynamic_objects/spider/Spinnen_Bein_tex.jpg");
	prepare_texture(&texture_wolf, "Data/dynamic_objects/wolf/Wolf_Body.jpg");
	prepare_texture(&texture_floor, "Data/floor_tile.jpg");
	prepare_texture(&texture_wall, "Data/wall_stone.jpg");
}

void prepare_reflection(void) {
	glGenFramebuffers(1, &reflection_FBO);
	glGenTextures(1, &reflection_texture);
	glGenRenderbuffers(1, &reflection_depth_RBO);

	glBindTexture(GL_TEXTURE_2D, reflection_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, reflection_depth_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, window_width, window_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, reflection_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflection_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflection_depth_RBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void prepare_shadow(void) {
	glGenTextures(1, &shadow_map);
	glBindTexture(GL_TEXTURE_2D, shadow_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadow_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void set_material_tiger(int ambient, int diffuse, int specular) {
 
	if (ambient) 
		glUniform3f(loc_Material_Ka, 0.4f, 0.3f, 0.1f);
	else 
		glUniform3f(loc_Material_Ka, 0.0f, 0.0f, 0.0f);
	if (diffuse)
		glUniform3f(loc_Material_Kd, 0.95164f, 0.60648f, 0.22648f);
	else 
		glUniform3f(loc_Material_Kd, 0.0f, 0.0f, 0.0f);
	if (specular) {
		glUniform3f(loc_Material_Ks, 0.628281f, 0.555802f, 0.555802f);
		glUniform1f(loc_Material_Shininess, 21.2f); // [0.0, 128.0]
	}
	else {
		glUniform3f(loc_Material_Ks, 0.0f, 0.0f, 0.0f);
		glUniform1f(loc_Material_Shininess, 1.0f); // [0.0, 128.0]
	}
}

void prepare_tiger(void) { // vertices enumerated clockwise
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles*n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
		tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(LOC_TEXCOORD);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_tiger(void) {
	glFrontFace(GL_CW);
	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[states.cur_frame_tiger],
		3 * tiger_n_triangles[states.cur_frame_tiger]);
	glBindVertexArray(0);
}

GLuint cat_VBO, cat_VAO; int cat_n_triangles;
GLuint ant_VBO, ant_VAO; int ant_n_triangles;

void prepare_static_object(GLuint *VBO, GLuint *VAO, int *n_triangles, const char *filename) {
	int n_bytes_per_vertex = 8 * sizeof(float);
	int n_bytes_per_triangle = 3 * n_bytes_per_vertex;
	GLfloat *vertices;

	*n_triangles = read_geometry(&vertices, n_bytes_per_triangle, filename);

	glGenBuffers(1, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, (*n_triangles) * n_bytes_per_triangle, vertices, GL_STATIC_DRAW);
	free(vertices);

	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(LOC_POSITION);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(LOC_NORMAL);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(LOC_TEXCOORD);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void set_material_default(void) {
	glUniform3f(loc_Material_Ka, 0.2f, 0.2f, 0.2f);
	glUniform3f(loc_Material_Kd, 0.75f, 0.75f, 0.75f);
	glUniform3f(loc_Material_Ks, 0.3f, 0.3f, 0.3f);
	glUniform1f(loc_Material_Shininess, 20.0f);
}

void set_texture_mapping(int use_texture, GLuint texture_name) {
	glUniform1i(loc_u_flag_texture_mapping, use_texture);
	if (use_texture) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_name);
		glUniform1i(loc_u_base_texture, 0);
	}
}

void draw_object_PS(GLuint VAO, int n_triangles, GLuint texture_name) {
	set_texture_mapping(states.use_texture_objects, texture_name);
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	glFrontFace(GL_CW);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * n_triangles);
	glBindVertexArray(0);
}

void draw_static_objects(void) {
	glUseProgram(h_ShaderProgram_PS);
	if (states.polygon_fill_mode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	set_material_default();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-650.0f, 0.0f, 600.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 40.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(90.0f, 90.0f, 90.0f));
	draw_object_PS(cat_VAO, cat_n_triangles, texture_cat);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-790.0f, 0.0f, 690.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -35.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(90.0f, 90.0f, 90.0f));
	draw_object_PS(cat_VAO, cat_n_triangles, texture_cat);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-380.0f, 0.0f, 900.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 150.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(175.0f, 175.0f, 175.0f));
	draw_object_PS(cat_VAO, cat_n_triangles, texture_cat);

	if (!reflection_pass_active) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-FLOOR_SIZE / 2.0f, 350.0f, -200.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 20.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
		draw_object_PS(ant_VAO, ant_n_triangles, texture_ant);

		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-FLOOR_SIZE / 2.0f, 650.0f, 350.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -60.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
		draw_object_PS(ant_VAO, ant_n_triangles, texture_ant);

		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-250.0f, 450.0f, -FLOOR_SIZE / 2.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 30.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
		draw_object_PS(ant_VAO, ant_n_triangles, texture_ant);
	}

	glUseProgram(0);
}

#define N_SPIDER_FRAMES 16
#define N_WOLF_FRAMES 17
#define SPIDER_BOUNCE_PERIOD 100
#define SPIDER_Y_MIN 150.0f
#define SPIDER_Y_MAX 850.0f
#define SPIDER_WALL_X 600.0f
#define REFLECTION_FADE_HEIGHT 400.0f
#define WOLF_PATROL_PERIOD 90
#define WOLF_X_MIN 300.0f
#define WOLF_X_MAX 1300.0f
#define WOLF_Z -600.0f

GLuint spider_VBO, spider_VAO;
int spider_n_triangles[N_SPIDER_FRAMES];
int spider_vertex_offset[N_SPIDER_FRAMES];
GLuint wolf_VBO, wolf_VAO;
int wolf_n_triangles[N_WOLF_FRAMES];
int wolf_vertex_offset[N_WOLF_FRAMES];

void prepare_dynamic_object(GLuint *VBO, GLuint *VAO, int *n_triangles, int *vertex_offset,
	int n_frames, const char *filename_format) {
	int i, n_bytes_per_vertex = 8 * sizeof(float);
	int n_bytes_per_triangle = 3 * n_bytes_per_vertex;
	int n_total_triangles = 0;
	char filename[512];
	GLfloat **vertices = (GLfloat **)malloc(n_frames * sizeof(GLfloat *));

	for (i = 0; i < n_frames; i++) {
		sprintf(filename, filename_format, i);
		n_triangles[i] = read_geometry(&vertices[i], n_bytes_per_triangle, filename);
		n_total_triangles += n_triangles[i];
		if (i == 0)
			vertex_offset[i] = 0;
		else
			vertex_offset[i] = vertex_offset[i - 1] + 3 * n_triangles[i - 1];
	}

	glGenBuffers(1, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);
	for (i = 0; i < n_frames; i++)
		glBufferSubData(GL_ARRAY_BUFFER, vertex_offset[i] * n_bytes_per_vertex,
			n_triangles[i] * n_bytes_per_triangle, vertices[i]);
	for (i = 0; i < n_frames; i++)
		free(vertices[i]);
	free(vertices);

	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(LOC_POSITION);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(LOC_NORMAL);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(LOC_TEXCOORD);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_dynamic_object(GLuint VAO, int *vertex_offset, int *n_triangles, int frame, GLuint texture_name) {
	set_texture_mapping(states.use_texture_objects, texture_name);
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	glFrontFace(GL_CW);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, vertex_offset[frame], 3 * n_triangles[frame]);
	glBindVertexArray(0);
}

void draw_dynamic_objects(void) {
	glUseProgram(h_ShaderProgram_PS);
	if (states.polygon_fill_mode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	set_material_default();

	int sphase = states.anim_tick % SPIDER_BOUNCE_PERIOD;
	int shalf = SPIDER_BOUNCE_PERIOD / 2;
	float st = (sphase < shalf) ? (float)sphase / shalf : 2.0f - (float)sphase / shalf;
	float spider_y = SPIDER_Y_MIN + (SPIDER_Y_MAX - SPIDER_Y_MIN) * st;

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(SPIDER_WALL_X, spider_y, -FLOOR_SIZE / 2.0f + 30.0f));
	if (sphase >= shalf)
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(120.0f, 120.0f, 120.0f));
	if (reflection_pass_active) {
		float spider_reflection_alpha = 1.0f - spider_y / REFLECTION_FADE_HEIGHT;
		if (spider_reflection_alpha < 0.0f) spider_reflection_alpha = 0.0f;
		glUniform1f(loc_u_alpha, spider_reflection_alpha);
	}
	draw_dynamic_object(spider_VAO, spider_vertex_offset, spider_n_triangles, states.cur_frame_spider, texture_spider);
	if (reflection_pass_active)
		glUniform1f(loc_u_alpha, 1.0f);

	int wphase = states.anim_tick % WOLF_PATROL_PERIOD;
	int whalf = WOLF_PATROL_PERIOD / 2;
	float wt = (wphase < whalf) ? (float)wphase / whalf : 2.0f - (float)wphase / whalf;
	float wolf_x = WOLF_X_MIN + (WOLF_X_MAX - WOLF_X_MIN) * wt;
	float wolf_heading = (wphase < whalf) ? 90.0f : -90.0f;

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(wolf_x, 0.0f, WOLF_Z));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, wolf_heading * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(180.0f, 180.0f, 180.0f));
	draw_dynamic_object(wolf_VAO, wolf_vertex_offset, wolf_n_triangles, states.cur_frame_wolf, texture_wolf);

	glUseProgram(0);
}

void draw_scene_objects(void) {
	glUseProgram(h_ShaderProgram_PS);
	if (states.polygon_fill_mode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	set_material_tiger(1, 1, 1);
	set_texture_mapping(states.use_texture_objects, texture_tiger);
	ModelViewMatrix = glm::rotate(ViewMatrix, -states.rotation_angle_tiger * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	float jump_height_x = (states.rotation_angle_tiger % 180) / 180.0f;
	float jump_height_y = 4.0f * 100.0f * jump_height_x * (1.0f - jump_height_x);
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(500.0f, jump_height_y, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	if (reflection_pass_active && states.use_blending)
		glUniform1f(loc_u_alpha, states.tiger_opacity);
	if (shadow_pass_active && states.use_blending)
		glUniform1f(loc_u_caster_alpha, states.tiger_opacity);
	draw_tiger();
	if (reflection_pass_active && states.use_blending)
		glUniform1f(loc_u_alpha, 1.0f);
	if (shadow_pass_active && states.use_blending)
		glUniform1f(loc_u_caster_alpha, 1.0f);
	glUseProgram(0);

	draw_static_objects();
	draw_dynamic_objects();
}

// callbacks
void display(void) {
	glm::mat4 light_view_projection_matrix(1.0f);
	if (states.use_shadow) {
		glm::vec3 light_world_position = glm::vec3(glm::rotate(glm::mat4(1.0f),
			states.revolution_angle_WC * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(LIGHT_POS_IN_WC, 1.0f));
		glm::mat4 light_view_matrix = glm::lookAt(light_world_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 light_projection_matrix = glm::ortho(-2000.0f, 2000.0f, -2000.0f, 2000.0f, 1.0f, 5000.0f);
		light_view_projection_matrix = light_projection_matrix * light_view_matrix;

		glUseProgram(h_ShaderProgram_PS);
		glUniform1i(loc_u_flag_shadow, 0);
		glUseProgram(0);

		glm::mat4 saved_ViewMatrix = ViewMatrix;
		glm::mat4 saved_ProjectionMatrix = ProjectionMatrix;
		ViewMatrix = light_view_matrix;
		ProjectionMatrix = light_projection_matrix;

		glBindFramebuffer(GL_FRAMEBUFFER, shadow_FBO);
		glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(2.0f, 4.0f);
		shadow_pass_active = 1;
		draw_scene_objects();
		shadow_pass_active = 0;
		glPolygonOffset(0.0f, 0.0f);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window_width, window_height);

		ViewMatrix = saved_ViewMatrix;
		ProjectionMatrix = saved_ProjectionMatrix;
	}

	if (states.use_reflection) {
		glm::mat4 saved_ViewMatrix = ViewMatrix;
		ViewMatrix = glm::scale(ViewMatrix, glm::vec3(1.0f, -1.0f, 1.0f));

		glUseProgram(h_ShaderProgram_PS);
		glUniform1i(loc_u_flag_shadow, states.use_shadow);
		if (states.use_shadow) {
			glm::mat4 shadow_matrix = light_view_projection_matrix * glm::inverse(ViewMatrix);
			glUniformMatrix4fv(loc_u_shadow_matrix, 1, GL_FALSE, &shadow_matrix[0][0]);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, shadow_map);
			glUniform1i(loc_u_shadow_map, 2);
			glActiveTexture(GL_TEXTURE0);
		}
		glUseProgram(0);

		glBindFramebuffer(GL_FRAMEBUFFER, reflection_FBO);
		glViewport(0, 0, window_width, window_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		reflection_pass_active = 1;
		draw_scene_objects();
		reflection_pass_active = 0;
		glEnable(GL_CULL_FACE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window_width, window_height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		ViewMatrix = saved_ViewMatrix;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(h_ShaderProgram_PS);
	glUniform1i(loc_u_flag_shadow, states.use_shadow);
	if (states.use_shadow) {
		glm::mat4 shadow_matrix = light_view_projection_matrix * glm::inverse(ViewMatrix);
		glUniformMatrix4fv(loc_u_shadow_matrix, 1, GL_FALSE, &shadow_matrix[0][0]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, shadow_map);
		glUniform1i(loc_u_shadow_map, 2);
		glActiveTexture(GL_TEXTURE0);
	}
	glUseProgram(0);

	// Draw the WC axes.
	glUseProgram(h_ShaderProgram_simple);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
 	draw_axes();
	glLineWidth(1.0f);
	glUseProgram(0);
	
	if (states.light_geometry_space == LIGHT_IN_EC) {
		// Draw the light position in EC.
		glUseProgram(h_ShaderProgram_simple);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		ModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(LIGHT_POS_IN_EC));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glLineWidth(2.0f);
		draw_axes();
		glLineWidth(1.0f);
		glUseProgram(0);
	}

	if (states.light_geometry_space == LIGHT_IN_WC) {
		glm::mat4 Light_Revolution = glm::rotate(glm::mat4(1.0f), states.revolution_angle_WC * TO_RADIAN,
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 Light_Position_WC = glm::vec3(Light_Revolution * glm::vec4(LIGHT_POS_IN_WC, 1.0f));

		glUseProgram(h_ShaderProgram_PS);
		glm::vec4 Light_Position_EC = ViewMatrix * glm::vec4(Light_Position_WC, 1.0f);
		glUniform4fv(loc_Light_Position, 1, &Light_Position_EC[0]);
		glm::vec3 Spot_Direction_EC = glm::mat3(ViewMatrix) * glm::vec3(LIGHT_SPOT_DIRECTION_IN_WC);
		glUniform3fv(loc_Light_Spot_direction, 1, &Spot_Direction_EC[0]);
		glUseProgram(0);

		// Draw the light position in WC.
		glUseProgram(h_ShaderProgram_simple);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		ModelViewMatrix = glm::translate(ViewMatrix, Light_Position_WC);
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glLineWidth(2.0f);
		draw_axes();
		glLineWidth(1.0f);
		glUseProgram(0);
	}

	// Draw the circling tiger with full shading.	
	glUseProgram(h_ShaderProgram_PS);
	if (states.polygon_fill_mode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	set_material_tiger(1, 1, 1);
	ModelViewMatrix = glm::rotate(ViewMatrix, -states.rotation_angle_tiger * TO_RADIAN, 
		glm::vec3(0.0f, 1.0f, 0.0f));
	float jump_height_x = (states.rotation_angle_tiger % 180) / 180.0f;
	float jump_height_y = 4.0f * 100.0f * jump_height_x * (1.0f - jump_height_x); // a simple jump function
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(500.0f, jump_height_y, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
 
	if (states.light_geometry_space == LIGHT_IN_MC) {
		glm::vec4 Light_Position_EC = ModelViewMatrix * glm::vec4(LIGHT_POS_IN_MC_MOVING_TIGER, 1.0f);  
		glUniform4fv(loc_Light_Position, 1, &Light_Position_EC[0]);
		if (states.use_spot_light_for_circling_tiger_MC) {
			glm::vec3 Spot_Direction_EC = glm::mat3(ModelViewMatrix) * glm::vec3(LIGHT_SPOT_DIRECTION_IN_TIGER_MC);
			glUniform3fv(loc_Light_Spot_direction, 1, &Spot_Direction_EC[0]);
		}
	}

	tiger_ModelViewMatrix = ModelViewMatrix;
	if (!states.use_blending) {
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		set_texture_mapping(states.use_texture_objects, texture_tiger);
		draw_tiger();
	}
	glUseProgram(0);

	if (states.light_geometry_space == LIGHT_IN_MC) {
		// Draw the light position in MC.
		glUseProgram(h_ShaderProgram_simple);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		ModelViewProjectionMatrix = glm::translate(ModelViewProjectionMatrix, 
			glm::vec3(LIGHT_POS_IN_MC_MOVING_TIGER));
		ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glLineWidth(2.0f);
		draw_axes();
		glLineWidth(1.0f);
		glUseProgram(0);
	}

	// Draw the floor and a billboard.
	glUseProgram(h_ShaderProgram_PS);
	if (states.polygon_fill_mode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	set_material_floor();
	set_texture_mapping(states.use_texture_floor_wall, texture_floor);
	glUniform1i(loc_u_flag_reflection, states.use_reflection);
	if (states.use_reflection) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, reflection_texture);
		glUniform1i(loc_u_reflection_texture, 1);
		glActiveTexture(GL_TEXTURE0);
	}
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-FLOOR_SIZE/2.0f, 0.0f, FLOOR_SIZE/2.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(FLOOR_SIZE, FLOOR_SIZE, FLOOR_SIZE));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_floor();

	glUniform1i(loc_u_flag_reflection, 0);
	set_texture_mapping(states.use_texture_floor_wall, texture_wall);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-FLOOR_SIZE/2.0f, 0.0f, -FLOOR_SIZE/2.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(FLOOR_SIZE, WALL_HEIGHT, 1.0));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_floor();  // use the same floor geometry for drawing walls.

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-FLOOR_SIZE/2.0f, 0.0f, FLOOR_SIZE/2.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(1.0f, WALL_HEIGHT, FLOOR_SIZE));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_floor();  // use the same floor geometry for drawing walls.
	glUseProgram(0);

	draw_static_objects();
	draw_dynamic_objects();

	if (states.use_blending) {
		glUseProgram(h_ShaderProgram_PS);
		if (states.polygon_fill_mode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		set_material_tiger(1, 1, 1);
		set_texture_mapping(states.use_texture_objects, texture_tiger);
		glUniform1f(loc_u_alpha, states.tiger_opacity);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		ModelViewMatrix = tiger_ModelViewMatrix;
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		draw_tiger();
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glUniform1f(loc_u_alpha, 1.0f);
		glUseProgram(0);
	}
	glutSwapBuffers();
}

void timer_scene(int value) {
	static unsigned int _timestamp_scene = 0;
	states.cur_frame_tiger = _timestamp_scene % N_TIGER_FRAMES;
	states.rotation_angle_tiger = _timestamp_scene % 360;
	states.cur_frame_spider = _timestamp_scene % N_SPIDER_FRAMES;
	states.cur_frame_wolf = _timestamp_scene % N_WOLF_FRAMES;
	states.anim_tick = _timestamp_scene;
	glutPostRedisplay();
	if (states.animation_mode) {
		_timestamp_scene++;
		glutTimerFunc(100, timer_scene, 0);
	}
}

void timer_WC_light_revolution(int value) {
	if (states.use_circular_motion_for_WC_light) {
		states.revolution_angle_WC = (states.revolution_angle_WC + WC_LIGHT_REVOLUTION_STEP) % 360;
		glutPostRedisplay();
		glutTimerFunc(50, timer_WC_light_revolution, 0);
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups
		break;
	case 'a':
		states.animation_mode = 1 - states.animation_mode;
		if (states.animation_mode) {
			glutTimerFunc(100, timer_scene, 0);
		}
		break;
	case 'p':
		states.polygon_fill_mode = 1 - states.polygon_fill_mode;
		if (states.polygon_fill_mode)  
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else 
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glutPostRedisplay();
		break;
	case 'v':
		states.use_halfway_vector = 1 - states.use_halfway_vector;
		if (states.use_halfway_vector)
			fprintf(stdout, "^^^ Using the halfway vector for specular reflection...\n");
		else
			fprintf(stdout, "^^^ Using the reflection vector for specular reflection...\n");
		glUseProgram(h_ShaderProgram_PS);
		glUniform1i(loc_use_halfway_vector, states.use_halfway_vector);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 'h':
		states.use_blending = 1 - states.use_blending;
		if (states.use_blending)
			fprintf(stdout, "^^^ Blending (transparency) for the tiger is ON. Opacity = %.2f\n", states.tiger_opacity);
		else
			fprintf(stdout, "^^^ Blending (transparency) for the tiger is OFF.\n");
		glutPostRedisplay();
		break;
	case 'i':
		if (!states.use_blending) break;
		states.tiger_opacity += 0.05f;
		if (states.tiger_opacity > 1.0f) states.tiger_opacity = 1.0f;
		fprintf(stdout, "^^^ Tiger opacity = %.2f\n", states.tiger_opacity);
		glutPostRedisplay();
		break;
	case 'j':
		if (!states.use_blending) break;
		states.tiger_opacity -= 0.05f;
		if (states.tiger_opacity < 0.0f) states.tiger_opacity = 0.0f;
		fprintf(stdout, "^^^ Tiger opacity = %.2f\n", states.tiger_opacity);
		glutPostRedisplay();
		break;
	case 'l':
		states.light_geometry_space = (states.light_geometry_space + 1) % 3;
		if (states.light_geometry_space == LIGHT_IN_EC) {
			states.use_spot_light_for_EC_light = 0;
			glUseProgram(h_ShaderProgram_PS);
			glUniform3f(loc_Light_L, DIRECT_LIGHT_L_IN_EC);
			glUniform1i(loc_apply_spot_light, states.use_spot_light_for_EC_light);
			glUniform1i(loc_apply_attenuation, 0);
			glm::vec4 Light_Position_EC = { LIGHT_POS_IN_EC, 1.0f };
			glUniform4fv(loc_Light_Position, 1, &Light_Position_EC[0]);
			glm::vec3 Spot_Direction_EC = glm::vec3(LIGHT_SPOT_DIRECTION_IN_EC);
			glUniform3fv(loc_Light_Spot_direction, 1, &Spot_Direction_EC[0]);
			glUseProgram(0);
			fprintf(stdout, "^^^ The light is defined in EC.\n");
		}
		else if (states.light_geometry_space == LIGHT_IN_WC) {
			// Could be more effcient...
			glUseProgram(h_ShaderProgram_PS);
			glUniform3f(loc_Light_L, DIRECT_LIGHT_L_IN_WC);
			states.use_spot_light_for_WC_light = 0;
			states.use_circular_motion_for_WC_light = 0;
			states.revolution_angle_WC = 0;
			glUniform1i(loc_apply_spot_light, states.use_spot_light_for_WC_light);
			glUniform1i(loc_apply_attenuation, 0);
			glm::vec4 Light_Position_EC = ViewMatrix * glm::vec4(LIGHT_POS_IN_WC, 1.0f);
			glUniform4fv(loc_Light_Position, 1, &Light_Position_EC[0]);
			glm::vec3 Spot_Direction_EC = glm::mat3(ViewMatrix) * glm::vec3(LIGHT_SPOT_DIRECTION_IN_WC);
			glUniform3fv(loc_Light_Spot_direction, 1, &Spot_Direction_EC[0]);
			glUseProgram(0);
			fprintf(stdout, "^^^ The light is defined in WC.\n");
		}
		else {
			// The moving light in MC is updated in display callback, so do nothing here.	
			states.use_spot_light_for_circling_tiger_MC = 0;
			states.use_attenuation_for_MC_light = 0;
			glUseProgram(h_ShaderProgram_PS);
			glUniform3f(loc_Light_L, DIRECT_LIGHT_L_IN_MC);
			glUniform1i(loc_apply_spot_light, states.use_spot_light_for_circling_tiger_MC);
			glUniform1i(loc_apply_attenuation, states.use_attenuation_for_MC_light);
			glUseProgram(0);
			fprintf(stdout, "^^^ The light is defined in MC.\n");
		}
		glutPostRedisplay();
		break;
	case 'm':
		if (states.light_geometry_space != LIGHT_IN_MC) break;
		states.use_spot_light_for_circling_tiger_MC = 1 - states.use_spot_light_for_circling_tiger_MC;
		glUseProgram(h_ShaderProgram_PS);
		glUniform1i(loc_apply_spot_light, states.use_spot_light_for_circling_tiger_MC);
		glUseProgram(0);
		if (states.use_spot_light_for_circling_tiger_MC) {
			fprintf(stdout, "^^^ Applying the spot light feature for the light defined for the circling tiger...\n");
		}
		else {
			fprintf(stdout, "^^^ Not applying the spot light feature for the light defined for the circling tiger...\n");
		}
		glutPostRedisplay();
		break;
	case 'b':
		if (states.light_geometry_space != LIGHT_IN_MC) break;
		states.use_attenuation_for_MC_light = 1 - states.use_attenuation_for_MC_light;
		glUseProgram(h_ShaderProgram_PS);
		glUniform1i(loc_apply_attenuation, states.use_attenuation_for_MC_light);
		glUseProgram(0);
		if (states.use_attenuation_for_MC_light) {
			fprintf(stdout, "^^^ Applying the light attenuation for the light defined in the tiger's MC...\n");
		}
		else {
			fprintf(stdout, "^^^ Not applying the light attenuation for the light defined in the tiger's MC...\n");
		}
		glutPostRedisplay();
		break;
	case 'd':
		if (states.light_geometry_space != LIGHT_IN_WC) break;
		states.use_circular_motion_for_WC_light = 1 - states.use_circular_motion_for_WC_light;
		if (states.use_circular_motion_for_WC_light) {
			glutTimerFunc(50, timer_WC_light_revolution, 0);
			fprintf(stdout, "^^^ Starting the circular motion for the light defined in WC...\n");
		}
		else {
			fprintf(stdout, "^^^ Stopping the circular motion for the light defined in WC...\n");
		}
		glutPostRedisplay();
		break;
	case 'w':
		if (states.light_geometry_space != LIGHT_IN_WC) break;
		states.use_spot_light_for_WC_light = 1 - states.use_spot_light_for_WC_light;
		glUseProgram(h_ShaderProgram_PS);
		glUniform1i(loc_apply_spot_light, states.use_spot_light_for_WC_light);
		glUseProgram(0);
		if (states.use_spot_light_for_WC_light) {
			fprintf(stdout, "^^^ Applying the spot light feature for the light defined in WC...\n");
		}
		else {
			fprintf(stdout, "^^^ Not applying the spot light feature for the light defined in WC......\n");
		}	
		glutPostRedisplay();
		break;
	case 'e':
		if (states.light_geometry_space != LIGHT_IN_EC) break;
		states.use_spot_light_for_EC_light = 1 - states.use_spot_light_for_EC_light;
		glUseProgram(h_ShaderProgram_PS);
		glUniform1i(loc_apply_spot_light, states.use_spot_light_for_EC_light);
		glUseProgram(0);
		if (states.use_spot_light_for_EC_light) {
			fprintf(stdout, "^^^ Applying the spot light feature for the light defined in EC...\n");
		}
		else {
			fprintf(stdout, "^^^ Not applying the spot light feature for the light defined in EC......\n");
		}
		glutPostRedisplay();
		break;
	case 'f':
		states.use_texture_objects = 1 - states.use_texture_objects;
		if (states.use_texture_objects)
			fprintf(stdout, "^^^ Applying textures to the tiger and placed objects...\n");
		else
			fprintf(stdout, "^^^ Not applying textures to the tiger and placed objects...\n");
		glutPostRedisplay();
		break;
	case 'g':
		states.use_texture_floor_wall = 1 - states.use_texture_floor_wall;
		if (states.use_texture_floor_wall)
			fprintf(stdout, "^^^ Applying textures to the floor and walls...\n");
		else
			fprintf(stdout, "^^^ Not applying textures to the floor and walls...\n");
		glutPostRedisplay();
		break;
	case 'r':
		states.use_reflection = 1 - states.use_reflection;
		if (states.use_reflection)
			fprintf(stdout, "^^^ Mirror floor reflection is ON.\n");
		else
			fprintf(stdout, "^^^ Mirror floor reflection is OFF.\n");
		glutPostRedisplay();
		break;
	case 'k':
		states.use_shadow = 1 - states.use_shadow;
		if (states.use_shadow)
			fprintf(stdout, "^^^ Shadow mapping (from the WC light) is ON.\n");
		else
			fprintf(stdout, "^^^ Shadow mapping is OFF.\n");
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

void initialize_camera(void) {
	camera_PRP = glm::vec3(INITIAL_CAMERA_PRP);
	camera_VRP = glm::vec3(INITIAL_CAMERA_VRP);
	ViewMatrix = glm::lookAt(camera_PRP, camera_VRP, glm::vec3(0.0f, 1.0f, 0.0f));
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			left_button_pressed = 1;
			mouse_prev_x = x;
			mouse_prev_y = y;
		}
		else {
			left_button_pressed = 0;
		}
	}
}

void motion(int x, int y) {
	if (!left_button_pressed) return;

	int dx = x - mouse_prev_x;
	int dy = y - mouse_prev_y;
	mouse_prev_x = x;
	mouse_prev_y = y;

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 forward = glm::normalize(camera_VRP - camera_PRP);
	glm::vec3 u_axis = glm::normalize(glm::cross(forward, up));
	glm::vec3 v_axis = glm::cross(u_axis, forward);

	camera_VRP += ((float)dx * u_axis - (float)dy * v_axis) * CAMERA_VRP_MOVE_SPEED;

	ViewMatrix = glm::lookAt(camera_PRP, camera_VRP, up);

	glutPostRedisplay();
}

#define INITIAL_FOVY_RAD		35.0f * TO_RADIAN
#define MINIMUM_FOVY_RAD		5.0f * TO_RADIAN
#define MAXIMUM_FOVY_RAD		60.0f * TO_RADIAN
#define ZOOM_MULTIPLIER			1.02f
float  zoom_factor;
float aspect_ratio; // set in reshape callback
void wheel(int wheel, int direction, int x, int y) {
	if (direction == -1) { // zoom out
		if (INITIAL_FOVY_RAD * zoom_factor * ZOOM_MULTIPLIER <= MAXIMUM_FOVY_RAD) {
			zoom_factor *= ZOOM_MULTIPLIER;
			ProjectionMatrix = glm::perspective(zoom_factor * INITIAL_FOVY_RAD, aspect_ratio, 10.0f, 5000.0f);
			glutPostRedisplay();
		}
	}
	else { // zoom in
		if (INITIAL_FOVY_RAD * zoom_factor / ZOOM_MULTIPLIER >= MINIMUM_FOVY_RAD) {
			zoom_factor /= ZOOM_MULTIPLIER;
			ProjectionMatrix = glm::perspective(zoom_factor * INITIAL_FOVY_RAD, aspect_ratio, 10.0f, 5000.0f);
			glutPostRedisplay();
		}
	}
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	aspect_ratio = (float) width / height;
	ProjectionMatrix = glm::perspective(zoom_factor * INITIAL_FOVY_RAD, aspect_ratio, 10.0f, 5000.0f);

	window_width = width;
	window_height = height;
	glBindTexture(GL_TEXTURE_2D, reflection_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, reflection_depth_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glUseProgram(h_ShaderProgram_PS);
	glUniform2f(loc_u_window_size, (float)width, (float)height);
	glUseProgram(0);

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);
	glDeleteVertexArrays(1, &rectangle_VAO);
	glDeleteBuffers(1, &rectangle_VBO);
	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);
	glDeleteVertexArrays(1, &cat_VAO);
	glDeleteBuffers(1, &cat_VBO);
	glDeleteVertexArrays(1, &ant_VAO);
	glDeleteBuffers(1, &ant_VBO);
	glDeleteVertexArrays(1, &spider_VAO);
	glDeleteBuffers(1, &spider_VBO);
	glDeleteVertexArrays(1, &wolf_VAO);
	glDeleteBuffers(1, &wolf_VBO);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(wheel);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info_simple[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_PS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Simple_Phong.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Simple_Phong.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info_simple);
	loc_primitive_color_simple = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");
	loc_ModelViewProjectionMatrix_simple = glGetUniformLocation(h_ShaderProgram_simple, 
		"u_ModelViewProjectionMatrix");

	h_ShaderProgram_PS = LoadShaders(shader_info_PS);
	loc_ModelViewProjectionMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "ModelViewProjectionMatrix");
	loc_ModelViewMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_PS = glGetUniformLocation(h_ShaderProgram_PS, "ModelViewMatrixInvTrans");

	loc_Light_Position = glGetUniformLocation(h_ShaderProgram_PS, "Light.Position");
	loc_Light_La = glGetUniformLocation(h_ShaderProgram_PS, "Light.La");
	loc_Light_L = glGetUniformLocation(h_ShaderProgram_PS, "Light.L");
	loc_Light_Spot_direction = glGetUniformLocation(h_ShaderProgram_PS, "Light.Spot_direction");
	loc_Light_Spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_PS, "Light.Spot_cutoff_angle");
	loc_Light_Spot_exponent = glGetUniformLocation(h_ShaderProgram_PS, "Light.Spot_exponent");
	loc_Light_Attenuation_factors = glGetUniformLocation(h_ShaderProgram_PS, "Light.Attenuation_factors");

	loc_Material_Ka = glGetUniformLocation(h_ShaderProgram_PS, "Material.Ka");
	loc_Material_Kd = glGetUniformLocation(h_ShaderProgram_PS, "Material.Kd");
	loc_Material_Ks = glGetUniformLocation(h_ShaderProgram_PS, "Material.Ks");
	loc_Material_Shininess = glGetUniformLocation(h_ShaderProgram_PS, "Material.Shininess");

	loc_use_halfway_vector = glGetUniformLocation(h_ShaderProgram_PS, "use_halfway_vector");
	loc_apply_spot_light = glGetUniformLocation(h_ShaderProgram_PS, "apply_spot_light");
	loc_apply_attenuation = glGetUniformLocation(h_ShaderProgram_PS, "apply_attenuation");
	loc_u_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_PS, "u_flag_texture_mapping");
	loc_u_base_texture = glGetUniformLocation(h_ShaderProgram_PS, "u_base_texture");
	loc_u_alpha = glGetUniformLocation(h_ShaderProgram_PS, "u_alpha");
	loc_u_flag_reflection = glGetUniformLocation(h_ShaderProgram_PS, "u_flag_reflection");
	loc_u_reflection_texture = glGetUniformLocation(h_ShaderProgram_PS, "u_reflection_texture");
	loc_u_window_size = glGetUniformLocation(h_ShaderProgram_PS, "u_window_size");
	loc_u_flag_shadow = glGetUniformLocation(h_ShaderProgram_PS, "u_flag_shadow");
	loc_u_shadow_map = glGetUniformLocation(h_ShaderProgram_PS, "u_shadow_map");
	loc_u_shadow_matrix = glGetUniformLocation(h_ShaderProgram_PS, "u_shadow_matrix");
	loc_u_caster_alpha = glGetUniformLocation(h_ShaderProgram_PS, "u_caster_alpha");
}

void initialize_OpenGL(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	if (states.polygon_fill_mode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUseProgram(h_ShaderProgram_PS);
	glUniform3f(loc_Light_La, 0.5f, 0.5f, 0.5f);
	glUniform1f(loc_Light_Spot_cutoff_angle, LIGHT_SPOT_CUTOFF_ANGLE);
	glUniform1f(loc_Light_Spot_exponent, LIGHT_SPOT_EXPONENT);
	glUniform3f(loc_Light_Attenuation_factors, LIGHT_ATTENUATION_FACTORS_MC);
	glUniform1i(loc_apply_attenuation, 0);
	glUniform1f(loc_u_alpha, 1.0f);
	glUniform1i(loc_u_flag_reflection, 0);
	glUniform2f(loc_u_window_size, (float)window_width, (float)window_height);
	glUniform1i(loc_u_flag_shadow, 0);
	glUniform1f(loc_u_caster_alpha, 1.0f);
	fprintf(stdout, "^^^ Using the reflection vector for specular reflection...\n");
	glUseProgram(0);

	states.light_geometry_space = LIGHT_IN_EC;
	states.use_spot_light_for_EC_light = 0;
	// If static, do this once after the shader program is prepared, and before rendering the scene.
	// You can move this light relative to the camera. 	
	glUseProgram(h_ShaderProgram_PS);
	glUniform3f(loc_Light_L, DIRECT_LIGHT_L_IN_EC);
	glUniform1i(loc_apply_spot_light, states.use_spot_light_for_EC_light);
	glm::vec4 Light_Position_EC = { LIGHT_POS_IN_EC, 1.0f };
	glUniform4fv(loc_Light_Position, 1, &Light_Position_EC[0]);
	glm::vec3 Spot_Direction_EC = glm::vec3(LIGHT_SPOT_DIRECTION_IN_EC);
	glUniform3fv(loc_Light_Spot_direction, 1, &Spot_Direction_EC[0]);
	glUseProgram(0);
	fprintf(stdout, "^^^ The light is defined in EC.\n");

	initialize_camera();
	zoom_factor = 1.0f;
}

void prepare_scene(void) {
	prepare_axes();
	prepare_floor();
	prepare_tiger();
	prepare_static_object(&cat_VBO, &cat_VAO, &cat_n_triangles, "Data/static_objects/cat_vnt.geom");
	prepare_static_object(&ant_VBO, &ant_VAO, &ant_n_triangles, "Data/static_objects/ant_vnt.geom");
	prepare_dynamic_object(&spider_VBO, &spider_VAO, spider_n_triangles, spider_vertex_offset,
		N_SPIDER_FRAMES, "Data/dynamic_objects/spider/spider_vnt_%02d.geom");
	prepare_dynamic_object(&wolf_VBO, &wolf_VAO, wolf_n_triangles, wolf_vertex_offset,
		N_WOLF_FRAMES, "Data/dynamic_objects/wolf/wolf_%02d_vnt.geom");
	prepare_textures();
	prepare_reflection();
	prepare_shadow();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error = glewInit();
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

#define N_MESSAGE_LINES 2
int main(int argc, char *argv[]) {
	char program_name[128] = "Sogang CSE4170/AIE4012 5.3.2.Tiger_Simple_Shading_PS_GLSL_Wheel_26_Single";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'a', 'p', 'v', 'l', 'm', 'w', 'e', 'b', 'd', 'f', 'g', 'h', 'i', 'j', 'r', 'k', 'ESC'",  "    - Mouse wheel: zoom, Left drag: change gaze direction" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1280, 1024);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
	return 1;
}