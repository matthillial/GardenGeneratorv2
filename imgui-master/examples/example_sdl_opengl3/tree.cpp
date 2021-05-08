//#include "tree.h"

//#include "../OpenGLStarterCode/glad/glad.h"  //Include order can matter here
//#include <glad/glad.h>
#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>

//For Visual Studios
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define GLM_FORCE_RADIANS
//#include "glm.hpp"
//#include "gtc/matrix_transform.hpp"
//#include "gtc/type_ptr.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


#include <cstdio>

#include <iostream>
#include <string>
#include <stack>
#include <map>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <ctime>
using namespace std;

#pragma once
class tree
{
public:
	string structure;
	vector<pair<char, float>> structurev2;
	glm::vec3 rootPos;
	float angle;
	float segLength;
	int growthTime;
	int segments;
	int segmentLimit = 256;
	map<char, string> rules;
	int vertices = 0;
	float growthSpeed;

	float segment2d[18] = {
		 -0.5f, 0.0f, 0.f,
		 0.5f , 1.0f, 0.f,
		-0.5f , 1.0f, 0.f,
		-0.5f , 0.0f, 0.f,
		 0.5f , 0.0f, 0.f,
		 0.5f , 1.0f, 0.f
	};
	float segment[288] = {
		// X      Y     Z     R     G      B      U      V
		  -0.5f, 0.0f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		  0.5f, 0.0f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		  0.5f,  1.0f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		  0.5f,  1.0f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		  -0.5f,  1.0f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		  -0.5f, 0.0f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		  -0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		  0.5f,  0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		  0.5f,  1.0f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		  0.5f,  1.0f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		  -0.5f,  1.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		  -0.5f,  0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

		  -0.5f,  1.0f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		  -0.5f,  1.0f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		  -0.5f,  0.0f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		  -0.5f,  0.0f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		  -0.5f,  0.0f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		  -0.5f,  1.0f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

		  0.5f,  1.0f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		  0.5f,  1.0f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		  0.5f,  0.0f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		  0.5f,  0.0f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		  0.5f,  0.0f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		  0.5f,  1.0f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

		  -0.5f,  0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		  0.5f,  0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		  0.5f,  0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		  0.5f,  0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		  -0.5f,  0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		  -0.5f,  0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

		  -0.5f,  1.0f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
		  0.5f,  1.0f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		  0.5f,  1.0f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
		  0.5f,  1.0f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
		  -0.5f,  1.0f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		  -0.5f,  1.0f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};
	vector<float> treeModel;
	float color[3] = { 0.5f, 0.27f, 0.07f };

	GLuint vao;
	GLuint vbo;
	//map<char, pair<char, float>[]> rulesv2;
	tree(vector<pair<char, float>> start, map<char, string> rulesIn, glm::vec3 root, float angleIn, float segLengthIn) {
		structurev2 = start;
		rootPos = root;
		angle = angleIn;
		segLength = segLengthIn;
		growthTime = 11;
		rules = rulesIn;
		segments = 0;
		growthSpeed = 0.01;
		for (int i = 0; i < start.size(); i++) {
			if (start.at(i).first == 'F') segments++;
		}

		glGenVertexArrays(1, &vao); //Create a VAO
		glBindVertexArray(vao); //Bind the above created VAO to the current context

		//printf("%d\n", vertices);
		//printf("Vertex: %f, %f, %f; \t%f, %f, %f \n", treeModel[0], treeModel[1], treeModel[2], treeModel[3], treeModel[4], treeModel[5]);

		glGenBuffers(1, &vbo);  //Create 1 buffer called vbo


		srand(static_cast <unsigned> (time(0)));
		//rulesv2 = rulesIn;
	}

	void initGL(GLint posAttrib, GLint colAttrib) {



	}

	void render(GLuint shaderProgram, GLint uniModel, glm::mat4 view, glm::mat4 proj) {
		growthTime--;
		stack<glm::mat4> branchNodes;
		glm::mat4 model = glm::mat4(1);
		model = glm::translate(model, rootPos);
		vector<pair<char, float>> newStructure;
		int frameSkip = 10;



		if (growthTime % frameSkip == 0) {
			if (growthTime <= 0) printf("GROW\n");
			vertices = 0;
			treeModel.clear();
			for (int i = 0; i < structurev2.size(); i++) {
				//Apply growth rules
				if (growthTime <= 0) {

					printf("%c, %f; ", structurev2.at(i).first, structurev2.at(i).second);
					newStructure.push_back(structurev2.at(i));
					if (rules.count(structurev2.at(i).first) > 0) {
						string insertRules = rules[structurev2.at(i).first];
						for (int j = 1; j < insertRules.length(); j++) {
							if (insertRules.at(j) == 'F') {
								segments++;
								newStructure.push_back(make_pair(insertRules.at(j), 0.0f));
							}
							else if (insertRules.at(j) == '-' || insertRules.at(j) == '+') {
								newStructure.push_back(make_pair(insertRules.at(j), static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 360))));
							}
							else newStructure.push_back(make_pair(insertRules.at(j), 0.0f));
						}
					}
					/*else {
						newStructure.push_back(structurev2.at(i));
					}*/
				}

				if (structurev2.at(i).first == 'F') {
					structurev2.at(i).second += growthSpeed / 10 * frameSkip;
					//printf("height: %f\n", structurev2.at(i).second);
					glm::mat4 scaleModel = glm::scale(model, glm::vec3(0.01, structurev2.at(i).second, 0.01));
					//scaleModel = glm::translate(scaleModel, glm::vec3(0, 5, 0));
					//transform segment vertices by scalemodel and add to treemodel
					for (int j = 0; j < sizeof(segment)/sizeof(segment[0]); j += 8) {
						glm::vec4 vertex = scaleModel * glm::vec4(segment[j], segment[j + 1], segment[j + 2], 1);
						//printf("Vertex: %f, %f, %f\n", vertex[0], vertex[1], vertex[2]);
						treeModel.push_back(vertex[0]);
						treeModel.push_back(vertex[1]);
						treeModel.push_back(vertex[2]);
						treeModel.push_back(color[0]);
						treeModel.push_back(color[1]);
						treeModel.push_back(color[2]);
						treeModel.push_back(0.0f); //u
						treeModel.push_back(0.0f); //v
						vertices++;
					}
					//glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(scaleModel));
					//glBindVertexArray(vao);  //Bind the VAO for the shader(s) we are using
					//glDrawArrays(GL_TRIANGLES, 0, 6); //Number of vertices
					model = glm::translate(model, glm::vec3(0, structurev2.at(i).second, 0));
				}
				if (structurev2.at(i).first == '+') {
					model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 0, 1));
					model = glm::rotate(model, glm::radians(structurev2.at(i).second), glm::vec3(0, 1, 0));
				}
				if (structurev2.at(i).first == '-') {
					model = glm::rotate(model, -glm::radians(angle), glm::vec3(0, 0, 1));
					model = glm::rotate(model, -glm::radians(structurev2.at(i).second), glm::vec3(0, 1, 0));
				}
				if (structurev2.at(i).first == '[') {
					branchNodes.push(model);
				}
				if (structurev2.at(i).first == ']') {
					model = branchNodes.top();
					branchNodes.pop();
				}
			}
		}


		GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		//Attribute, vals/attrib., type, normalized?, stride, offset
		//Binds to VBO current GL_ARRAY_BUFFER
		glEnableVertexAttribArray(posAttrib);

		GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		//Attribute, vals/attrib., type, normalized?, stride, offset
		//Binds to VBO current GL_ARRAY_BUFFER
		glEnableVertexAttribArray(colAttrib);


		model = glm::mat4(1);
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(vao);

		//glBindVertexArray(0); //Unbind the VAO once we have set all the attributes


		glUseProgram(shaderProgram); //Set the active shader (only one can be used at a time)
		glBindBuffer(GL_ARRAY_BUFFER, vbo); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
		glBufferData(GL_ARRAY_BUFFER, vertices * 8 * sizeof(float), &treeModel[0], GL_STATIC_DRAW); //upload vertices to vbo
		glDrawArrays(GL_TRIANGLES, 0, vertices); //Number of vertices*/

		if (growthTime <= 0) {
			structurev2 = newStructure;
			growthTime = 1000;
			printf("\n");
		}
	}
};
