#include "RenderManager.h"
#include "../SceneManager/SceneManager.h"
#include "../DisplayManager/DisplayManager.h"

void RenderManager::setSceneManager(SceneManager* sm)
{
	sceneManager = sm;
}

void RenderManager::setDisplayManager(DisplayManager* dm)
{
	displayManager = dm;
}

void RenderManager::loadSceneResources()
{
	models = sceneManager->GetSceneModels();
	light = sceneManager->GetLight();
	camera = sceneManager->GetCamera();
}

void RenderManager::frame()
{
	loadSceneResources();
	Matrix4X4 view = camera->GetViewMatrix();
	Matrix4X4 projection = camera->GetPerspectiveMatrix();

	for (Model* model : *models)
	{
		std::vector<Vector3>& Vertices = model->Vertices;
		std::vector<Vector2>& TexCoords = model->TexCoords;
		std::vector<Vector3>& Normals = model->Normals;

		std::vector<Index3I>& VerticesIndices = model->VerticesIndices;
		std::vector<Index3I>& TexCoordsIndices = model->TexCoordsIndices;
		std::vector<Index3I>& NormalsIndices = model->NormalsIndices;

		Matrix4X4 modelMatrix = model->GetModelMatrix();

		std::vector<Vector3> WorldVertices;  // 顶点着色器输出
		std::vector<Vector3> WorldNormals;
		std::vector<Vector3> ClipVertices;

		shader.VertexShader(modelMatrix, view, projection, Vertices, Normals, WorldVertices, WorldNormals, ClipVertices);

	}

}


