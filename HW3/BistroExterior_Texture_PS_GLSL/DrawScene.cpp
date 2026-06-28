//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2023 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadScene.h"
#include "MyObjects.h"
#include "SceneTypes.h"
#include "GameMode.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

extern SCENE scene;

// for simple shaders
GLuint h_ShaderProgram_simple; // handle to shader program
GLuint h_ShaderProgram_background, h_ShaderProgram_equiToCube;
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// for PBR
GLuint h_ShaderProgram_TXPBR;
#define NUMBER_OF_LIGHT_SUPPORTED 1
GLint loc_global_ambient_color;
GLint loc_lightCount;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPBR, loc_ModelViewMatrix_TXPBR, loc_ModelViewMatrixInvTrans_TXPBR;
GLint loc_cameraPos;

#define TEXTURE_INDEX_DIFFUSE	(0)
#define TEXTURE_INDEX_NORMAL	(1)
#define TEXTURE_INDEX_SPECULAR	(2)
#define TEXTURE_INDEX_EMISSIVE	(3)
#define TEXTURE_INDEX_SKYMAP	(4)

// for skybox shaders
GLuint h_ShaderProgram_skybox;
GLint loc_cubemap_skybox;
GLint loc_ModelViewProjectionMatrix_SKY;

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.
glm::mat4 ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

SCENE_MODE current_mode = MODE_NORMAL;

/*********************************  START: camera *********************************/
Camera camera_info[NUM_CAMERAS];
Camera camera_info_default[NUM_CAMERAS];
Camera current_camera;

int current_camera_index = CAMERA_U;

bool is_world_observation_camera(void) {
	return (current_camera_index == CAMERA_U ||
			current_camera_index == CAMERA_I ||
			current_camera_index == CAMERA_O ||
			current_camera_index == CAMERA_P);
}

using glm::mat4;
void set_ViewMatrix_from_camera_frame(void) {
	ViewMatrix = glm::mat4(current_camera.uaxis[0], current_camera.vaxis[0], current_camera.naxis[0], 0.0f,
		current_camera.uaxis[1], current_camera.vaxis[1], current_camera.naxis[1], 0.0f,
		current_camera.uaxis[2], current_camera.vaxis[2], current_camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-current_camera.pos[0], -current_camera.pos[1], -current_camera.pos[2]));
}

void set_current_camera(int camera_num) {
	Camera* pCamera = &camera_info[camera_num];

	current_camera_index = camera_num;
	memcpy(&current_camera, pCamera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();
	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera(void) {
	Camera *pCamera = &camera_info[CAMERA_U];
	pCamera->pos[0] = -800.0f; pCamera->pos[1] = 200.0f; pCamera->pos[2] = 2000.0f;
	pCamera->uaxis[0] = -0.3f; pCamera->uaxis[1] = -0.9f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.9f; pCamera->vaxis[1] = -0.3f; pCamera->vaxis[2] = 0.3f;
	pCamera->naxis[0] = -0.3f; pCamera->naxis[1] = 0.1f; pCamera->naxis[2] = 1.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * 105.0f; pCamera->aspect_ratio = scene.camera.aspect; pCamera->near_c = 50.0f; pCamera->far_c = 50000.0f;

	pCamera = &camera_info[CAMERA_I];
	pCamera->pos[0] = 1300.0f; pCamera->pos[1] = 3800.0f; pCamera->pos[2] = 800.0f;
	pCamera->uaxis[0] = -0.8f; pCamera->uaxis[1] = -0.6f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = -0.6f; pCamera->naxis[1] = 0.8f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * 100.0f; pCamera->aspect_ratio = scene.camera.aspect; pCamera->near_c = 50.0f; pCamera->far_c = 50000.0f;

	pCamera = &camera_info[CAMERA_O];
	pCamera->pos[0] = 6000.0f; pCamera->pos[1] = -1480.0f; pCamera->pos[2] = 650.0f;
	pCamera->uaxis[0] = -0.701f; pCamera->uaxis[1] = 0.712f; pCamera->uaxis[2] = -0.043f;
	pCamera->vaxis[0] = -0.082f; pCamera->vaxis[1] = -0.020f; pCamera->vaxis[2] = 0.996f;
	pCamera->naxis[0] = 0.708f; pCamera->naxis[1] = 0.702f; pCamera->naxis[2] = 0.072f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * 70.0f; pCamera->aspect_ratio = scene.camera.aspect; pCamera->near_c = 50.0f; pCamera->far_c = 50000.0f;

	pCamera = &camera_info[CAMERA_P];
	pCamera->pos[0] = 1250.0f; pCamera->pos[1] = 3000.0f; pCamera->pos[2] = 640.0f;
	pCamera->uaxis[0] = -0.8f; pCamera->uaxis[1] = 0.6f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 0.6f; pCamera->naxis[1] = 0.8f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * 90.0f; pCamera->aspect_ratio = scene.camera.aspect; pCamera->near_c = 50.0f; pCamera->far_c = 50000.0f;

	pCamera = &camera_info[CAMERA_A];
	pCamera->pos[0] = -1600.0f; pCamera->pos[1] = 300.0f; pCamera->pos[2] = 850.0f;
	pCamera->uaxis[0] = 0.0f; pCamera->uaxis[1] = -1.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = -1.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * 90.0f; pCamera->aspect_ratio = scene.camera.aspect; pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	pCamera = &camera_info[CAMERA_T];
	pCamera->fovy = 70.0f * TO_RADIAN;
	pCamera->aspect_ratio = scene.camera.aspect;
	pCamera->near_c = 0.1f;
	pCamera->far_c = 50000.0f;
	pCamera->move = 0;

	pCamera = &camera_info[CAMERA_G];
	pCamera->fovy = 70.0f * TO_RADIAN;
	pCamera->aspect_ratio = scene.camera.aspect;
	pCamera->near_c = 0.1f;
	pCamera->far_c = 50000.0f;
	pCamera->move = 0;

	set_current_camera(CAMERA_U);

	memcpy(camera_info_default, camera_info, sizeof(camera_info));
}

void update_current_camera(void) {
	memcpy(&camera_info[current_camera_index], &current_camera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void print_camera_info(void) {
	fprintf(stdout, " * Camera position: (%f, %f, %f)\n", current_camera.pos[0], current_camera.pos[1], current_camera.pos[2]);
	fprintf(stdout, " * Camera orientation (u, v, n): (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)\n",
		current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2],
		current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2],
		current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
	fprintf(stdout, " * Camera fovy: %f (degrees: %f)\n", current_camera.fovy, current_camera.fovy * TO_DEGREE); // 추가

}
/*********************************  END: camera *********************************/

/******************************  START: shader setup ****************************/
// Begin of Callback function definitions
void prepare_shader_program(void) {
	char string[256];

	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram_simple);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");

	ShaderInfo shader_info_TXPBR[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Background/PBR_Tx.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Background/PBR_Tx.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_TXPBR = LoadShaders(shader_info_TXPBR);
	glUseProgram(h_ShaderProgram_TXPBR);

	loc_ModelViewProjectionMatrix_TXPBR = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPBR = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPBR = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_ModelViewMatrixInvTrans");

	loc_lightCount = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_light_count");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPBR, string);
		sprintf(string, "u_light[%d].color", i);
		loc_light[i].color = glGetUniformLocation(h_ShaderProgram_TXPBR, string);
	}

	loc_cameraPos = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_camPos");

	//Textures
	loc_material.diffuseTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_albedoMap");
	loc_material.normalTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_normalMap");
	loc_material.specularTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_metallicRoughnessMap");
	loc_material.emissiveTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_emissiveMap");

	ShaderInfo shader_info_skybox[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Background/skybox.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Background/skybox.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_skybox = LoadShaders(shader_info_skybox);
	loc_cubemap_skybox = glGetUniformLocation(h_ShaderProgram_skybox, "u_skymap");
	loc_ModelViewProjectionMatrix_SKY = glGetUniformLocation(h_ShaderProgram_skybox, "u_ModelViewProjectionMatrix");
}
/*******************************  END: shder setup ******************************/

/****************************  START: geometry setup ****************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION	0
#define INDEX_NORMAL			1
#define INDEX_TEX_COORD			2

bool b_draw_grid = false;

//axes
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded axes into graphics memory.\n");
}

void draw_axes(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(8000.0f, 8000.0f, 8000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//grid
#define GRID_LENGTH			(100)
#define NUM_GRID_VETICES	((2 * GRID_LENGTH + 1) * 4)
GLuint grid_VBO, grid_VAO;
GLfloat grid_vertices[NUM_GRID_VETICES][3];
GLfloat grid_color[3] = { 0.5f, 0.5f, 0.5f };

void prepare_grid(void) {

	//set grid vertices
	int vertex_idx = 0;
	for (int x_idx = -GRID_LENGTH; x_idx <= GRID_LENGTH; x_idx++)
	{
		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = -GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	for (int y_idx = -GRID_LENGTH; y_idx <= GRID_LENGTH; y_idx++)
	{
		grid_vertices[vertex_idx][0] = -GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &grid_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), &grid_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &grid_VAO);
	glBindVertexArray(grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VAO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	fprintf(stdout, " * Loaded grid into graphics memory.\n");
}

void draw_grid(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(1.0f);
	glBindVertexArray(grid_VAO);
	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, 0, NUM_GRID_VETICES);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

// bistro_exterior
GLuint* bistro_exterior_VBO;
GLuint* bistro_exterior_VAO;
int* bistro_exterior_n_triangles;
int* bistro_exterior_vertex_offset;
GLfloat** bistro_exterior_vertices;
GLuint* bistro_exterior_texture_names;

int flag_fog;
bool* flag_texture_mapping;

void initialize_lights(void) { // follow OpenGL conventions for initialization
	glUseProgram(h_ShaderProgram_TXPBR);

	glUniform1f(loc_lightCount, scene.n_lights);

	for (int i = 0; i < scene.n_lights; i++) {
		glUniform4f(loc_light[i].position,
			scene.light_list[i].pos[0],
			scene.light_list[i].pos[1],
			scene.light_list[i].pos[2],
			0.0f);

		glUniform3f(loc_light[i].color,
			scene.light_list[i].color[0],
			scene.light_list[i].color[1],
			scene.light_list[i].color[2]);
	}

	glUseProgram(0);
}

bool readTexImage2D_from_file(char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	if (tx_pixmap == NULL)
		return false;
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	GLenum format, internalFormat;
	if (tx_bits_per_pixel == 32) {
		format = GL_BGRA;
		internalFormat = GL_RGBA;
	}
	else if (tx_bits_per_pixel == 24) {
		format = GL_BGR;
		internalFormat = GL_RGB;
	}
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap = FreeImage_ConvertTo32Bits(tx_pixmap);
		format = GL_BGRA;
		internalFormat = GL_RGBA;
	}

	width = FreeImage_GetWidth(tx_pixmap);
	height = FreeImage_GetHeight(tx_pixmap);
	data = FreeImage_GetBits(tx_pixmap);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap);

	return true;
}

void prepare_bistro_exterior(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	// VBO, VAO malloc
	bistro_exterior_VBO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);
	bistro_exterior_VAO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);

	bistro_exterior_n_triangles = (int*)malloc(sizeof(int) * scene.n_materials);
	bistro_exterior_vertex_offset = (int*)malloc(sizeof(int) * scene.n_materials);

	flag_texture_mapping = (bool*)malloc(sizeof(bool) * scene.n_textures);

	// vertices
	bistro_exterior_vertices = (GLfloat**)malloc(sizeof(GLfloat*) * scene.n_materials);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		MATERIAL* pMaterial = &(scene.material_list[materialIdx]);
		GEOMETRY_TRIANGULAR_MESH* tm = &(pMaterial->geometry.tm);

		// vertex
		bistro_exterior_vertices[materialIdx] = (GLfloat*)malloc(sizeof(GLfloat) * 8 * tm->n_triangle * 3);

		int vertexIdx = 0;
		for (int triIdx = 0; triIdx < tm->n_triangle; triIdx++) {
			TRIANGLE tri = tm->triangle_list[triIdx];
			for (int triVertex = 0; triVertex < 3; triVertex++) {
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].x;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].y;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].z;

				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].x;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].y;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].z;

				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].u;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].v;
			}
		}

		// # of triangles
		bistro_exterior_n_triangles[materialIdx] = tm->n_triangle;

		if (materialIdx == 0)
			bistro_exterior_vertex_offset[materialIdx] = 0;
		else
			bistro_exterior_vertex_offset[materialIdx] = bistro_exterior_vertex_offset[materialIdx - 1] + 3 * bistro_exterior_n_triangles[materialIdx - 1];

		glGenBuffers(1, &bistro_exterior_VBO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, bistro_exterior_VBO[materialIdx]);
		glBufferData(GL_ARRAY_BUFFER, bistro_exterior_n_triangles[materialIdx] * 3 * n_bytes_per_vertex,
			bistro_exterior_vertices[materialIdx], GL_STATIC_DRAW);

		// As the geometry data exists now in graphics memory, ...
		free(bistro_exterior_vertices[materialIdx]);

		// Initialize vertex array object.
		glGenVertexArrays(1, &bistro_exterior_VAO[materialIdx]);
		glBindVertexArray(bistro_exterior_VAO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, bistro_exterior_VBO[materialIdx]);
		glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
		glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
		glVertexAttribPointer(INDEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_NORMAL);
		glVertexAttribPointer(INDEX_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_TEX_COORD);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if ((materialIdx > 0) && (materialIdx % 100 == 0))
			fprintf(stdout, " * Loaded %d bistro exterior materials into graphics memory.\n", materialIdx / 100 * 100);
	}
	fprintf(stdout, " * Loaded %d bistro exterior materials into graphics memory.\n", scene.n_materials);

	// textures
	bistro_exterior_texture_names = (GLuint*)malloc(sizeof(GLuint) * scene.n_textures);
	glGenTextures(scene.n_textures, bistro_exterior_texture_names);

	for (int texId = 0; texId < scene.n_textures; texId++) {
		glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[texId]);

		bool bReturn = readTexImage2D_from_file(scene.texture_file_name[texId]);

		if (bReturn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			//glGenerateMipmap(GL_TEXTURE_2D);
			flag_texture_mapping[texId] = true;
		}
		else {
			flag_texture_mapping[texId] = false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	fprintf(stdout, " * Loaded bistro exterior textures into graphics memory.\n");

	free(bistro_exterior_vertices);
}

void bindTexture(GLuint tex, int glTextureId, int texId) {
	if (INVALID_TEX_ID != texId) {
		glActiveTexture(GL_TEXTURE0 + glTextureId);
		glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[texId]);
		glUniform1i(tex, glTextureId);
	}
}

void draw_bistro_exterior(void) {
	glUseProgram(h_ShaderProgram_TXPBR);
	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPBR, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPBR, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPBR, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glUniform4fv(loc_cameraPos, 1, current_camera.pos);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		int diffuseTexId = scene.material_list[materialIdx].diffuseTexId;
		int normalMapTexId = scene.material_list[materialIdx].normalMapTexId;
		int specularTexId = scene.material_list[materialIdx].specularTexId;;
		int emissiveTexId = scene.material_list[materialIdx].emissiveTexId;

		bindTexture(loc_material.diffuseTex, TEXTURE_INDEX_DIFFUSE, diffuseTexId);
		bindTexture(loc_material.normalTex, TEXTURE_INDEX_NORMAL, normalMapTexId);
		bindTexture(loc_material.specularTex, TEXTURE_INDEX_SPECULAR, specularTexId);
		bindTexture(loc_material.emissiveTex, TEXTURE_INDEX_EMISSIVE, emissiveTexId);
		glEnable(GL_TEXTURE_2D);

		glBindVertexArray(bistro_exterior_VAO[materialIdx]);
		glDrawArrays(GL_TRIANGLES, 0, 3 * bistro_exterior_n_triangles[materialIdx]);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}

// skybox
GLuint skybox_VBO, skybox_VAO;
GLuint skybox_texture_name;

GLfloat cube_vertices[72][3] = {
	// vertices enumerated clockwise
	  // 6*2*3 * 2 (POS & NORM)

	// position
	-1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,    1.0f,  1.0f,  1.0f, //right
	 1.0f,  1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f, -1.0f, //left
	 1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f,  1.0f,

	-1.0f, -1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f, //top
	 1.0f,  1.0f,  1.0f,    1.0f, -1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,   -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f, //bottom
	 1.0f, -1.0f, -1.0f,    1.0f,  1.0f, -1.0f,   -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,   -1.0f, -1.0f, -1.0f,   -1.0f,  1.0f, -1.0f, //back
	-1.0f,  1.0f, -1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f,    1.0f,  1.0f,  1.0f, //front
	 1.0f,  1.0f,  1.0f,    1.0f,  1.0f, -1.0f,    1.0f, -1.0f, -1.0f,

	 // normal
	 0.0f, 0.0f, -1.0f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f, -1.0f,
	 0.0f, 0.0f, -1.0f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f, -1.0f,

	-1.0f, 0.0f,  0.0f,     -1.0f, 0.0f,  0.0f,    -1.0f, 0.0f,  0.0f,
	-1.0f, 0.0f,  0.0f,     -1.0f, 0.0f,  0.0f,    -1.0f, 0.0f,  0.0f,

	 1.0f, 0.0f,  0.0f,      1.0f, 0.0f,  0.0f,     1.0f, 0.0f,  0.0f,
	 1.0f, 0.0f,  0.0f,      1.0f, 0.0f,  0.0f,     1.0f, 0.0f,  0.0f,

	 0.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,
	 0.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,

	 0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
	 0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 0.0f,

	 0.0f, -1.0f, 0.0f,      0.0f, -1.0f, 0.0f,     0.0f, -1.0f, 0.0f,
	 0.0f, -1.0f, 0.0f,      0.0f, -1.0f, 0.0f,     0.0f, -1.0f, 0.0f
};

void readTexImage2DForCubeMap(const char* filename, GLenum texture_target) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);

	width = FreeImage_GetWidth(tx_pixmap);
	height = FreeImage_GetHeight(tx_pixmap);
	FreeImage_FlipVertical(tx_pixmap);
	data = FreeImage_GetBits(tx_pixmap);

	glTexImage2D(texture_target, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap);
}

void prepare_skybox(void) { // Draw skybox.
	glGenVertexArrays(1, &skybox_VAO);
	glGenBuffers(1, &skybox_VBO);

	glBindVertexArray(skybox_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_VBO);
	glBufferData(GL_ARRAY_BUFFER, 36 * 3 * sizeof(GLfloat), &cube_vertices[0][0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &skybox_texture_name);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_name);

	readTexImage2DForCubeMap("Scene/Cubemap/px.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	readTexImage2DForCubeMap("Scene/Cubemap/nx.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	readTexImage2DForCubeMap("Scene/Cubemap/py.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	readTexImage2DForCubeMap("Scene/Cubemap/ny.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	readTexImage2DForCubeMap("Scene/Cubemap/pz.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	readTexImage2DForCubeMap("Scene/Cubemap/nz.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	fprintf(stdout, " * Loaded cube map textures into graphics memory.\n\n");

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void draw_skybox(void) {
	glUseProgram(h_ShaderProgram_skybox);

	glUniform1i(loc_cubemap_skybox, TEXTURE_INDEX_SKYMAP);

	ModelViewMatrix = ViewMatrix * glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20000, 20000, 20000));
	//ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(20000.0f, 20000.0f, 20000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_SKY, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(skybox_VAO);
	glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_SKYMAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_name);

	glFrontFace(GL_CW);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 3);
	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
	glUseProgram(0);
}

#define N_TIGER_FRAMES		12
#define N_TIGER_WAYPOINTS	25

float tiger_position[3] = { 1150.0f,  2988.0f, 10.0f };
float tiger_angle = 0.0f;

float tiger_t = 0.0f;
int tiger_segment = 0;
int flag_tiger_animation = 1;

float tiger_waypoints[N_TIGER_WAYPOINTS][3] = {
	{  582.0f,  2137.0f, 10.0f }, 
	{   15.0f,  1287.0f, 10.0f },
	{ -502.0f,   677.0f, 10.0f },
	{ -797.0f,    82.0f, 10.0f },
	{ -509.0f,  -543.0f, 10.0f },
	{   18.0f,  -829.0f, 10.0f },
	{  605.0f, -1030.0f, 10.0f },
	{ 1577.0f, -1405.0f, 10.0f },
	{ 2435.0f, -1755.0f, 10.0f },
	{ 3287.0f, -2101.0f, 10.0f },
	{ 3735.0f, -2860.0f, 10.0f },
	{ 4888.0f, -3245.0f, 10.0f },
	{ 5274.0f, -2477.0f, 10.0f },
	{ 4671.0f, -1690.0f, 10.0f },
	{ 3730.0f, -2037.0f, 10.0f },
	{ 3287.0f, -2101.0f, 10.0f },
	{ 2435.0f, -1755.0f, 10.0f },
	{ 1577.0f, -1405.0f, 10.0f },
	{  605.0f, -1030.0f, 10.0f },
	{   18.0f,  -829.0f, 10.0f },
	{ -509.0f,  -543.0f, 10.0f },
	{ -797.0f,    82.0f, 10.0f },
	{ -502.0f,   677.0f, 10.0f },
	{   15.0f,  1287.0f, 10.0f },
	{  582.0f,  2137.0f, 10.0f },
};

#define N_BEN_FRAMES 30
#define N_BEN_WAYPOINTS 11
#define TIGER_DANGER_RADIUS 800.0f

float ben_position[3] = { 5975.0f, -3109.0f, 0.0f };
float ben_angle = 0.0f;

float ben_t = 0.0f;
int ben_segment = 0;
int ben_direction = 1;
int flag_ben_running = 0;
int flag_ben_waiting = 0;

float ben_waypoints[N_BEN_WAYPOINTS][3] = {
	{ 5975.0f, -3109.0f, 0.0f },
	{ 5313.0f, -2281.0f, 0.0f },
	{ 4787.0f, -1677.0f, 0.0f },
	{ 4026.0f, -1912.0f, 0.0f },
	{ 3326.0f, -2090.0f, 0.0f },
	{ 2767.0f, -1871.0f, 0.0f },
	{ 1696.0f, -1434.0f, 0.0f },
	{  419.0f,  -994.0f, 0.0f },
	{ -338.0f,  -692.0f, 0.0f },
	{ -380.0f,  -186.0f, 0.0f },
	{  101.0f,   144.0f, 0.0f },
};

#define N_SPIDER_FRAMES			16
#define SPIDER_CLIMB_HEIGHT		1450.0f
#define SPIDER_CLIMB_SPEED		5.0f
#define SPIDER_GRAVITY			-40.0f
#define SPIDER_BOUNCE_DECAY		0.4f
#define SPIDER_MAX_BOUNCES		5
#define SPIDER_GROUND			60.0f
#define SPIDER_WAIT				15

typedef enum { SPIDER_CLIMBING, SPIDER_FALLING, SPIDER_BOUNCING, SPIDER_WAITING } SPIDER_STATE;
SPIDER_STATE spider_state = SPIDER_CLIMBING;

float spider_position[3] = { 1730.0f, 3300.0f, 0.0f };
float spider_angle;
float spider_velocity;
float spider_bounce_count;
int spider_wait_count;

void draw_my_objects_20210041(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUseProgram(h_ShaderProgram_simple);

	if (current_mode != MODE_GAME || !item_collected[0]) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-1700.0f, 1000.0f, 25.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -45.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniform3f(loc_primitive_color, 1.0f, 0.5f, 0.0f);
		draw_bus();
	}

	if (current_mode != MODE_GAME || !item_collected[1]) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(3100.0f, -2700.0f, -40.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniform3f(loc_primitive_color, 0.0f, 1.0f, 0.0f);
		draw_tower();
	}

	if (current_mode != MODE_GAME || !item_collected[2]) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(700.0f, 2800.0f, 30.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -45.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(120.0f, 120.0f, 120.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniform3f(loc_primitive_color, 0.0f, 0.5f, 1.0f);
		draw_bike();
	}

	if (current_mode != MODE_GAME || !item_collected[3]) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(1120.0f, -960.0f, 300.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -20.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(30.0f, 30.0f, 30.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniform3f(loc_primitive_color, 1.0f, 0.0f, 0.0f);
		draw_ant();
	}

	if (current_mode != MODE_GAME || !item_collected[4]) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-240.0f, -300.0f, 30.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(70.0f, 70.0f, 70.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniform3f(loc_primitive_color, 1.0f, 0.0f, 0.0f);
		draw_cat();
	}

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(tiger_position[0], tiger_position[1], tiger_position[2]));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, tiger_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniform3f(loc_primitive_color, 1.0f, 0.5f, 0.0f);
	draw_tiger();

	if (current_mode != MODE_GAME) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(ben_position[0], ben_position[1], ben_position[2]));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ben_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(300.0f, 300.0f, 300.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniform3f(loc_primitive_color, 0.0f, 1.0f, 1.0f);
		draw_ben();
	}

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(spider_position[0], spider_position[1], spider_position[2]));
	if (spider_state != SPIDER_CLIMBING)
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -135.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniform3f(loc_primitive_color, 0.8f, 0.2f, 0.8f);
	draw_spider();

	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
/*****************************  END: geometry setup *****************************/

/********************  START: callback function definitions *********************/
int tracking_target[NUM_CAMERAS] = { 0, };
CAMERA_INDEX tracking_cameras[4] = {
	CAMERA_U, CAMERA_I, CAMERA_O, CAMERA_P
};

float distance_2D(float x1, float y1, float x2, float y2) {
	float dx = x1 - x2;
	float dy = y1 - y2;
	return sqrtf(dx * dx + dy * dy);
}

bool is_occluded(CAMERA_INDEX cam_idx, float obj_pos[3],
	int vp_x, int vp_y, int vp_w, int vp_h) {
	
	Camera* cam = &camera_info[cam_idx];

	glm::mat4 ViewMatrix = glm::mat4(
		cam->uaxis[0], cam->vaxis[0], cam->naxis[0], 0.0f,
		cam->uaxis[1], cam->vaxis[1], cam->naxis[1], 0.0f,
		cam->uaxis[2], cam->vaxis[2], cam->naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-cam->pos[0], -cam->pos[1], -cam->pos[2]));

	glm::mat4 ProjectionMatrix = glm::perspective(cam->fovy, (float)vp_w / vp_h, cam->near_c, cam->far_c);

	glm::vec4 clip = ProjectionMatrix * ViewMatrix * glm::vec4(obj_pos[0], obj_pos[1], obj_pos[2], 1.0f);
	if (clip.w <= 0.0f) return true;

	glm::vec3 ndc = glm::vec3(clip) / clip.w;
	if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f)
		return true;

	int sx = vp_x + (int)((ndc.x + 1.0f) * 0.5f * vp_w);
	int sy = vp_y + (int)((ndc.y + 1.0f) * 0.5f * vp_h);

	float depths[25];
	glReadPixels(sx - 2, sy - 2, 5, 5, GL_DEPTH_COMPONENT, GL_FLOAT, depths);
	float min_depth = depths[0];
	for (int i = 1; i < 25; i++) {
		if (depths[i] < min_depth) min_depth = depths[i];
	}

	float obj_depth = (ndc.z + 1.0f) * 0.5f;

	return min_depth < obj_depth - 0.01f;
}

bool is_in_camera(CAMERA_INDEX cam_idx, float obj_pos[3],
	int vp_x, int vp_y, int vp_w, int vp_h) {

	Camera* cam = &camera_info[cam_idx];

	glm::vec3 to_obj = glm::normalize(glm::vec3(
		obj_pos[0] - cam->pos[0],
		obj_pos[1] - cam->pos[1],
		obj_pos[2] - cam->pos[2]));

	glm::vec3 forward = glm::vec3(-cam->naxis[0], -cam->naxis[1], -cam->naxis[2]);

	float dot = glm::dot(to_obj, forward);
	float angle = acosf(glm::clamp(dot, -1.0f, 1.0f));

	if (!(dot > 0.0f && angle < cam->fovy * 0.6f)) return false;

	return !is_occluded(cam_idx, obj_pos, vp_x, vp_y, vp_w, vp_h);
}

int tracking_cooldown[NUM_CAMERAS] = { 0, };
#define TRACKING_COOLDOWN_FRAMES 10

void update_tracking_cameras(CAMERA_INDEX cam_idx,
	int vp_x, int vp_y, int vp_w, int vp_h) {

	if (tracking_target[cam_idx] != 0) {
		float* tracked_pos =
			(tracking_target[cam_idx] == 1) ? tiger_position :
			(tracking_target[cam_idx] == 2) ? ben_position : spider_position;

		if (is_occluded(cam_idx, tracked_pos, vp_x, vp_y, vp_w, vp_h)) {
			memcpy(&camera_info[cam_idx], &camera_info_default[cam_idx], sizeof(Camera));
			tracking_target[cam_idx] = 0;
			if (current_camera_index == cam_idx)
				set_current_camera(cam_idx);
		}
		else {
			Camera* cam = &camera_info[cam_idx];
			glm::vec3 forward = glm::normalize(glm::vec3(
				tracked_pos[0] - cam->pos[0],
				tracked_pos[1] - cam->pos[1],
				tracked_pos[2] - cam->pos[2]));
			glm::vec3 n = -forward;
			glm::vec3 u = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 0.0f, 1.0f)));
			glm::vec3 v = glm::cross(n, u);
			cam->naxis[0] = n.x; cam->naxis[1] = n.y; cam->naxis[2] = n.z;
			cam->uaxis[0] = u.x; cam->uaxis[1] = u.y; cam->uaxis[2] = u.z;
			cam->vaxis[0] = v.x; cam->vaxis[1] = v.y; cam->vaxis[2] = v.z;
			if (current_camera_index == cam_idx)
				set_current_camera(cam_idx);
		}
		return;
	}

	if (tracking_cooldown[cam_idx] > 0) {
		tracking_cooldown[cam_idx]--;
		return;
	}

	bool tiger_visible = is_in_camera(cam_idx, tiger_position, vp_x, vp_y, vp_w, vp_h);
	bool ben_visible = is_in_camera(cam_idx, ben_position, vp_x, vp_y, vp_w, vp_h);
	bool spider_visible = is_in_camera(cam_idx, spider_position, vp_x, vp_y, vp_w, vp_h);

	if (!tiger_visible && !ben_visible && !spider_visible) return;

	float d_tiger = tiger_visible ?
		distance_2D(camera_info_default[cam_idx].pos[0], camera_info_default[cam_idx].pos[1],
			tiger_position[0], tiger_position[1]) : FLT_MAX;
	float d_ben = ben_visible ?
		distance_2D(camera_info_default[cam_idx].pos[0], camera_info_default[cam_idx].pos[1],
			ben_position[0], ben_position[1]) : FLT_MAX;
	float d_spider = spider_visible ?
		distance_2D(camera_info_default[cam_idx].pos[0], camera_info_default[cam_idx].pos[1],
			spider_position[0], spider_position[1]) : FLT_MAX;

	float* target_pos = nullptr;
	if (d_tiger <= d_ben && d_tiger <= d_spider) {
		target_pos = tiger_position;
		tracking_target[cam_idx] = 1;
	}
	else if (d_ben <= d_tiger && d_ben <= d_spider) {
		target_pos = ben_position;
		tracking_target[cam_idx] = 2;
	}
	else {
		target_pos = spider_position;
		tracking_target[cam_idx] = 3;
	}

	Camera* cam = &camera_info[cam_idx];
	glm::vec3 forward = glm::normalize(glm::vec3(
		target_pos[0] - cam->pos[0],
		target_pos[1] - cam->pos[1],
		target_pos[2] - cam->pos[2]));
	glm::vec3 n = -forward;
	glm::vec3 u = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 0.0f, 1.0f)));
	glm::vec3 v = glm::cross(n, u);
	cam->naxis[0] = n.x; cam->naxis[1] = n.y; cam->naxis[2] = n.z;
	cam->uaxis[0] = u.x; cam->uaxis[1] = u.y; cam->uaxis[2] = u.z;
	cam->vaxis[0] = v.x; cam->vaxis[1] = v.y; cam->vaxis[2] = v.z;
	if (current_camera_index == cam_idx)
		set_current_camera(cam_idx);
}

#define N_CCTV_CAMERAS 7

void draw_cctv(void) {
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);

	int top_h = (h / 2);
	int bot_h = h - top_h;

	int top_w = (w / 4);
	int bot_w = w / 3;

	CAMERA_INDEX cctv_cameras[N_CCTV_CAMERAS] = {
		CAMERA_U, CAMERA_I, CAMERA_O, CAMERA_P,
		CAMERA_A, CAMERA_T, CAMERA_G
	};

	int vp[N_CCTV_CAMERAS][4] = {
		{ (w / 4) * 0, (h / 2), (w / 4), (h / 2) },
		{ (w / 4) * 1, (h / 2), (w / 4), (h / 2) },
		{ (w / 4) * 2, (h / 2), (w / 4), (h / 2) },
		{ (w / 4) * 3, (h / 2), (w / 4), (h / 2) },
		{ (w / 4) * 0,		 0, (w / 4), (h / 2) },
		{ (w / 4) * 1,		 0, (w / 4), (h / 2) },
		{ (w / 4) * 2,		 0, (w / 4), (h / 2) },
	};

	Camera saved_camera = current_camera;
	int saved_idx = current_camera_index;

	for (int i = 0; i < N_CCTV_CAMERAS; i++) {
		glViewport(vp[i][0], vp[i][1], vp[i][2], vp[i][3]);
		glScissor(vp[i][0], vp[i][1], vp[i][2], vp[i][3]);
		glEnable(GL_SCISSOR_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);

		ProjectionMatrix = glm::perspective(
			camera_info[cctv_cameras[i]].fovy,
			(float)vp[i][2] / vp[i][3],
			camera_info[cctv_cameras[i]].near_c,
			camera_info[cctv_cameras[i]].far_c);

		memcpy(&current_camera, &camera_info[cctv_cameras[i]], sizeof(Camera));
		set_ViewMatrix_from_camera_frame();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		draw_bistro_exterior();
		draw_skybox();
		draw_my_objects_20210041();
		if (i < 4)
			update_tracking_cameras(cctv_cameras[i], vp[i][0], vp[i][1], vp[i][2], vp[i][3]);
	}
	glDisable(GL_SCISSOR_TEST);

	current_camera_index = saved_idx;
	memcpy(&current_camera, &saved_camera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();
	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glViewport(0, 0, w, h);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	switch (current_mode) {
	case MODE_NORMAL:
		draw_grid();
		draw_axes();
		draw_bistro_exterior();
		draw_skybox();
		draw_my_objects_20210041();
		break;
	case MODE_CCTV:
		draw_cctv();
		break;
	case MODE_GAME:
		draw_grid();
		draw_axes();
		draw_bistro_exterior();
		draw_skybox();
		draw_game_objects();
		draw_my_objects_20210041();
		break;
	}
	glutSwapBuffers();
}

#define CAM_TSPEED 50.0f
#define CAM_RSPEED 0.3f

int ctrl_pressed = 0;
int mouse_prev_x = 0;
int mouse_prev_y = 0;
int saved_camera_before_cctv = CAMERA_U;

void keyboard(unsigned char key, int x, int y) {
	if (key == 'm') {
		if (current_mode != MODE_GAME) {
			current_mode = MODE_GAME;
			init_game_mode();
			set_current_camera(CAMERA_B);
		}
		else {
			current_mode = MODE_NORMAL;
			exit_game_mode();
			set_current_camera(CAMERA_U);
		}
		glutPostRedisplay();
		return;
	}

	if (current_mode == MODE_GAME) {
		keyboard_game(key, x, y);
		return;
	}

	switch (key) {
	case 'f':
		b_draw_grid = b_draw_grid ? false : true;
		glutPostRedisplay();
		break;
	case 'u':
		set_current_camera(CAMERA_U);
		glutPostRedisplay();
		break;
	case 'i':
		set_current_camera(CAMERA_I);
		glutPostRedisplay();
		break;
	case 'o':
		set_current_camera(CAMERA_O);
		glutPostRedisplay();
		break;
	case 'p':
		set_current_camera(CAMERA_P);
		glutPostRedisplay();
		break;
	case 'a':
		set_current_camera(CAMERA_A);
		glutPostRedisplay();
		break;
	case 'q':
		if (current_camera_index == CAMERA_A && current_mode != MODE_CCTV) {
			current_camera.pos[0] -= CAM_TSPEED * current_camera.vaxis[0];
			current_camera.pos[1] -= CAM_TSPEED * current_camera.vaxis[1];
			current_camera.pos[2] -= CAM_TSPEED * current_camera.vaxis[2];

			update_current_camera();
			glutPostRedisplay();
		}
		break;
	case 'e':
		if (current_camera_index == CAMERA_A && current_mode != MODE_CCTV) {
			current_camera.pos[0] += CAM_TSPEED * current_camera.vaxis[0];
			current_camera.pos[1] += CAM_TSPEED * current_camera.vaxis[1];
			current_camera.pos[2] += CAM_TSPEED * current_camera.vaxis[2];

			update_current_camera();
			glutPostRedisplay();
		}
		break;
	case 'z':
		if (current_camera_index == CAMERA_A && current_mode != MODE_CCTV) {
			glm::mat3 RotationMatrix;
			glm::vec3 direction;
			RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), -CAM_RSPEED * TO_RADIAN,
				glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2])));

			direction = RotationMatrix * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
			current_camera.uaxis[0] = direction[0];
			current_camera.uaxis[1] = direction[1];
			current_camera.uaxis[2] = direction[2];

			direction = RotationMatrix * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
			current_camera.vaxis[0] = direction[0];
			current_camera.vaxis[1] = direction[1];
			current_camera.vaxis[2] = direction[2];

			update_current_camera();
			glutPostRedisplay();
		}
		break;
	case 'x':
		if (current_camera_index == CAMERA_A && current_mode != MODE_CCTV) {
			glm::mat3 RotationMatrix;
			glm::vec3 direction;
			RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), CAM_RSPEED * TO_RADIAN,
				glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2])));

			direction = RotationMatrix * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
			current_camera.uaxis[0] = direction[0];
			current_camera.uaxis[1] = direction[1];
			current_camera.uaxis[2] = direction[2];

			direction = RotationMatrix * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
			current_camera.vaxis[0] = direction[0];
			current_camera.vaxis[1] = direction[1];
			current_camera.vaxis[2] = direction[2];

			update_current_camera();
			glutPostRedisplay();
		}
		break;
	case 'r':
		flag_tiger_animation = !flag_tiger_animation;
		break;
	case 't':
		set_current_camera(CAMERA_T);
		glutPostRedisplay();
		break;
	case 'g':
		set_current_camera(CAMERA_G);
		glutPostRedisplay();
		break;
	case 'c':
		if (current_mode != MODE_CCTV) {
			saved_camera_before_cctv = current_camera_index;
			current_mode = MODE_CCTV;
		}
		else {
			current_mode = MODE_NORMAL;
			for (int i = 0; i < 4; i++) {
				memcpy(&camera_info[tracking_cameras[i]], &camera_info_default[tracking_cameras[i]], sizeof(Camera));
				tracking_target[i] = 0;
				tracking_cooldown[i] = 0;
			}
			set_current_camera(saved_camera_before_cctv);
		}
		glutPostRedisplay();
		break;
	case 'k':
		print_camera_info();
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void keyboard_up(unsigned char key, int x, int y) {
	if (current_mode == MODE_GAME) {
		keyboard_game_up(key, x, y);
	}
}

void special(int key, int x, int y) {
	if (key == GLUT_KEY_CTRL_L || key == GLUT_KEY_CTRL_R) {
		ctrl_pressed = 1;
		mouse_prev_x = x;
		mouse_prev_y = y;
	}

	if (current_camera_index == CAMERA_A && current_mode != MODE_CCTV) {
		switch (key) {
		case GLUT_KEY_UP:
			current_camera.pos[0] -= CAM_TSPEED * current_camera.naxis[0];
			current_camera.pos[1] -= CAM_TSPEED * current_camera.naxis[1];
			current_camera.pos[2] -= CAM_TSPEED * current_camera.naxis[2];
			
			update_current_camera();
			glutPostRedisplay();
			break;
		case GLUT_KEY_DOWN:
			current_camera.pos[0] += CAM_TSPEED * current_camera.naxis[0];
			current_camera.pos[1] += CAM_TSPEED * current_camera.naxis[1];
			current_camera.pos[2] += CAM_TSPEED * current_camera.naxis[2];
			
			update_current_camera();
			glutPostRedisplay();
			break;
		case GLUT_KEY_LEFT:
			current_camera.pos[0] -= CAM_TSPEED * current_camera.uaxis[0];
			current_camera.pos[1] -= CAM_TSPEED * current_camera.uaxis[1];
			current_camera.pos[2] -= CAM_TSPEED * current_camera.uaxis[2];
			
			update_current_camera();
			glutPostRedisplay();
			break;
		case GLUT_KEY_RIGHT:
			current_camera.pos[0] += CAM_TSPEED * current_camera.uaxis[0];
			current_camera.pos[1] += CAM_TSPEED * current_camera.uaxis[1];
			current_camera.pos[2] += CAM_TSPEED * current_camera.uaxis[2];
			
			update_current_camera();
			glutPostRedisplay();
			break;
		}
	}
}

void specialup(int key, int x, int y) {
	if (key == GLUT_KEY_CTRL_L || key == GLUT_KEY_CTRL_R)
		ctrl_pressed = 0;
}

#define ZOOM_SENSITIVITY	0.7f
#define ZOOM_MIN			5.0f
#define ZOOM_MAX			120.0f

void wheel(int wheel, int direction, int x, int y) {
	if (ctrl_pressed && (is_world_observation_camera() || current_camera_index == CAMERA_A) && current_mode != MODE_CCTV) {

		current_camera.fovy -= direction * ZOOM_SENSITIVITY * TO_RADIAN;
		if (current_camera.fovy < ZOOM_MIN * TO_RADIAN)
			current_camera.fovy = ZOOM_MIN * TO_RADIAN;
		if (current_camera.fovy > ZOOM_MAX * TO_RADIAN)
			current_camera.fovy = ZOOM_MAX * TO_RADIAN;

		camera_info[current_camera_index].fovy = current_camera.fovy;

		ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
	}
}

void mousemove(int x, int y) {
	if (current_mode == MODE_GAME) {
		mousemove_game(x, y);
		return;
	}

	if (ctrl_pressed && (is_world_observation_camera() || current_camera_index == CAMERA_A) && current_mode != MODE_CCTV) {
		int dx = x - mouse_prev_x;
		int dy = y - mouse_prev_y;
		mouse_prev_x = x;
		mouse_prev_y = y;

		glm::vec3 u = glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
		glm::vec3 v = glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
		glm::vec3 n = glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);

		if (dx != 0) {
			glm::mat3 rotate_yaw = glm::mat3(glm::rotate(glm::mat4(1.0f), -dx * CAM_RSPEED * TO_RADIAN, v));
			u = rotate_yaw * u;
			n = rotate_yaw * n;
		}
		 
		if (dy != 0) {
			glm::mat3 rotate_pitch = glm::mat3(glm::rotate(glm::mat4(1.0f), -dy * CAM_RSPEED * TO_RADIAN, u));
			v = rotate_pitch * v;
			n = rotate_pitch * n;
		}

		n = glm::normalize(n);
		u = glm::normalize(glm::cross(v, n));
		v = glm::normalize(glm::cross(n, u));

		current_camera.uaxis[0] = u[0]; current_camera.uaxis[1] = u[1]; current_camera.uaxis[2] = u[2];
		current_camera.vaxis[0] = v[0]; current_camera.vaxis[1] = v[1]; current_camera.vaxis[2] = v[2];
		current_camera.naxis[0] = n[0]; current_camera.naxis[1] = n[1]; current_camera.naxis[2] = n[2];
		
		update_current_camera();
		glutPostRedisplay();
	}
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);

	float aspect_ratio = (float)width / height;
	for (int i = 0; i < NUM_CAMERAS; i++)
		camera_info[i].aspect_ratio = aspect_ratio;
	current_camera.aspect_ratio = aspect_ratio;

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &grid_VAO);
	glDeleteBuffers(1, &grid_VBO);

	glDeleteVertexArrays(scene.n_materials, bistro_exterior_VAO);
	glDeleteBuffers(scene.n_materials, bistro_exterior_VBO);
	glDeleteTextures(scene.n_textures, bistro_exterior_texture_names);

	glDeleteVertexArrays(1, &skybox_VAO);
	glDeleteBuffers(1, &skybox_VBO);

	free(bistro_exterior_n_triangles);
	free(bistro_exterior_vertex_offset);

	free(bistro_exterior_VAO);
	free(bistro_exterior_VBO);

	free(bistro_exterior_texture_names);
	free(flag_texture_mapping);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);
	glDeleteVertexArrays(1, &ben_VAO);
	glDeleteBuffers(1, &ben_VBO);
	glDeleteVertexArrays(1, &spider_VAO);
	glDeleteBuffers(1, &spider_VBO);

	glDeleteVertexArrays(1, &bus_VAO);
	glDeleteBuffers(1, &bus_VBO);
	glDeleteVertexArrays(1, &tower_VAO);
	glDeleteBuffers(1, &tower_VBO);
	glDeleteVertexArrays(1, &bike_VAO);
	glDeleteBuffers(1, &bike_VBO);
	glDeleteVertexArrays(1, &ant_VAO);
	glDeleteBuffers(1, &ant_VBO);
	glDeleteVertexArrays(1, &cat_VAO);
	glDeleteBuffers(1, &cat_VBO);
}
/*********************  END: callback function definitions **********************/
glm::vec3 catmull_rom_spline(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
	return 0.5f * (
		2.0f * p1 +
		(-p0 + p2) * t +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
}

void update_ben(void) {
	float dist = distance_2D(ben_position[0], ben_position[1], tiger_position[0], tiger_position[1]);

	if (flag_ben_waiting) {
		if (dist > TIGER_DANGER_RADIUS * 2.0f)
			flag_ben_waiting = 0;
		else
			return;
	}

	if (dist < TIGER_DANGER_RADIUS && !flag_ben_running) {
		flag_ben_running = 1;
		ben_direction = -ben_direction;
		ben_t = 1.0f - ben_t;
	}
	else if (flag_ben_running && dist > TIGER_DANGER_RADIUS * 2.0f) {
		flag_ben_running = 0;
	}

	float speed = flag_ben_running ? 0.05f : 0.03f;
	ben_t += speed;

	if (ben_t >= 1.0f) {
		ben_t = 0.0f;
		ben_segment += ben_direction;

		if (ben_segment >= N_BEN_WAYPOINTS - 1) {
			ben_segment = N_BEN_WAYPOINTS - 1;
			ben_t = 1.0f;
			flag_ben_waiting = 1;
			flag_ben_running = 0;
			ben_direction = -1;
		}
		else if (ben_segment < 0) {
			ben_segment = 0;
			ben_t = 0.0f;
			flag_ben_waiting = 1;
			flag_ben_running = 0;
			ben_direction = 1;
		}
	}

	int p0 = (ben_segment - 1 < 0) ? 0 : ben_segment - 1;
	int p1 = ben_segment;
	int p2 = (ben_segment + 1 >= N_BEN_WAYPOINTS) ? N_BEN_WAYPOINTS - 1 : ben_segment + 1;
	int p3 = (ben_segment + 2 >= N_BEN_WAYPOINTS) ? N_BEN_WAYPOINTS - 1 : ben_segment + 2;

	float t = (ben_direction == 1) ? ben_t : 1.0f - ben_t;

	glm::vec3 pos = catmull_rom_spline(
		glm::vec3(ben_waypoints[p0][0], ben_waypoints[p0][1], ben_waypoints[p0][2]),
		glm::vec3(ben_waypoints[p1][0], ben_waypoints[p1][1], ben_waypoints[p1][2]),
		glm::vec3(ben_waypoints[p2][0], ben_waypoints[p2][1], ben_waypoints[p2][2]),
		glm::vec3(ben_waypoints[p3][0], ben_waypoints[p3][1], ben_waypoints[p3][2]),
		t
	);

	float dx = pos.x - ben_position[0];
	float dy = pos.y - ben_position[1];
	if (dx != 0.0f || dy != 0.0f)
		ben_angle = atan2f(dy, dx) + 180.0f * TO_RADIAN;

	ben_position[0] = pos.x;
	ben_position[1] = pos.y;
	ben_position[2] = pos.z;

	cur_frame_ben = (cur_frame_ben + (flag_ben_running ? 5 : 3)) % N_BEN_FRAMES;
}

#define TIGER_EYE_X     0.0f
#define TIGER_EYE_Y		-88.0f
#define TIGER_EYE_Z		62.0f
#define TIGER_SCALE		2.0f
#define HEAD_AMPLITUDE	3.0f
#define ROLL_AMPLITUDE  1.5f

void update_camera_t(void) {
	float eye_y_scaled = TIGER_EYE_Y * TIGER_SCALE;
	float eye_z_scaled = TIGER_EYE_Z * TIGER_SCALE;

	camera_info[CAMERA_T].pos[0] = tiger_position[0] - eye_y_scaled * sinf(tiger_angle);
	camera_info[CAMERA_T].pos[1] = tiger_position[1] + eye_y_scaled * cosf(tiger_angle);
	camera_info[CAMERA_T].pos[2] = tiger_position[2] + eye_z_scaled;

	glm::vec3 n = glm::vec3(-sinf(tiger_angle), cosf(tiger_angle), 0.0f);
	glm::vec3 u = glm::vec3(-cosf(tiger_angle), -sinf(tiger_angle), 0.0f);
	glm::vec3 v = glm::vec3(0.0f, 0.0f, 1.0f);

	float frame_angle = cur_frame_tiger * 2.0f * 3.141592f / N_TIGER_FRAMES;
	float head_angle = HEAD_AMPLITUDE * TO_RADIAN * sinf(frame_angle);
	float roll_angle = ROLL_AMPLITUDE * TO_RADIAN * sinf(frame_angle + 3.141592f * 0.5f);

	glm::mat3 head_rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), head_angle, u));
	n = head_rotation * n;
	v = head_rotation * v;

	glm::mat3 roll_rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), roll_angle, n));
	u = roll_rotation * u;
	v = roll_rotation * v;

	camera_info[CAMERA_T].naxis[0] = n.x; camera_info[CAMERA_T].naxis[1] = n.y; camera_info[CAMERA_T].naxis[2] = n.z;
	camera_info[CAMERA_T].uaxis[0] = u.x; camera_info[CAMERA_T].uaxis[1] = u.y; camera_info[CAMERA_T].uaxis[2] = u.z;
	camera_info[CAMERA_T].vaxis[0] = v.x; camera_info[CAMERA_T].vaxis[1] = v.y; camera_info[CAMERA_T].vaxis[2] = v.z;

	if (current_camera_index == CAMERA_T)
		set_current_camera(CAMERA_T);
}

#define CAMERA_G_DIST	800.0f
#define CAMERA_G_HEIGHT 400.0f

void update_camera_g(void) {
	camera_info[CAMERA_G].pos[0] = tiger_position[0] + (-sinf(tiger_angle)) * CAMERA_G_DIST;
	camera_info[CAMERA_G].pos[1] = tiger_position[1] + cosf(tiger_angle) * CAMERA_G_DIST;
	camera_info[CAMERA_G].pos[2] = tiger_position[2] + CAMERA_G_HEIGHT;

	glm::vec3 forward = glm::normalize(glm::vec3(
						tiger_position[0] - camera_info[CAMERA_G].pos[0],
						tiger_position[1] - camera_info[CAMERA_G].pos[1],
						tiger_position[2] + 124.0f - camera_info[CAMERA_G].pos[2]));
	glm::vec3 n = -forward;
	glm::vec3 world_up = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 u = glm::normalize(glm::cross(forward, world_up));
	glm::vec3 v = glm::cross(n, u);

	camera_info[CAMERA_G].naxis[0] = n.x; camera_info[CAMERA_G].naxis[1] = n.y; camera_info[CAMERA_G].naxis[2] = n.z;
	camera_info[CAMERA_G].uaxis[0] = u.x; camera_info[CAMERA_G].uaxis[1] = u.y; camera_info[CAMERA_G].uaxis[2] = u.z;
	camera_info[CAMERA_G].vaxis[0] = v.x; camera_info[CAMERA_G].vaxis[1] = v.y; camera_info[CAMERA_G].vaxis[2] = v.z;

	if (current_camera_index == CAMERA_G)
		set_current_camera(CAMERA_G);
}

void update_tiger() {
	if (flag_tiger_animation) {
		cur_frame_tiger = (cur_frame_tiger + 1) % N_TIGER_FRAMES;
		float prev_t = tiger_t;
		tiger_t += 0.015f;

		if (tiger_t >= 1.0f) {
			tiger_t = 0.0f;
			tiger_segment = (tiger_segment + 1) % (N_TIGER_WAYPOINTS - 1);
		}

		int p0 = (tiger_segment - 1 + N_TIGER_WAYPOINTS) % N_TIGER_WAYPOINTS;
		int p1 = tiger_segment % N_TIGER_WAYPOINTS;
		int p2 = (tiger_segment + 1) % N_TIGER_WAYPOINTS;
		int p3 = (tiger_segment + 2) % N_TIGER_WAYPOINTS;

		glm::vec3 pos = catmull_rom_spline(
			glm::vec3(tiger_waypoints[p0][0], tiger_waypoints[p0][1], tiger_waypoints[p0][2]),
			glm::vec3(tiger_waypoints[p1][0], tiger_waypoints[p1][1], tiger_waypoints[p1][2]),
			glm::vec3(tiger_waypoints[p2][0], tiger_waypoints[p2][1], tiger_waypoints[p2][2]),
			glm::vec3(tiger_waypoints[p3][0], tiger_waypoints[p3][1], tiger_waypoints[p3][2]),
			tiger_t
		);

		float dx = pos.x - tiger_position[0];
		float dy = pos.y - tiger_position[1];
		if (dx != 0.0f || dy != 0.0f)
			tiger_angle = atan2f(dy, dx) + 90.0f * TO_RADIAN;

		tiger_position[0] = pos.x;
		tiger_position[1] = pos.y;
		tiger_position[2] = pos.z;
	}

	update_camera_t();
	update_camera_g();
}

void update_spider(void) {
	switch (spider_state) {
	case SPIDER_CLIMBING:
		spider_position[2] += SPIDER_CLIMB_SPEED;

		if (spider_position[2] >= SPIDER_CLIMB_HEIGHT) {
			spider_position[2] = SPIDER_CLIMB_HEIGHT;
			spider_velocity = 0.0f;
			spider_bounce_count = 0;
			spider_state = SPIDER_FALLING;
		}
		break;

	case SPIDER_FALLING:
		spider_velocity += SPIDER_GRAVITY * 0.033f;
		spider_position[2] += spider_velocity;

		if (spider_position[2] <= SPIDER_GROUND) {
			spider_position[2] = SPIDER_GROUND;
			spider_state = SPIDER_BOUNCING;
		}
		break;

	case SPIDER_BOUNCING:
		spider_velocity += SPIDER_GRAVITY * 0.033f;
		spider_position[2] += spider_velocity;

		if (spider_position[2] <= SPIDER_GROUND) {
			spider_position[2] = SPIDER_GROUND;

			if (spider_bounce_count < SPIDER_MAX_BOUNCES) {
				spider_velocity = -spider_velocity * SPIDER_BOUNCE_DECAY;
				spider_bounce_count++;
			}
			else {
				spider_velocity = 0.0f;
				spider_wait_count = 0;
				spider_state = SPIDER_WAITING;
			}
		}
		break;

	case SPIDER_WAITING:
		spider_wait_count++;
		if (spider_wait_count >= SPIDER_WAIT) {
			spider_wait_count = 0;
			spider_state = SPIDER_CLIMBING;
		}
		break;
	}

	cur_frame_spider = (cur_frame_spider + 1) % N_SPIDER_FRAMES;
}

void timer_scene(int value) {
	if (current_mode == MODE_GAME) {
		update_game_mode();
		if (flag_jumping == 0 && (key_w || key_s || key_a || key_d)) {
			cur_frame_ben = (cur_frame_ben + 3) % N_BEN_FRAMES;
		}
	}
	
	else {
		update_ben();
	}
	update_tiger();
	update_spider();

	glutPostRedisplay();
	glutTimerFunc(50, timer_scene, 0);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboard_up);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialup);
	glutMouseWheelFunc(wheel);
	glutPassiveMotionFunc(mousemove);
	glutMotionFunc(mousemove);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutTimerFunc(50, timer_scene, 0);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	initialize_lights();
}

void prepare_scene(void) {
	prepare_axes();
	prepare_grid();
	prepare_bistro_exterior();
	prepare_skybox();
	prepare_bus();
	prepare_tower();
	prepare_bike();
	prepare_ant();
	prepare_cat();
	prepare_tiger();
	prepare_ben();
	prepare_spider();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
	initialize_camera();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "********************************************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "********************************************************************************\n\n");
}

void print_message(const char* m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "********************************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170/AIE4012 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n********************************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 10
void drawScene(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170/AIE4012 Bistro Exterior Scene (Open Version)";
	char messages[N_MESSAGE_LINES][256] = {};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(900, 600);
	glutInitWindowPosition(20, 20);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
