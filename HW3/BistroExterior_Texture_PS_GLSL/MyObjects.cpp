#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <FreeImage/FreeImage.h>

#include "Shaders/LoadShaders.h"
#include "My_Shading.h"

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

/******************************** START: objects ********************************/
// texture stuffs
#define TEXTURE_FLOOR				(0)
#define TEXTURE_TIGER				(1)
#define TEXTURE_WOLF				(2)
#define TEXTURE_SPIDER				(3)
#define TEXTURE_DRAGON				(4)
#define TEXTURE_OPTIMUS				(5)
#define TEXTURE_COW					(6)
#define TEXTURE_BUS					(7)
#define TEXTURE_BIKE				(8)
#define TEXTURE_GODZILLA			(9)
#define TEXTURE_IRONMAN				(10)
#define TEXTURE_TANK				(11)
#define TEXTURE_NATHAN				(12)
#define TEXTURE_OGRE				(13)
#define TEXTURE_CAT					(14)
#define TEXTURE_ANT					(15)
#define TEXTURE_TOWER				(16)
#define N_TEXTURES_USED				(17)

GLuint texture_names[N_TEXTURES_USED];
bool flag_texture_mapping = true;

// texture id
#define TEXTURE_ID_DIFFUSE	(0)
#define TEXTURE_ID_NORMAL	(1)

int read_geometry_vnt(GLfloat** object, int bytes_per_primitive, char* filename) {
	int n_triangles;
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);

	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

// for multiple materials
int read_geometry_vntm(GLfloat** object, int bytes_per_primitive,
	int* n_matrial_indicies, int** material_indicies,
	int* n_materials, char*** diffuse_texture_names,
	Material_Parameters** material_parameters,
	bool* bOnce,
	char* filename) {
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}

	int n_faces;
	fread(&n_faces, sizeof(int), 1, fp);

	*object = (float*)malloc(n_faces * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...\n", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_faces, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);

	fread(n_matrial_indicies, sizeof(int), 1, fp);

	int bytes_per_indices = sizeof(int) * 2;
	*material_indicies = (int*)malloc(bytes_per_indices * (*n_matrial_indicies)); // material id, offset
	if (*material_indicies == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...\n", filename);
		return -1;
	}

	fread(*material_indicies, bytes_per_indices, (*n_matrial_indicies), fp);

	if (*bOnce == false) {
		fread(n_materials, sizeof(int), 1, fp);

		*material_parameters = (Material_Parameters*)malloc(sizeof(Material_Parameters) * (*n_materials));
		*diffuse_texture_names = (char**)malloc(sizeof(char*) * (*n_materials));
		for (int i = 0; i < (*n_materials); i++) {
			fread((*material_parameters)[i].ambient_color, sizeof(float), 3, fp); //Ka
			fread((*material_parameters)[i].diffuse_color, sizeof(float), 3, fp); //Kd
			fread((*material_parameters)[i].specular_color, sizeof(float), 3, fp); //Ks
			fread(&(*material_parameters)[i].specular_exponent, sizeof(float), 1, fp); //Ns
			fread((*material_parameters)[i].emissive_color, sizeof(float), 3, fp); //Ke

			(*material_parameters)[i].ambient_color[3] = 1.0f;
			(*material_parameters)[i].diffuse_color[3] = 1.0f;
			(*material_parameters)[i].specular_color[3] = 1.0f;
			(*material_parameters)[i].emissive_color[3] = 1.0f;

			(*diffuse_texture_names)[i] = (char*)malloc(sizeof(char) * 256);
			fread((*diffuse_texture_names)[i], sizeof(char), 256, fp);
		}
		*bOnce = true;
	}

	fclose(fp);

	return n_faces;
}

int cur_frame_tiger = 0, cur_frame_ben = 0, cur_frame_wolf, cur_frame_spider = 0, cur_frame_nathan = 0;

// floor object
#define TEX_COORD_EXTENT 1.0f
GLuint rectangle_VBO, rectangle_VAO;
GLfloat rectangle_vertices[6][8] = {  // vertices enumerated counterclockwise
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, TEX_COORD_EXTENT, 0.0f },
	{ 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, TEX_COORD_EXTENT, TEX_COORD_EXTENT },
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, TEX_COORD_EXTENT, TEX_COORD_EXTENT },
	{ 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, TEX_COORD_EXTENT }
};

Material_Parameters material_floor;

void prepare_floor(void) { // Draw coordinate axes.
	// Initialize vertex buffer object.
	glGenBuffers(1, &rectangle_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), &rectangle_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &rectangle_VAO);
	glBindVertexArray(rectangle_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_floor.ambient_color[0] = 0.0f;
	material_floor.ambient_color[1] = 0.05f;
	material_floor.ambient_color[2] = 0.0f;
	material_floor.ambient_color[3] = 1.0f;

	material_floor.diffuse_color[0] = 0.2f;
	material_floor.diffuse_color[1] = 0.5f;
	material_floor.diffuse_color[2] = 0.2f;
	material_floor.diffuse_color[3] = 1.0f;

	material_floor.specular_color[0] = 0.24f;
	material_floor.specular_color[1] = 0.5f;
	material_floor.specular_color[2] = 0.24f;
	material_floor.specular_color[3] = 1.0f;

	material_floor.specular_exponent = 2.5f;

	material_floor.emissive_color[0] = 0.0f;
	material_floor.emissive_color[1] = 0.0f;
	material_floor.emissive_color[2] = 0.0f;
	material_floor.emissive_color[3] = 1.0f;


	//	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//   float border_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//	 glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

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
GLfloat* tiger_vertices[N_TIGER_FRAMES];

Material_Parameters material_tiger;

void prepare_tiger(void) { // vertices enumerated clockwise
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/tiger/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry_vnt(&tiger_vertices[i], n_bytes_per_triangle, filename);
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
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

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
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_tiger.ambient_color[0] = 0.24725f;
	material_tiger.ambient_color[1] = 0.1995f;
	material_tiger.ambient_color[2] = 0.0745f;
	material_tiger.ambient_color[3] = 1.0f;

	material_tiger.diffuse_color[0] = 0.75164f;
	material_tiger.diffuse_color[1] = 0.60648f;
	material_tiger.diffuse_color[2] = 0.22648f;
	material_tiger.diffuse_color[3] = 1.0f;

	material_tiger.specular_color[0] = 0.728281f;
	material_tiger.specular_color[1] = 0.655802f;
	material_tiger.specular_color[2] = 0.466065f;
	material_tiger.specular_color[3] = 1.0f;

	material_tiger.specular_exponent = 51.2f;

	material_tiger.emissive_color[0] = 0.1f;
	material_tiger.emissive_color[1] = 0.1f;
	material_tiger.emissive_color[2] = 0.0f;
	material_tiger.emissive_color[3] = 1.0f;
}

void draw_tiger(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
}

// ben object
#define N_BEN_FRAMES 30
GLuint ben_VBO, ben_VAO;
int ben_n_triangles[N_BEN_FRAMES];
int ben_vertex_offset[N_BEN_FRAMES];
GLfloat* ben_vertices[N_BEN_FRAMES];

int ben_n_material_indices;
int* ben_material_indices[N_BEN_FRAMES];

int ben_n_materials;
char** ben_diffuse_texture;
GLuint* ben_texture_names;

Material_Parameters* material_ben;

void prepare_ben(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ben_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	bool bOnce = false;
	for (i = 0; i < N_BEN_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/ben/ben_vntm_%d%d.geom", i / 10, i % 10);
		ben_n_triangles[i] = read_geometry_vntm(&ben_vertices[i], n_bytes_per_triangle,
			&ben_n_material_indices,
			&ben_material_indices[i],
			&ben_n_materials,
			&ben_diffuse_texture,
			&material_ben,
			&bOnce,
			filename);

		// assume all geometry files are effective
		ben_n_total_triangles += ben_n_triangles[i];

		if (i == 0)
			ben_vertex_offset[i] = 0;
		else
			ben_vertex_offset[i] = ben_vertex_offset[i - 1] + 3 * ben_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &ben_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glBufferData(GL_ARRAY_BUFFER, ben_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_BEN_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, ben_vertex_offset[i] * n_bytes_per_vertex,
			ben_n_triangles[i] * n_bytes_per_triangle, ben_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_BEN_FRAMES; i++)
		free(ben_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &ben_VAO);
	glBindVertexArray(ben_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_ben(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(ben_VAO);

	for (int i = 0; i < ben_n_material_indices - 1; i++) {
		int cur_vertex_offset = ben_vertex_offset[cur_frame_ben] + ben_material_indices[cur_frame_ben][2 * i + 1];
		int n_vertices = ben_material_indices[cur_frame_ben][2 * (i + 1) + 1] - ben_material_indices[cur_frame_ben][2 * i + 1];
		glDrawArrays(GL_TRIANGLES, cur_vertex_offset, n_vertices);
	}

	glBindVertexArray(0);

}

// wolf object
#define N_WOLF_FRAMES 17
GLuint wolf_VBO, wolf_VAO;
int wolf_n_triangles[N_WOLF_FRAMES];
int wolf_vertex_offset[N_WOLF_FRAMES];
GLfloat* wolf_vertices[N_WOLF_FRAMES];

Material_Parameters material_wolf;

void prepare_wolf(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, wolf_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_WOLF_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/wolf/wolf_%02d_vnt.geom", i);
		wolf_n_triangles[i] = read_geometry_vnt(&wolf_vertices[i], n_bytes_per_triangle, filename);

		// assume all geometry files are effective
		wolf_n_total_triangles += wolf_n_triangles[i];

		if (i == 0)
			wolf_vertex_offset[i] = 0;
		else
			wolf_vertex_offset[i] = wolf_vertex_offset[i - 1] + 3 * wolf_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &wolf_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glBufferData(GL_ARRAY_BUFFER, wolf_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_WOLF_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, wolf_vertex_offset[i] * n_bytes_per_vertex,
			wolf_n_triangles[i] * n_bytes_per_triangle, wolf_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_WOLF_FRAMES; i++)
		free(wolf_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &wolf_VAO);
	glBindVertexArray(wolf_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_wolf.ambient_color[0] = 0.5f;
	material_wolf.ambient_color[1] = 0.5f;
	material_wolf.ambient_color[2] = 0.5f;
	material_wolf.ambient_color[3] = 1.0f;

	material_wolf.diffuse_color[0] = 0.500000f;
	material_wolf.diffuse_color[1] = 0.500000f;
	material_wolf.diffuse_color[2] = 0.500000f;
	material_wolf.diffuse_color[3] = 1.0f;

	material_wolf.specular_color[0] = 0.005f;
	material_wolf.specular_color[1] = 0.005f;
	material_wolf.specular_color[2] = 0.005f;
	material_wolf.specular_color[3] = 1.0f;

	material_wolf.specular_exponent = 5.334717f;

	material_wolf.emissive_color[0] = 0.000000f;
	material_wolf.emissive_color[1] = 0.000000f;
	material_wolf.emissive_color[2] = 0.000000f;
	material_wolf.emissive_color[3] = 1.0f;
}

void draw_wolf(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(wolf_VAO);
	glDrawArrays(GL_TRIANGLES, wolf_vertex_offset[cur_frame_wolf], 3 * wolf_n_triangles[cur_frame_wolf]);
	glBindVertexArray(0);
}

// spider object
#define N_SPIDER_FRAMES 16
GLuint spider_VBO, spider_VAO;
int spider_n_triangles[N_SPIDER_FRAMES];
int spider_vertex_offset[N_SPIDER_FRAMES];
GLfloat* spider_vertices[N_SPIDER_FRAMES];

Material_Parameters material_spider;

void prepare_spider(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, spider_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_SPIDER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/spider/spider_vnt_%d%d.geom", i / 10, i % 10);
		spider_n_triangles[i] = read_geometry_vnt(&spider_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		spider_n_total_triangles += spider_n_triangles[i];

		if (i == 0)
			spider_vertex_offset[i] = 0;
		else
			spider_vertex_offset[i] = spider_vertex_offset[i - 1] + 3 * spider_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &spider_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glBufferData(GL_ARRAY_BUFFER, spider_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_SPIDER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, spider_vertex_offset[i] * n_bytes_per_vertex,
			spider_n_triangles[i] * n_bytes_per_triangle, spider_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_SPIDER_FRAMES; i++)
		free(spider_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &spider_VAO);
	glBindVertexArray(spider_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_spider.ambient_color[0] = 0.5f;
	material_spider.ambient_color[1] = 0.5f;
	material_spider.ambient_color[2] = 0.5f;
	material_spider.ambient_color[3] = 1.0f;

	material_spider.diffuse_color[0] = 0.9f;
	material_spider.diffuse_color[1] = 0.5f;
	material_spider.diffuse_color[2] = 0.1f;
	material_spider.diffuse_color[3] = 1.0f;

	material_spider.specular_color[0] = 0.5f;
	material_spider.specular_color[1] = 0.5f;
	material_spider.specular_color[2] = 0.5f;
	material_spider.specular_color[3] = 1.0f;

	material_spider.specular_exponent = 11.334717f;

	material_spider.emissive_color[0] = 0.000000f;
	material_spider.emissive_color[1] = 0.000000f;
	material_spider.emissive_color[2] = 0.000000f;
	material_spider.emissive_color[3] = 1.0f;
}

void draw_spider(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(spider_VAO);
	glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);
}

// dragon object
GLuint dragon_VBO, dragon_VAO;
int dragon_n_triangles;
GLfloat* dragon_vertices;

Material_Parameters material_dragon;

void prepare_dragon(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, dragon_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/dragon_vnt.geom");
	dragon_n_triangles = read_geometry_vnt(&dragon_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	dragon_n_total_triangles += dragon_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &dragon_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glBufferData(GL_ARRAY_BUFFER, dragon_n_total_triangles * 3 * n_bytes_per_vertex, dragon_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(dragon_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &dragon_VAO);
	glBindVertexArray(dragon_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_dragon.ambient_color[0] = 0.5f;
	material_dragon.ambient_color[1] = 0.5f;
	material_dragon.ambient_color[2] = 0.5f;
	material_dragon.ambient_color[3] = 1.0f;

	material_dragon.diffuse_color[0] = 0.3f;
	material_dragon.diffuse_color[1] = 0.3f;
	material_dragon.diffuse_color[2] = 0.4f;
	material_dragon.diffuse_color[3] = 1.0f;

	material_dragon.specular_color[0] = 0.3f;
	material_dragon.specular_color[1] = 0.3f;
	material_dragon.specular_color[2] = 0.4f;
	material_dragon.specular_color[3] = 1.0f;

	material_dragon.specular_exponent = 11.334717f;

	material_dragon.emissive_color[0] = 0.000000f;
	material_dragon.emissive_color[1] = 0.000000f;
	material_dragon.emissive_color[2] = 0.000000f;
	material_dragon.emissive_color[3] = 1.0f;
}

void draw_dragon(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(dragon_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * dragon_n_triangles);
	glBindVertexArray(0);
}

// optimus object
GLuint optimus_VBO, optimus_VAO;
int optimus_n_triangles;
GLfloat* optimus_vertices;

Material_Parameters material_optimus;

void prepare_optimus(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, optimus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/optimus_vnt.geom");
	optimus_n_triangles = read_geometry_vnt(&optimus_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	optimus_n_total_triangles += optimus_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &optimus_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glBufferData(GL_ARRAY_BUFFER, optimus_n_total_triangles * 3 * n_bytes_per_vertex, optimus_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(optimus_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &optimus_VAO);
	glBindVertexArray(optimus_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_optimus.ambient_color[0] = 0.5f;
	material_optimus.ambient_color[1] = 0.5f;
	material_optimus.ambient_color[2] = 0.5f;
	material_optimus.ambient_color[3] = 1.0f;

	material_optimus.diffuse_color[0] = 0.2f;
	material_optimus.diffuse_color[1] = 0.2f;
	material_optimus.diffuse_color[2] = 0.9f;
	material_optimus.diffuse_color[3] = 1.0f;

	material_optimus.specular_color[0] = 1.0f;
	material_optimus.specular_color[1] = 1.0f;
	material_optimus.specular_color[2] = 1.0f;
	material_optimus.specular_color[3] = 1.0f;

	material_optimus.specular_exponent = 52.334717f;

	material_optimus.emissive_color[0] = 0.000000f;
	material_optimus.emissive_color[1] = 0.000000f;
	material_optimus.emissive_color[2] = 0.000000f;
	material_optimus.emissive_color[3] = 1.0f;
}

void draw_optimus(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(optimus_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * optimus_n_triangles);
	glBindVertexArray(0);
}

// cow object
GLuint cow_VBO, cow_VAO;
int cow_n_triangles;
GLfloat* cow_vertices;

Material_Parameters material_cow;

void prepare_cow(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, cow_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/cow_vnt.geom");
	cow_n_triangles = read_geometry_vnt(&cow_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	cow_n_total_triangles += cow_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &cow_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cow_VBO);
	glBufferData(GL_ARRAY_BUFFER, cow_n_total_triangles * 3 * n_bytes_per_vertex, cow_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(cow_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &cow_VAO);
	glBindVertexArray(cow_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cow_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_cow.ambient_color[0] = 0.5f;
	material_cow.ambient_color[1] = 0.5f;
	material_cow.ambient_color[2] = 0.5f;
	material_cow.ambient_color[3] = 1.0f;

	material_cow.diffuse_color[0] = 0.7f;
	material_cow.diffuse_color[1] = 0.5f;
	material_cow.diffuse_color[2] = 0.2f;
	material_cow.diffuse_color[3] = 1.0f;

	material_cow.specular_color[0] = 0.4f;
	material_cow.specular_color[1] = 0.4f;
	material_cow.specular_color[2] = 0.2f;
	material_cow.specular_color[3] = 1.0f;

	material_cow.specular_exponent = 5.334717f;

	material_cow.emissive_color[0] = 0.000000f;
	material_cow.emissive_color[1] = 0.000000f;
	material_cow.emissive_color[2] = 0.000000f;
	material_cow.emissive_color[3] = 1.0f;
}

void draw_cow(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(cow_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * cow_n_triangles);
	glBindVertexArray(0);
}

// bike object
GLuint bike_VBO, bike_VAO;
int bike_n_triangles;
GLfloat* bike_vertices;

Material_Parameters material_bike;

void prepare_bike(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, bike_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/bike_vnt.geom");
	bike_n_triangles = read_geometry_vnt(&bike_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	bike_n_total_triangles += bike_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &bike_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, bike_VBO);
	glBufferData(GL_ARRAY_BUFFER, bike_n_total_triangles * 3 * n_bytes_per_vertex, bike_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(bike_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &bike_VAO);
	glBindVertexArray(bike_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, bike_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_bike.ambient_color[0] = 0.5f;
	material_bike.ambient_color[1] = 0.5f;
	material_bike.ambient_color[2] = 0.5f;
	material_bike.ambient_color[3] = 1.0f;

	material_bike.diffuse_color[0] = 0.800000f;
	material_bike.diffuse_color[1] = 0.800000f;
	material_bike.diffuse_color[2] = 0.900000f;
	material_bike.diffuse_color[3] = 1.0f;

	material_bike.specular_color[0] = 1.0f;
	material_bike.specular_color[1] = 1.0f;
	material_bike.specular_color[2] = 1.0f;
	material_bike.specular_color[3] = 1.0f;

	material_bike.specular_exponent = 522.334717f;

	material_bike.emissive_color[0] = 0.000000f;
	material_bike.emissive_color[1] = 0.000000f;
	material_bike.emissive_color[2] = 0.000000f;
	material_bike.emissive_color[3] = 1.0f;
}

void draw_bike(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(bike_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * bike_n_triangles);
	glBindVertexArray(0);
}

// bus object
GLuint bus_VBO, bus_VAO;
int bus_n_triangles;
GLfloat* bus_vertices;

Material_Parameters material_bus;

void prepare_bus(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, bus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/bus_vnt.geom");
	bus_n_triangles = read_geometry_vnt(&bus_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	bus_n_total_triangles += bus_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &bus_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, bus_VBO);
	glBufferData(GL_ARRAY_BUFFER, bus_n_total_triangles * 3 * n_bytes_per_vertex, bus_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(bus_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &bus_VAO);
	glBindVertexArray(bus_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, bus_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_bus.ambient_color[0] = 0.4f;
	material_bus.ambient_color[1] = 0.4f;
	material_bus.ambient_color[2] = 0.4f;
	material_bus.ambient_color[3] = 1.0f;

	material_bus.diffuse_color[0] = 0.96862f;
	material_bus.diffuse_color[1] = 0.90980f;
	material_bus.diffuse_color[2] = 0.79607f;
	material_bus.diffuse_color[3] = 1.0f;

	material_bus.specular_color[0] = 0.5f;
	material_bus.specular_color[1] = 0.5f;
	material_bus.specular_color[2] = 0.5f;
	material_bus.specular_color[3] = 1.0f;

	material_bus.specular_exponent = 50.334717f;

	material_bus.emissive_color[0] = 0.000000f;
	material_bus.emissive_color[1] = 0.000000f;
	material_bus.emissive_color[2] = 0.000000f;
	material_bus.emissive_color[3] = 1.0f;
}

void draw_bus(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(bus_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * bus_n_triangles);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

// godzilla object
GLuint godzilla_VBO, godzilla_VAO;
int godzilla_n_triangles;
GLfloat* godzilla_vertices;

Material_Parameters material_godzilla;

void prepare_godzilla(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, godzilla_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/godzilla_vnt.geom");
	godzilla_n_triangles = read_geometry_vnt(&godzilla_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	godzilla_n_total_triangles += godzilla_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &godzilla_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, godzilla_VBO);
	glBufferData(GL_ARRAY_BUFFER, godzilla_n_total_triangles * 3 * n_bytes_per_vertex, godzilla_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(godzilla_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &godzilla_VAO);
	glBindVertexArray(godzilla_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, godzilla_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_godzilla.ambient_color[0] = 0.5f;
	material_godzilla.ambient_color[1] = 0.5f;
	material_godzilla.ambient_color[2] = 0.5f;
	material_godzilla.ambient_color[3] = 1.0f;

	material_godzilla.diffuse_color[0] = 0.4f;
	material_godzilla.diffuse_color[1] = 0.4f;
	material_godzilla.diffuse_color[2] = 0.2f;
	material_godzilla.diffuse_color[3] = 1.0f;

	material_godzilla.specular_color[0] = 0.4f;
	material_godzilla.specular_color[1] = 0.4f;
	material_godzilla.specular_color[2] = 0.2f;
	material_godzilla.specular_color[3] = 1.0f;

	material_godzilla.specular_exponent = 5.334717f;

	material_godzilla.emissive_color[0] = 0.000000f;
	material_godzilla.emissive_color[1] = 0.000000f;
	material_godzilla.emissive_color[2] = 0.000000f;
	material_godzilla.emissive_color[3] = 1.0f;
}

void draw_godzilla(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(godzilla_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * godzilla_n_triangles);
	glBindVertexArray(0);
}

// ironman object
GLuint ironman_VBO, ironman_VAO;
int ironman_n_triangles;
GLfloat* ironman_vertices;

Material_Parameters material_ironman;

void prepare_ironman(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, ironman_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ironman_vnt.geom");
	ironman_n_triangles = read_geometry_vnt(&ironman_vertices, n_bytes_per_triangle, filename);
	ironman_n_triangles = read_geometry_vnt(&ironman_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ironman_n_total_triangles += ironman_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &ironman_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glBufferData(GL_ARRAY_BUFFER, ironman_n_total_triangles * 3 * n_bytes_per_vertex, ironman_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(ironman_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ironman_VAO);
	glBindVertexArray(ironman_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_ironman.ambient_color[0] = 0.4f;
	material_ironman.ambient_color[1] = 0.4f;
	material_ironman.ambient_color[2] = 0.4f;
	material_ironman.ambient_color[3] = 1.0f;

	material_ironman.diffuse_color[0] = 0.900000f;
	material_ironman.diffuse_color[1] = 0.200000f;
	material_ironman.diffuse_color[2] = 0.200000f;
	material_ironman.diffuse_color[3] = 1.0f;

	material_ironman.specular_color[0] = 1.0f;
	material_ironman.specular_color[1] = 1.0f;
	material_ironman.specular_color[2] = 1.0f;
	material_ironman.specular_color[3] = 1.0f;

	material_ironman.specular_exponent = 52.334717f;

	material_ironman.emissive_color[0] = 0.000000f;
	material_ironman.emissive_color[1] = 0.000000f;
	material_ironman.emissive_color[2] = 0.000000f;
	material_ironman.emissive_color[3] = 1.0f;
}

void draw_ironman(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(ironman_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);
	glBindVertexArray(0);
}

// tank object
GLuint tank_VBO, tank_VAO;
int tank_n_triangles;
GLfloat* tank_vertices;

Material_Parameters material_tank;

void prepare_tank(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, tank_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/tank_vnt.geom");
	tank_n_triangles = read_geometry_vnt(&tank_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	tank_n_total_triangles += tank_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &tank_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glBufferData(GL_ARRAY_BUFFER, tank_n_total_triangles * 3 * n_bytes_per_vertex, tank_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(tank_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &tank_VAO);
	glBindVertexArray(tank_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_tank.ambient_color[0] = 0.0f;
	material_tank.ambient_color[1] = 0.0f;
	material_tank.ambient_color[2] = 0.0f;
	material_tank.ambient_color[3] = 1.0f;

	material_tank.diffuse_color[0] = 0.200000f;
	material_tank.diffuse_color[1] = 0.980000f;
	material_tank.diffuse_color[2] = 0.500000f;
	material_tank.diffuse_color[3] = 1.0f;

	material_tank.specular_color[0] = 1.0f;
	material_tank.specular_color[1] = 1.0f;
	material_tank.specular_color[2] = 1.0f;
	material_tank.specular_color[3] = 1.0f;

	material_tank.specular_exponent = 52.334717f;

	material_tank.emissive_color[0] = 0.000000f;
	material_tank.emissive_color[1] = 0.000000f;
	material_tank.emissive_color[2] = 0.000000f;
	material_tank.emissive_color[3] = 1.0f;
}

void draw_tank(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(tank_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tank_n_triangles);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

// nathan object
#define N_NATHAN_FRAMES 69
GLuint nathan_VBO, nathan_VAO;
int nathan_n_triangles[N_NATHAN_FRAMES];
int nathan_vertex_offset[N_NATHAN_FRAMES];
GLfloat* nathan_vertices[N_NATHAN_FRAMES];

Material_Parameters material_nathan;

void prepare_nathan(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, nathan_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_NATHAN_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/nathan/rp_nathan_animated_003_walking%d.geom", i);
		nathan_n_triangles[i] = read_geometry_vnt(&nathan_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		nathan_n_total_triangles += nathan_n_triangles[i];

		if (i == 0)
			nathan_vertex_offset[i] = 0;
		else
			nathan_vertex_offset[i] = nathan_vertex_offset[i - 1] + 3 * nathan_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &nathan_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, nathan_VBO);
	glBufferData(GL_ARRAY_BUFFER, nathan_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_NATHAN_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, nathan_vertex_offset[i] * n_bytes_per_vertex,
			nathan_n_triangles[i] * n_bytes_per_triangle, nathan_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_NATHAN_FRAMES; i++)
		free(nathan_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &nathan_VAO);
	glBindVertexArray(nathan_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, nathan_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_nathan.ambient_color[0] = 0.0f;
	material_nathan.ambient_color[1] = 0.0f;
	material_nathan.ambient_color[2] = 0.0f;
	material_nathan.ambient_color[3] = 1.0f;

	material_nathan.diffuse_color[0] = 0.9922f;
	material_nathan.diffuse_color[1] = 0.9922f;
	material_nathan.diffuse_color[2] = 0.9922f;
	material_nathan.diffuse_color[3] = 1.0f;

	material_nathan.specular_color[0] = 0.1184f;
	material_nathan.specular_color[1] = 0.0665f;
	material_nathan.specular_color[2] = 0.0512f;
	material_nathan.specular_color[3] = 1.0f;

	material_nathan.specular_exponent = 50.334717f;

	material_nathan.emissive_color[0] = 0.000000f;
	material_nathan.emissive_color[1] = 0.000000f;
	material_nathan.emissive_color[2] = 0.000000f;
	material_nathan.emissive_color[3] = 1.0f;
}

void draw_nathan(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(nathan_VAO);
	glDrawArrays(GL_TRIANGLES, nathan_vertex_offset[cur_frame_nathan], 3 * nathan_n_triangles[cur_frame_nathan]);
	glBindVertexArray(0);
}

// cat object
GLuint cat_VBO, cat_VAO;
int cat_n_triangles;
GLfloat* cat_vertices;

Material_Parameters material_cat;

void prepare_cat(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, cat_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/cat_vnt.geom");
	cat_n_triangles = read_geometry_vnt(&cat_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	cat_n_total_triangles += cat_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &cat_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cat_VBO);
	glBufferData(GL_ARRAY_BUFFER, cat_n_total_triangles * 3 * n_bytes_per_vertex, cat_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(cat_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &cat_VAO);
	glBindVertexArray(cat_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cat_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_cat.ambient_color[0] = 0.5f;
	material_cat.ambient_color[1] = 0.5f;
	material_cat.ambient_color[2] = 0.5f;
	material_cat.ambient_color[3] = 1.0f;

	material_cat.diffuse_color[0] = 0.7f;
	material_cat.diffuse_color[1] = 0.5f;
	material_cat.diffuse_color[2] = 0.2f;
	material_cat.diffuse_color[3] = 1.0f;

	material_cat.specular_color[0] = 0.4f;
	material_cat.specular_color[1] = 0.4f;
	material_cat.specular_color[2] = 0.2f;
	material_cat.specular_color[3] = 1.0f;

	material_cat.specular_exponent = 5.334717f;

	material_cat.emissive_color[0] = 0.000000f;
	material_cat.emissive_color[1] = 0.000000f;
	material_cat.emissive_color[2] = 0.000000f;
	material_cat.emissive_color[3] = 1.0f;
}

void draw_cat(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(cat_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * cat_n_triangles);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

// ogre object
GLuint ogre_VBO, ogre_VAO;
int ogre_n_triangles;
GLfloat* ogre_vertices;

Material_Parameters material_ogre;

void prepare_ogre(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, ogre_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ogre_vnt.geom");
	ogre_n_triangles = read_geometry_vnt(&ogre_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ogre_n_total_triangles += ogre_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &ogre_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ogre_VBO);
	glBufferData(GL_ARRAY_BUFFER, ogre_n_total_triangles * 3 * n_bytes_per_vertex, ogre_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(ogre_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ogre_VAO);
	glBindVertexArray(ogre_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ogre_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_ogre.ambient_color[0] = 0.5f;
	material_ogre.ambient_color[1] = 0.5f;
	material_ogre.ambient_color[2] = 0.5f;
	material_ogre.ambient_color[3] = 1.0f;

	material_ogre.diffuse_color[0] = 0.012f;
	material_ogre.diffuse_color[1] = 0.7f;
	material_ogre.diffuse_color[2] = 0.57f;
	material_ogre.diffuse_color[3] = 1.0f;

	material_ogre.specular_color[0] = 0.4f;
	material_ogre.specular_color[1] = 0.4f;
	material_ogre.specular_color[2] = 0.2f;
	material_ogre.specular_color[3] = 1.0f;

	material_ogre.specular_exponent = 5.334717f;

	material_ogre.emissive_color[0] = 0.000000f;
	material_ogre.emissive_color[1] = 0.000000f;
	material_ogre.emissive_color[2] = 0.000000f;
	material_ogre.emissive_color[3] = 1.0f;
}

void draw_ogre(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(ogre_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ogre_n_triangles);
	glBindVertexArray(0);
}

// ant object
GLuint ant_VBO, ant_VAO;
int ant_n_triangles;
GLfloat* ant_vertices;

Material_Parameters material_ant;

void prepare_ant(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, ant_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ant_vnt.geom");
	ant_n_triangles = read_geometry_vnt(&ant_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ant_n_total_triangles += ant_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &ant_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ant_VBO);
	glBufferData(GL_ARRAY_BUFFER, ant_n_total_triangles * 3 * n_bytes_per_vertex, ant_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(ant_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ant_VAO);
	glBindVertexArray(ant_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ant_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// need to change values
	material_ant.ambient_color[0] = 0.5f;
	material_ant.ambient_color[1] = 0.5f;
	material_ant.ambient_color[2] = 0.5f;
	material_ant.ambient_color[3] = 1.0f;

	material_ant.diffuse_color[0] = 0.7f;
	material_ant.diffuse_color[1] = 0.5f;
	material_ant.diffuse_color[2] = 0.2f;
	material_ant.diffuse_color[3] = 1.0f;

	material_ant.specular_color[0] = 0.4f;
	material_ant.specular_color[1] = 0.4f;
	material_ant.specular_color[2] = 0.2f;
	material_ant.specular_color[3] = 1.0f;

	material_ant.specular_exponent = 5.334717f;

	material_ant.emissive_color[0] = 0.000000f;
	material_ant.emissive_color[1] = 0.000000f;
	material_ant.emissive_color[2] = 0.000000f;
	material_ant.emissive_color[3] = 1.0f;
}

void draw_ant(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(ant_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ant_n_triangles);
	glBindVertexArray(0);
}

// city object
GLuint city_VBO, city_VAO;
int city_n_triangles;
GLfloat* city_vertices;

int city_n_material_indices;
int* city_material_indices;

int city_n_materials;
char** city_diffuse_texture;
GLuint* city_texture_names;

Material_Parameters* material_city;

void prepare_city(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, city_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	bool bOnce = false;
	sprintf(filename, "Data/static_objects/BlenderCity_vntm.geom");
	city_n_triangles = read_geometry_vntm(&city_vertices, n_bytes_per_triangle,
		&city_n_material_indices,
		&city_material_indices,
		&city_n_materials,
		&city_diffuse_texture,
		&material_city,
		&bOnce,
		filename);

	// assume all geometry files are effective
	city_n_total_triangles += city_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &city_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, city_VBO);
	glBufferData(GL_ARRAY_BUFFER, city_n_total_triangles * n_bytes_per_triangle, city_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(city_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &city_VAO);
	glBindVertexArray(city_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, city_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_city(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(city_VAO);

	for (int i = 0; i < city_n_material_indices - 1; i++) {
		int cur_material_id = city_material_indices[2 * i];
		int cur_vertex_offset = city_material_indices[2 * i + 1];
		int n_vertices = city_material_indices[2 * (i + 1) + 1] - city_material_indices[2 * i + 1];
		glDrawArrays(GL_TRIANGLES, cur_vertex_offset, n_vertices);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(0);
}

// helicopter object
GLuint helicopter_VBO, helicopter_VAO;
int helicopter_n_triangles;
GLfloat* helicopter_vertices;

int helicopter_n_material_indices;
int* helicopter_material_indices;

int helicopter_n_materials;
char** helicopter_diffuse_texture;
GLuint* helicopter_texture_names;

Material_Parameters* material_helicopter;

void prepare_helicopter(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, helicopter_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	bool bOnce = false;

	sprintf(filename, "Data/static_objects/helicopter_vntm.geom");
	helicopter_n_triangles = read_geometry_vntm(&helicopter_vertices, n_bytes_per_triangle,
		&helicopter_n_material_indices,
		&helicopter_material_indices,
		&helicopter_n_materials,
		&helicopter_diffuse_texture,
		&material_helicopter,
		&bOnce,
		filename);

	// assume all geometry files are effective
	helicopter_n_total_triangles += helicopter_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &helicopter_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, helicopter_VBO);
	glBufferData(GL_ARRAY_BUFFER, helicopter_n_total_triangles * n_bytes_per_triangle, helicopter_vertices, GL_STATIC_DRAW);

	free(helicopter_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &helicopter_VAO);
	glBindVertexArray(helicopter_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, helicopter_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_helicopter(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(helicopter_VAO);

	for (int i = 0; i < helicopter_n_material_indices - 1; i++) {
		int cur_material_id = helicopter_material_indices[2 * i];
		int cur_vertex_offset = helicopter_material_indices[2 * i + 1];
		int n_vertices = helicopter_material_indices[2 * (i + 1) + 1] - helicopter_material_indices[2 * i + 1];
		glDrawArrays(GL_TRIANGLES, cur_vertex_offset, n_vertices);
	}

	glBindVertexArray(0);
}

// tower object
GLuint tower_VBO, tower_VAO;
int tower_n_triangles;
GLfloat* tower_vertices;

Material_Parameters material_tower;

void prepare_tower(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, tower_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/woodTower_vnt.geom");
	tower_n_triangles = read_geometry_vnt(&tower_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	tower_n_total_triangles += tower_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &tower_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tower_VBO);
	glBufferData(GL_ARRAY_BUFFER, tower_n_total_triangles * 3 * n_bytes_per_vertex, tower_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(tower_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &tower_VAO);
	glBindVertexArray(tower_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tower_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_tower.ambient_color[0] = 1.0f;
	material_tower.ambient_color[1] = 1.0f;
	material_tower.ambient_color[2] = 1.0f;
	material_tower.ambient_color[3] = 1.0f;

	material_tower.diffuse_color[0] = 0.4f;
	material_tower.diffuse_color[1] = 0.4f;
	material_tower.diffuse_color[2] = 0.4f;
	material_tower.diffuse_color[3] = 1.0f;

	material_tower.specular_color[0] = 0.0f;
	material_tower.specular_color[1] = 0.0f;
	material_tower.specular_color[2] = 0.0f;
	material_tower.specular_color[3] = 1.0f;

	material_tower.specular_exponent = 0.0f;

	material_tower.emissive_color[0] = 0.000000f;
	material_tower.emissive_color[1] = 0.000000f;
	material_tower.emissive_color[2] = 0.000000f;
	material_tower.emissive_color[3] = 1.0f;
}

void draw_tower(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(tower_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tower_n_triangles);
	glBindVertexArray(0);
}
/********************************* END: objects *********************************/