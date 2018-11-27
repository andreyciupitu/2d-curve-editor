#include "Laborator2.h"

#include <vector>
#include <iostream>

#include <Core/Engine.h>

using namespace std;

// Order of function calling can be seen in "Source/Core/World.cpp::LoopUpdate()"
// https://github.com/UPB-Graphics/Framework-EGC/blob/master/Source/Core/World.cpp

glm::vec3* selected = nullptr;

Laborator2::Laborator2()
{
}

Laborator2::~Laborator2()
{
}

void Laborator2::Init()
{
	auto camera = GetSceneCamera();
	camera->SetOrthographic(16.0f, 9.0f, 0.01, 100.0f);
	camera->SetPositionAndRotation(glm::vec3(0, 5.0f, 0), glm::quat(glm::vec3(-90 * TO_RADIANS, 0, 0)));
	camera->Update();

	ToggleGroundPlane();

	// Create a shader program for surface generation
	{
		Shader *shader = new Shader("SurfaceGeneration");
		shader->AddShader("Source/Laboratoare/Laborator2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Laborator2/Shaders/GeometryShader.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Laborator2/Shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	//parameters related to surface generation
	no_of_generated_points = 10;	//number of points on a Bezier curve
	no_of_instances = 5;			//number of instances (number of curves that contain the surface)
	max_translate = 8.0f;			//for the translation surface, it's the distance between the first and the last curve
	max_rotate = glm::radians(360.0f);	//for the rotation surface, it's the angle between the first and the last curve

	//define control points
	control_p1 = glm::vec3(-4, 0.0f, -2.5);
	control_p2 = glm::vec3(-2.5, 0.0f, 1.5);
	control_p3 = glm::vec3(-1.5, 0.0f, 3.0);
	control_p4 = glm::vec3(-4.0, 0.0f, 4.0);


	// Create a bogus mesh with 2 points (a line)
	{
		vector<VertexFormat> vertices
		{
			VertexFormat(glm::vec3(-4.0, 0.0,  -2.5), glm::vec3(0, 1, 0)),
			VertexFormat(glm::vec3(-4.0, 0.0,  5.5), glm::vec3(0, 1, 0))
		};

		vector<unsigned short> indices =
		{
			0, 1
		};

		meshes["surface"] = new Mesh("generated initial surface points");
		meshes["surface"]->InitFromData(vertices, indices);
		meshes["surface"]->SetDrawMode(GL_LINES);
	}

	// Create a bogus mesh with 2 points (a line)
	{
		vector<VertexFormat> vertices
		{
			VertexFormat(glm::vec3(1, 0, 1), glm::vec3(0, 1, 0)),
			VertexFormat(glm::vec3(-1, 0, 1), glm::vec3(0, 1, 0)),
			VertexFormat(glm::vec3(-1, 0, -1), glm::vec3(0, 1, 0)),
			VertexFormat(glm::vec3(1, 0, -1), glm::vec3(0, 1, 0))

		};

		vector<unsigned short> indices =
		{
			0, 1, 2,
			0, 2, 3
		};

		meshes["quad"] = new Mesh("quad");
		meshes["quad"]->InitFromData(vertices, indices);
	}

}


void Laborator2::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void Laborator2::RenderMeshInstanced(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix, int instances, const glm::vec3 &color)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader 
	glUseProgram(shader->program);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object instanced
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0,instances);

}


void Laborator2::Update(float deltaTimeSeconds)
{
	ClearScreen();

	Shader *shader = shaders["SurfaceGeneration"];
	shader->Use();

	//send uniforms to shaders
	glUniform3f(glGetUniformLocation(shader->program, "control_p1"), control_p1.x, control_p1.y, control_p1.z);
	glUniform3f(glGetUniformLocation(shader->program, "control_p2"), control_p2.x, control_p2.y, control_p2.z);
	glUniform3f(glGetUniformLocation(shader->program, "control_p3"), control_p3.x, control_p3.y, control_p3.z);
	glUniform3f(glGetUniformLocation(shader->program, "control_p4"), control_p4.x, control_p4.y, control_p4.z);
	glUniform1i(glGetUniformLocation(shader->program, "no_of_instances"), no_of_instances);

	//TODO 
	//trimitei la shadere numarul de puncte care aproximeaza o curba (no_of_generated_points)
	//si caracteristici pentru crearea suprafetelor de translatie/rotatie (max_translate, max_rotate)
	
	Mesh* mesh = meshes["surface"];
	//draw the object instanced
	RenderMeshInstanced(mesh, shader, glm::mat4(1), no_of_instances);

	glm::vec3 offset = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 scaling = glm::vec3(0.1f);

	glm ::mat4 model = glm::mat4(1);
	model = glm::translate(model, control_p1 + offset);
	model = glm::scale(model, scaling);
	RenderMesh(meshes["quad"], shaders["Simple"], model);

	model = glm::mat4(1);
	model = glm::translate(model, control_p2 + offset);
	model = glm::scale(model, scaling);
	RenderMesh(meshes["quad"], shaders["Simple"], model);

	model = glm::mat4(1);
	model = glm::translate(model, control_p3 + offset);
	model = glm::scale(model, scaling);
	RenderMesh(meshes["quad"], shaders["Simple"], model);

	model = glm::mat4(1);
	model = glm::translate(model, control_p4 + offset);
	model = glm::scale(model, scaling);
	RenderMesh(meshes["quad"], shaders["Simple"], model);
}

void Laborator2::FrameEnd()
{
	DrawCoordinatSystem();
}

// Read the documentation of the following functions in: "Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/Framework-EGC/blob/master/Source/Core/Window/InputController.h

void Laborator2::OnInputUpdate(float deltaTime, int mods)
{
	// treat continuous update based on input
	if (window->KeyHold(GLFW_KEY_LEFT))
		control_p1 -= glm::vec3(0.05f, 0.0f, 0.0f);
	if (window->KeyHold(GLFW_KEY_RIGHT))
		control_p1 += glm::vec3(0.05f, 0.0f, 0.0f);
	if (window->KeyHold(GLFW_KEY_UP))
		control_p1 += glm::vec3(0.0f, 0.0f, 0.05f);
	if (window->KeyHold(GLFW_KEY_DOWN))
		control_p1 -= glm::vec3(0.00f, 0.0f, 0.05f);
};

void Laborator2::OnKeyPress(int key, int mods)
{
	
	//TODO 
	//modificati numarul de instante si numarul de puncte generate
};

void Laborator2::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void Laborator2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
	if (selected != nullptr)
	{
		glm::ivec2 resolution = window->GetResolution();
		glm::vec3 mousePos = glm::unProject(glm::vec3(mouseX, mouseY, 0.0f), GetSceneCamera()->GetViewMatrix(),
			GetSceneCamera()->GetProjectionMatrix(), glm::vec4(0.0f, 0.0f, resolution.x, resolution.y));
		*selected = glm::vec3(mousePos.x, 0.0f, -mousePos.z);
	}
};

void Laborator2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT))
	{
		glm::ivec2 resolution = window->GetResolution();
		mouseY = resolution.y - mouseY;
		glm::vec3 mousePosNear = glm::unProject(glm::vec3(mouseX, mouseY, 0.0f), GetSceneCamera()->GetViewMatrix(),
			GetSceneCamera()->GetProjectionMatrix(), glm::vec4(0.0f, 0.0f, resolution.x, resolution.y));
		glm::vec3 mousePosFar = glm::unProject(glm::vec3(mouseX, mouseY, 1.0f), GetSceneCamera()->GetViewMatrix(),
			GetSceneCamera()->GetProjectionMatrix(), glm::vec4(0.0f, 0.0f, resolution.x, resolution.y));

		glm::vec3 mouseDir = mousePosFar - mousePosNear;

		float t = -mousePosNear.y / mouseDir.y;

		cout << mousePosNear << endl;
		cout << mousePosFar << endl;

		glm::vec3 mousePos = mousePosNear + t * mouseDir;

		float min = 10.0f;

		if (glm::length(mousePos - control_p1) < 0.1f)
		{
			cout << "Hit first point" << endl;
			selected = &control_p1;
			min = glm::length(mousePos - control_p1);
		}

		if (glm::length(mousePos - control_p2) < 0.1f && glm::length(mousePos - control_p2) < min)
		{
			cout << "Hit second point" << endl;
			selected = &control_p2;
		}
		cout << mousePos << endl;
		cout << control_p1 << endl;
		cout << mousePos - control_p1 << endl;

	}
	// add mouse button press event
};

void Laborator2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT))
	{
		selected = nullptr;
	}
	// add mouse button release event
}

void Laborator2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void Laborator2::OnWindowResize(int width, int height)
{
	// treat window resize event
}
