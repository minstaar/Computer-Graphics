#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0

int win_width = 0, win_height = 0; 
float centerx = 0.0f, centery = 0.0f;

// 2D 물체 정의 부분은 objects.h 파일로 분리
// 새로운 물체 추가 시 prepare_scene() 함수에서 해당 물체에 대한 prepare_***() 함수를 수행함.
// cleanup() 함수에서 해당 resource를 free 시킴.
#include "objects.h"

int leftbuttonpressed = 0;
int animation_mode = 1;
int draw_airplane_and_house_mode = 1, draw_wheels_mode = 0, draw_satellite_mode = 0, draw_halt_mode = 0;
unsigned int timestamp = 0;

void timer(int value) {
	timestamp = (timestamp + 1) % UINT_MAX;
	glutPostRedisplay();
	if (animation_mode)
		glutTimerFunc(10, timer, 0);
}

void display(void) {
	glm::mat4 ModelMatrix;
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	ModelMatrix = glm::mat4(1.0f);
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes();
    draw_house(); // in MC

	if (draw_airplane_and_house_mode) {
		int house_clock = (timestamp % 1442) / 2 - 360; // -360 <= house_clock <= 360 
		float rotation_angle_house = atanf(100.0f * TO_RADIAN * cosf(house_clock * TO_RADIAN));
		float scaling_factor_house = 5.0f * (1.0f - fabs(cosf(house_clock * TO_RADIAN)));

		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(centerx, centery, 0.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3((float)house_clock,
			100.0f * sinf(house_clock * TO_RADIAN), 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scaling_factor_house, scaling_factor_house, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, rotation_angle_house, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_house(); // in WC

		int airplane_clock = timestamp % 720; // 0 <= airplane_clock <= 719 
		if (airplane_clock <= 360) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, airplane_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));
		}
		else {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, -(airplane_clock)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));

			if (airplane_clock <= 540)
				airplane_s_factor = (airplane_clock - 360.0f) / 180.0f + 1.0f;
			else
				airplane_s_factor = -(airplane_clock - 540.0f) / 180.0f + 2.0f;
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(airplane_s_factor, airplane_s_factor, 1.0f));
		}

		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_airplane();
	}

	if (draw_wheels_mode) {
		for (int i = 0; i < 4; i++) {
			ModelMatrix = glm::rotate(glm::mat4(1.0f), i * 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(250.0f, 0.0f, 0.0f));

			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_house();


//			for (int j = 0; j < 12; j++) {
			for (int j = 0; j < 1; j++) {
				glm::mat4 ModelMatrix_2 = glm::rotate(ModelMatrix, (j * 30.0f + timestamp) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
				ModelMatrix_2 = glm::translate(ModelMatrix_2, glm::vec3(100.0f, 0.0f, 0.0f));

				ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_2;
				glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
				draw_hat();
				draw_cake();

				if (draw_satellite_mode) {
					for (int k = 0; k < 4; k++) {
						glm::mat4 ModelMatrix_3;
						if (draw_halt_mode)
							ModelMatrix_3 = glm::rotate(ModelMatrix_2, -(k * 90.0f + timestamp) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
						else
							ModelMatrix_3 = glm::rotate(ModelMatrix_2, (k * 90.0f + timestamp) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

						ModelMatrix_3 = glm::translate(ModelMatrix_3, glm::vec3(100.0f, 0.0f, 0.0f));
						ModelMatrix_3 = glm::scale(ModelMatrix_3, glm::vec3(0.75f, 0.75f, 0.75f));

						ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_3;
						glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
						switch (k % 3) {
						case 0:
							draw_cocktail();
							break;
						case 1:
							draw_airplane();
							break;
						case 2:
							draw_sword();
							break;
						}
					}
				}
			}
		}
	}

	glFlush();	
}   

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		animation_mode = 1 - animation_mode;
		if (animation_mode)
			glutTimerFunc(10, timer, 0);
		break;
	case 'b':
		draw_airplane_and_house_mode = 1 - draw_airplane_and_house_mode;
		glutPostRedisplay();
		break;
	case 'w':
		draw_wheels_mode = 1 - draw_wheels_mode;
		glutPostRedisplay();
		break;
	case 's':
		draw_satellite_mode = 1 - draw_satellite_mode;
		glutPostRedisplay();
		break;
	case 'h':
		draw_halt_mode = 1 - draw_halt_mode;
		glutPostRedisplay();
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void mouse(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		leftbuttonpressed = 1;
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
		leftbuttonpressed = 0;
}

void motion(int x, int y) {
	if (leftbuttonpressed) {
		centerx =  x - win_width/2.0f, centery = (win_height - y) - win_height/2.0f;
		glutPostRedisplay();
	}
} 
	
void reshape(int width, int height) {
	win_width = width, win_height = height;
	
  	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0, 
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	update_axes();

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);

	glDeleteVertexArrays(1, &VAO_airplane);
	glDeleteBuffers(1, &VBO_airplane);

	glDeleteVertexArrays(1, &VAO_house);
	glDeleteBuffers(1, &VBO_house);

	glDeleteVertexArrays(1, &VAO_cake);
	glDeleteBuffers(1, &VBO_cake);

	glDeleteVertexArrays(1, &VAO_hat);
	glDeleteBuffers(1, &VBO_hat);

	glDeleteVertexArrays(1, &VAO_cocktail);
	glDeleteBuffers(1, &VBO_cocktail);

	glDeleteVertexArrays(1, &VAO_sword);
	glDeleteBuffers(1, &VBO_sword);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(10, timer, 0); // animation_mode = 1
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE); 
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glClearColor(44 / 255.0f, 180 / 255.0f, 49 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_airplane();
	prepare_house();
	prepare_cake();
	prepare_hat();
	prepare_cocktail();
	prepare_sword();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program(); 
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

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

#define N_MESSAGE_LINES 2
int main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 Simple2DTransformationMotion_Hier_3.0.3";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'a', 'b', 'w', 's', 'h', 'ESC'"
		"    - Mouse used: L-click and move"
	};

	glutInit (&argc, argv);
 	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize (1200, 800);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
	return 0;
}


