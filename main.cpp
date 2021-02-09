// Include standard headers
#include <iostream>
#include <vector>
using namespace std;

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include <shader.hpp>
#include <texture.hpp>
#include <controls.hpp>
#include <objloader.hpp>
#include <vboindexer.hpp>

int main(void)
{
	//initialise glfw
	if (!glfwInit())
	{
		cout << "Could not initialise GLFW" << endl;
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create window
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Scene", NULL, NULL);
	if (window == NULL)
	{
		cout << "Could not create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		cout << "Could not initialise GLEW" << endl;
		glfwTerminate();
		return -1;
	}

	// tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//and to make sure we can escape using the sticky key
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//shaders
	GLuint textureShader = LoadShaders("vertShader.vert", "fragShader.frag");
	GLuint depthShader = LoadShaders("vertDepth.vert", "fragDepth.frag");
	GLuint solidShader = LoadShaders("vertSolid.vert", "fragSolid.frag");
	//GLuint debugShader = LoadShaders("debugShader.vert", "debugShader.frag");

	//textures
	GLuint Texture = loadBMP_custom("checkerBoard.bmp");
/*	GLuint TextureR = loadBMP_custom("RED.BMP");
	GLuint TextureG = loadBMP_custom("GRN.BMP");
	GLuint TextureB = loadBMP_custom("BLU.BMP");
*/
	GLuint depthMVPID = glGetUniformLocation(depthShader, "MVP");

	GLuint ProjID = glGetUniformLocation(textureShader, "P");
	GLuint ModelID = glGetUniformLocation(textureShader, "M");
	GLuint ViewID = glGetUniformLocation(textureShader, "V");
	GLuint lightProjID = glGetUniformLocation(textureShader, "lightP");
	GLuint lightViewID = glGetUniformLocation(textureShader, "lightV"); 
	GLuint lightPosID = glGetUniformLocation(textureShader, "lightPosition");

	GLuint TextureID = glGetUniformLocation(textureShader, "Texture");
	GLuint depthMapID = glGetUniformLocation(textureShader, "depthMap");
	GLuint cameraPosID = glGetUniformLocation(textureShader, "cameraPosition");

	//add handles for non-image texture shaders
	GLuint ProjSolidID = glGetUniformLocation(solidShader, "P");
	GLuint ModelSolidID = glGetUniformLocation(solidShader, "M");
	GLuint ViewSolidID = glGetUniformLocation(solidShader, "V");
	GLuint lightProjSolidID = glGetUniformLocation(solidShader, "lightP");
	GLuint lightViewSolidID = glGetUniformLocation(solidShader, "lightV");

	GLuint lightPosSolidID = glGetUniformLocation(solidShader, "lightPosition");
	GLuint depthMapSolidID = glGetUniformLocation(solidShader, "depthMap");
	GLuint cameraPosSolidID = glGetUniformLocation(solidShader, "cameraPosition");
	GLuint tSolidID = glGetUniformLocation(solidShader, "t");
	float t = 0.0f;

	//GLuint debugMapID = glGetUniformLocation(debugShader, "depthMap");

	//first depth map FBO
	GLuint depthMapFBO1;
	glGenFramebuffers(1, &depthMapFBO1);

	GLuint depthMap1;
	glGenTextures(1, &depthMap1);
	glBindTexture(GL_TEXTURE_2D, depthMap1);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO1);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap1, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Load plane model
	vector<vec3> V;
	vector<vec2> T;
	vector<vec3> N;
	if (!(loadOBJ("plane.obj", V, T, N)))
	{
		cout << "Plane failed to load" << endl;
		return -1;
	}

	vector<unsigned short> indices;
	vector<vec3> indexN, indexV;
	vector<vec2> indexT;
	indexVBO(V, T, N, indices, indexV, indexT, indexN);

	//Load into VBO

	GLuint planeV;
	glGenBuffers(1, &planeV);
	glBindBuffer(GL_ARRAY_BUFFER, planeV);
	glBufferData(GL_ARRAY_BUFFER, indexV.size()*sizeof(vec3), &indexV[0], GL_STATIC_DRAW);

	GLuint planeT;
	glGenBuffers(1, &planeT);
	glBindBuffer(GL_ARRAY_BUFFER, planeT);
	glBufferData(GL_ARRAY_BUFFER, indexT.size()*sizeof(vec2), &indexT[0], GL_STATIC_DRAW);

	GLuint planeN;
	glGenBuffers(1, &planeN);
	glBindBuffer(GL_ARRAY_BUFFER, planeN);
	glBufferData(GL_ARRAY_BUFFER, indexN.size()*sizeof(vec3), &indexN[0], GL_STATIC_DRAW);

	GLuint planeInd;
	glGenBuffers(1, &planeInd);
	glBindBuffer(GL_ARRAY_BUFFER, planeInd);
	glBufferData(GL_ARRAY_BUFFER, indices.size()*sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	GLuint planeNum = indices.size();

	//clear everything
	V.clear();
	T.clear();
	N.clear();

	indices.clear();
	indexV.clear();
	indexT.clear();
	indexN.clear();

	//Load cone model
	if (!(loadOBJ("cone.obj", V, T, N)))
	{
		cout << "Cone failed to load" << endl;
		return -1;
	}
	indexVBO(V, T, N, indices, indexV, indexT, indexN);

	//Load into VBO

	GLuint coneV;
	glGenBuffers(1, &coneV);
	glBindBuffer(GL_ARRAY_BUFFER, coneV);
	glBufferData(GL_ARRAY_BUFFER, indexV.size()*sizeof(vec3), &indexV[0], GL_STATIC_DRAW);

	GLuint coneT;
	glGenBuffers(1, &coneT);
	glBindBuffer(GL_ARRAY_BUFFER, coneT);
	glBufferData(GL_ARRAY_BUFFER, indexT.size()*sizeof(vec2), &indexT[0], GL_STATIC_DRAW);

	GLuint coneN;
	glGenBuffers(1, &coneN);
	glBindBuffer(GL_ARRAY_BUFFER, coneN);
	glBufferData(GL_ARRAY_BUFFER, indexN.size()*sizeof(vec3), &indexN[0], GL_STATIC_DRAW);

	GLuint coneInd;
	glGenBuffers(1, &coneInd);
	glBindBuffer(GL_ARRAY_BUFFER, coneInd);
	glBufferData(GL_ARRAY_BUFFER, indices.size()*sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	GLuint coneNum = indices.size();

	//clear everything
	V.clear();
	T.clear();
	N.clear();

	indices.clear();
	indexV.clear();
	indexT.clear();
	indexN.clear();

	//Load cube model
	if (!(loadOBJ("cube.obj", V, T, N)))
	{
		cout << "Cube failed to load" << endl;
		return -1;
	}
	indexVBO(V, T, N, indices, indexV, indexT, indexN);

	//Load into VBO

	GLuint cubeV;
	glGenBuffers(1, &cubeV);
	glBindBuffer(GL_ARRAY_BUFFER, cubeV);
	glBufferData(GL_ARRAY_BUFFER, indexV.size()*sizeof(vec3), &indexV[0], GL_STATIC_DRAW);

	GLuint cubeT;
	glGenBuffers(1, &cubeT);
	glBindBuffer(GL_ARRAY_BUFFER, cubeT);
	glBufferData(GL_ARRAY_BUFFER, indexT.size()*sizeof(vec2), &indexT[0], GL_STATIC_DRAW);

	GLuint cubeN;
	glGenBuffers(1, &cubeN);
	glBindBuffer(GL_ARRAY_BUFFER, cubeN);
	glBufferData(GL_ARRAY_BUFFER, indexN.size()*sizeof(vec3), &indexN[0], GL_STATIC_DRAW);

	GLuint cubeInd;
	glGenBuffers(1, &cubeInd);
	glBindBuffer(GL_ARRAY_BUFFER, cubeInd);
	glBufferData(GL_ARRAY_BUFFER, indices.size()*sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	GLuint cubeNum = indices.size();

	//clear everything
	V.clear();
	T.clear();
	N.clear();

	indices.clear();
	indexV.clear();
	indexT.clear();
	indexN.clear();

	//Load sphere model
	if (!(loadOBJ("sphere.obj", V, T, N)))
	{
		cout << "Sphere failed to load" << endl;
		return -1;
	}
	indexVBO(V, T, N, indices, indexV, indexT, indexN);

	//Load into VBO

	GLuint sphereV;
	glGenBuffers(1, &sphereV);
	glBindBuffer(GL_ARRAY_BUFFER, sphereV);
	glBufferData(GL_ARRAY_BUFFER, indexV.size()*sizeof(vec3), &indexV[0], GL_STATIC_DRAW);

	GLuint sphereT;
	glGenBuffers(1, &sphereT);
	glBindBuffer(GL_ARRAY_BUFFER, sphereT);
	glBufferData(GL_ARRAY_BUFFER, indexT.size()*sizeof(vec2), &indexT[0], GL_STATIC_DRAW);

	GLuint sphereN;
	glGenBuffers(1, &sphereN);
	glBindBuffer(GL_ARRAY_BUFFER, sphereN);
	glBufferData(GL_ARRAY_BUFFER, indexN.size()*sizeof(vec3), &indexN[0], GL_STATIC_DRAW);

	GLuint sphereInd;
	glGenBuffers(1, &sphereInd);
	glBindBuffer(GL_ARRAY_BUFFER, sphereInd);
	glBufferData(GL_ARRAY_BUFFER, indices.size()*sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	GLuint sphereNum = indices.size();

	//clear everything
	V.clear();
	T.clear();
	N.clear();

	indices.clear();
	indexV.clear();
	indexT.clear();
	indexN.clear();

	vec3 lightPos(5.0, 5.0, 5.0);

	while ((glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) && (!glfwWindowShouldClose(window)))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO1);
		glViewport(0,0,1024,1024);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(depthShader);

		//first depth map

		mat4 lightProj = perspective(radians(90.0f), 1.0f, 1.0f, 20.0f);
		mat4 lightView = lookAt(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat4 lightModel = mat4(1.0f);
		mat4 depthMVP = lightProj * lightView * lightModel;

		glUniformMatrix4fv(depthMVPID, 1, GL_FALSE, &depthMVP[0][0]);

		//render scene

		//plane

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, planeV);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeInd);

		glDrawElements(GL_TRIANGLES, planeNum, GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);

		//cone

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, coneV);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, coneInd);

		glDrawElements(GL_TRIANGLES, coneNum, GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);

		//cube

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, cubeV);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeInd);

		glDrawElements(GL_TRIANGLES, cubeNum, GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);

		//sphere

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, sphereV);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereInd);

		glDrawElements(GL_TRIANGLES, sphereNum, GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);

		//the actual scene

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, 1024, 768);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(textureShader);

		glUniformMatrix4fv(lightProjID, 1, GL_FALSE, &lightProj[0][0]);
		glUniformMatrix4fv(lightViewID, 1, GL_FALSE, &lightView[0][0]);

		computeMatricesFromInputs(window);
		mat4 proj = getProjectionMatrix();
		mat4 view = getViewMatrix();
		mat4 model = mat4(1.0);

		//set shader variables
		glUniformMatrix4fv(ProjID, 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(ModelID, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(ViewID, 1, GL_FALSE, &view[0][0]);
		glUniform3f(lightPosID, lightPos.x, lightPos.y, lightPos.z);

		glUniform1i(TextureID, 0);
		glUniform1i(depthMapID, 1);
		glUniform3f(cameraPosID, getPos().x, getPos().y, getPos().z);

		//rendering time!

		//plane

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap1);

		//vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, planeV);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//uvs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, planeT);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//normals

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, planeN);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeInd);

		glDrawElements(GL_TRIANGLES, planeNum, GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		//activate non-image shader around this point
		//and set variables
		glUseProgram(solidShader);

		glUniformMatrix4fv(ProjSolidID, 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(ModelSolidID, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(ViewSolidID, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(lightProjSolidID, 1, GL_FALSE, &lightProj[0][0]);
		glUniformMatrix4fv(lightViewSolidID, 1, GL_FALSE, &lightView[0][0]);

		glUniform3f(lightPosSolidID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(cameraPosSolidID, getPos().x, getPos().y, getPos().z);
		glUniform1i(depthMapSolidID, 0);
		glUniform1f(tSolidID, t);
		
		//cone

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap1);

		//vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, coneV);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//uvs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, coneT);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//normals

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, coneN);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, coneInd);

		glDrawElements(GL_TRIANGLES, coneNum, GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		//cube

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap1);

		//vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, cubeV);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//uvs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, cubeT);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//normals

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, cubeN);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeInd);

		glDrawElements(GL_TRIANGLES, cubeNum, GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		//sphere

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap1);

		//vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, sphereV);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//uvs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, sphereT);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//normals

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, sphereN);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereInd);

		glDrawElements(GL_TRIANGLES, sphereNum, GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		//draw depth map for debugging purposes
/*
		glViewport(0, 0, 256, 256);
		glUseProgram(debugShader);
		glUniform1i(debugMapID, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap1);
		float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
		GLuint quadVAO = 0, quadVBO;
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindVertexArray(quadVAO);
    	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    	glBindVertexArray(0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
*/

		if (t >= 1.0) t = 0.0f;
		else t += 0.01;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}