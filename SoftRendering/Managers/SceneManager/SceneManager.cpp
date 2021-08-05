#include "SceneManager.h"
#include "Managers/RenderManager/RenderManager.h"
#include "Model/Model.h"
#include "Model/Light.h"
#include "Model/OBJ_Loader.h"
#include "Model/Constant.h"
#include "MeshSimplifier/SkinnedMesh.h"
#include "MeshSimplifier/Simplifier.h"

SceneManager::SceneManager(): renderManager(nullptr), camera(nullptr), light(nullptr)
{
	std::cout << "Scene loading" << std::endl;

	loadModel("../Models/spot.obj");
	loadLight();
	loadCamera();

	if (SHADOW && !GENERAL && !WIREFRAME)
	{
		loadPlane();
	}
	//loadPlane();
}

SceneManager::~SceneManager()
{
	for (Model* model : models)
	{
		delete model;
	}
	delete light;
	delete camera;
}

void SceneManager::loadPlane()
{
	Model* m = new Model;
	//m->Position = Vector3(0.0f, -1.3f, 0.0f);
	m->Position = Vector3(0.0f, -0.4f, 0.0f);
	m->Scale = Vector3(0.3f);

	float planeVertices[] = {
		// positions            // normals         // texcoords
		 5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		-5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,

		 5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		 5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
		-5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f
	};

	/*float planeVertices[] = {
		// positions            // normals         // texcoords
		 5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		-5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		 0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,

		-5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		 0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,

		 0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,
		 5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,

		 5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		 0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		 5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
	};*/

	for (int i = 0; i < 6; ++i)
	{
		int index = i * 8;
		m->Vertices.push_back(Vector3(planeVertices[index], planeVertices[index + 1], planeVertices[index + 2]));
		m->Normals.push_back(Vector3(planeVertices[index + 3], planeVertices[index + 4], planeVertices[index + 5]));
		m->TexCoords.push_back(Vector2(planeVertices[index + 6], planeVertices[index + 7]));

		if ((i + 1) % 3 == 0)
		{
			m->VerticesIndices.push_back(Index3I(i - 2, i - 1, i));
			m->TexCoordsIndices.push_back(Index3I(i - 2, i - 1, i));
			m->NormalsIndices.push_back(Index3I(i - 2, i - 1, i));
		}
	}
	models.push_back(m);
}

void SceneManager::loadModel(std::string name)
{
	Model* m = Loader::LoadFile(name);
	if (!m) return;

	if (EnableMeshSimplify)
	{
		double start = clock();
		SimplifyMesh(m);
		double end = clock();
		std::cout << "Simplify Cost time: " << (end - start) / CLOCKS_PER_SEC << "s" << std::endl;
	}

	models.push_back(m);
}

void SceneManager::loadLight()
{
	light = new Light;
	//light->Position = Vector3(1.0f, 3.0f, -1.0f);
	light->Position = Vector3(-1.0f, 2.0f, -2.0f);
	if (!models.empty())
	{
		light->Direction = light->Position - Vector3();//models[0]->Position;
	}

	Model* m = new Model;		//为光源添加顶点数据等，用于光源可视化
	m->Position = light->Position;
	m->Scale = Vector3(0.1f);
	light->mesh = m;

	float vertices[] = {		//cube数据  P/N/T
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};

	for (int i = 0; i < 36; ++i)
	{
		int index = i * 8;
		m->Vertices.push_back(Vector3(vertices[index], vertices[index+1], vertices[index + 2]));
		m->Normals.push_back(Vector3(vertices[index + 3], vertices[index + 4], vertices[index + 5]));
		m->TexCoords.push_back(Vector2(vertices[index + 6], vertices[index + 7]));

		if ((i + 1) % 3 == 0)
		{
			m->VerticesIndices.push_back(Index3I(i - 2, i - 1, i));
			m->TexCoordsIndices.push_back(Index3I(i - 2, i - 1, i));
			m->NormalsIndices.push_back(Index3I(i - 2, i - 1, i));
		}
	}
}

void SceneManager::loadCamera()
{
	camera = new Camera;
	//camera->Position = Vector3(0.0f, 4.0f, 2.0f);
	camera->Position = Vector3(0.0f, 0.5f, 1.3f);
	if (!models.empty())
	{
		//camera->Target = Vector3(0.0f, 0.0f, -1.0f);//Vector3(0.0f); //models[0]->Position;
		camera->Target = Vector3(0.0f); //models[0]->Position;
	}
}

void SceneManager::setRenderManager(RenderManager* rm)
{
	renderManager = rm;
}

void SceneManager::frame()
{
	for (Model* model : models)
	{
		model->update();
	}
	light->update();
}

std::vector<Model*>* SceneManager::GetSceneModels()
{
	return &models;
}

Light* SceneManager::GetLight()
{
	return light;
}

Camera* SceneManager::GetCamera()
{
	return camera;
}

void SceneManager::ComputeUVBounds(SkinnedMesh& Mesh, std::vector<Vector2>& UVBounds)
{
	const int NumValidUVs = Mesh.TexCoordCount();
	for (int i = 0; i < NumValidUVs; ++i)
	{
		UVBounds[2 * i] = Vector2(FLT_MAX, FLT_MAX);
		UVBounds[2 * i + 1] = Vector2(-FLT_MAX, -FLT_MAX);
	}

	for (int i = 0; i < Mesh.NumVertices(); ++i)
	{
		const auto& Attrs = Mesh.VertexBuffer[i].BasicAttributes;
		for (int t = 0; t < NumValidUVs; ++t)
		{
			UVBounds[2 * t]._x = std::min(Attrs.TexCoords[t]._x, UVBounds[2 * t]._x);
			UVBounds[2 * t]._y = std::min(Attrs.TexCoords[t]._y, UVBounds[2 * t]._y);

			UVBounds[2 * t + 1]._x = std::max(Attrs.TexCoords[t]._x, UVBounds[2 * t + 1]._x);
			UVBounds[2 * t + 1]._y = std::max(Attrs.TexCoords[t]._y, UVBounds[2 * t + 1]._y);
		}
	}
}

void SceneManager::SimplifyMesh(Model* m)
{
	SkinnedMesh Mesh;
	ConvertToSkinnedMesh(m, Mesh);
	int RemainVerts			= RemainPercent * m->Vertices.size();
	int RemainTris			= RemainPercent * m->VerticesIndices.size();
	float MaxDistance		= FLT_MAX;
	float MaxFeatureCost	= FLT_MAX;
	std::cout << "remain size   " << RemainVerts << "    " << RemainTris << std::endl;
	SimplifierTerminator Terminator(RemainTris, RemainVerts, MaxFeatureCost, MaxDistance);

	double NormalWeight = 1.0;
	double TangentWeight = 0.10;
	double BiTangentWeight = 0.10;
	double UVWeight = 0.50;
	double BoneWeight = 0.25;
	double ColorWeight = 0.10;

	const int NumUVs = MAX_TEXCOORDS;
	std::vector<Vector2> UVBounds(2 * NumUVs);
	ComputeUVBounds(Mesh, UVBounds);

	MeshSimplifier::DenseVecDType	BasicAttrWeights;
	{

		// Normal
		BasicAttrWeights[0] = NormalWeight;
		BasicAttrWeights[1] = NormalWeight;
		BasicAttrWeights[2] = NormalWeight;

		//Tangent
		BasicAttrWeights[3] = TangentWeight;
		BasicAttrWeights[4] = TangentWeight;
		BasicAttrWeights[5] = TangentWeight;
		//BiTangent
		BasicAttrWeights[6] = BiTangentWeight;
		BasicAttrWeights[7] = BiTangentWeight;
		BasicAttrWeights[8] = BiTangentWeight;

		//Color
		BasicAttrWeights[9] = ColorWeight; // r
		BasicAttrWeights[10] = ColorWeight; // b
		BasicAttrWeights[11] = ColorWeight; // b
		BasicAttrWeights[12] = ColorWeight; // alpha


		const int NumNonUVAttrs = 13;

		// Number of UVs actually used.
		const int NumValidUVs = Mesh.TexCoordCount();
		for (int i = 0; i < NumValidUVs; ++i)
		{
			Vector2& UVMin = UVBounds[2 * i];
			Vector2& UVMax = UVBounds[2 * i + 1];

			double URange = UVMax._x - UVMin._x;
			double VRange = UVMax._y - UVMin._y;

			double UWeight = (abs(URange) > 1.e-5) ? UVWeight / URange : 0.;
			double VWeight = (abs(VRange) > 1.e-5) ? UVWeight / VRange : 0.;

			BasicAttrWeights[NumNonUVAttrs + 2 * i] = UWeight; // U
			BasicAttrWeights[NumNonUVAttrs + 2 * i + 1] = VWeight; // V
		}

		for (int i = NumNonUVAttrs; i < NumNonUVAttrs + NumValidUVs * 2; ++i)
		{
			BasicAttrWeights[i] = UVWeight; // 0.5;
		}

		for (int i = NumNonUVAttrs + NumValidUVs * 2; i < NumNonUVAttrs + NumUVs * 2; ++i)
		{
			BasicAttrWeights[i] = 0.;
		}

	}



	MeshSimplifier  Simplifier(Mesh.VertexBuffer, (unsigned int)Mesh.NumVertices(),
							   Mesh.IndexBuffer, (unsigned int)Mesh.NumIndices(),
							   VolumeImportance__, PreserveVolume__, CheckBoneBoundaries__);

	Mesh.Empty();

	{
		Simplifier.SetBoundaryConstraintWeight(EdgeWeightValue__);

		Simplifier.SetAttributeWeights(BasicAttrWeights);

		if (LockBoundary__)
		{
			Simplifier.SetBoundaryLocked();
		}

		if (LockDifferentColorEdge)
		{
			Simplifier.SetColorEdgeLocked();
		}
	}

	Simplifier.SimplifyMesh(Terminator);

	Mesh.Resize(Simplifier.GetNumTris(), Simplifier.GetNumVerts());

	Simplifier.OutputMesh(Mesh.VertexBuffer, Mesh.IndexBuffer);

	Mesh.Compact();

	ConvertToModelMesh(Mesh, m);
}


void SceneManager::ConvertToSkinnedMesh(Model* m, SkinnedMesh& outMesh)
{
	int NumTris = m->VerticesIndices.size();
	int NumVerts = m->Vertices.size();

	outMesh.Resize(NumTris, NumVerts);
	outMesh.SetTexCoordCount(1);

	unsigned int* outIndexBuffer = outMesh.IndexBuffer;
	MeshVertType* outVertexBuffer = outMesh.VertexBuffer;


	std::vector<Vector3>& Vertices = m->Vertices;
	std::vector<Vector2>& TexCoords = m->TexCoords;
	std::vector<Vector3>& Normals = m->Normals;
	for (int i = 0; i < NumTris; ++i)
	{
		Index3I& VertexIndex = m->VerticesIndices[i];
		Index3I& TexCoordIndex = m->TexCoordsIndices[i];
		Index3I& NormalIndex = m->NormalsIndices[i];

		for (int j = 0; j < 3; ++j)
		{
			int index = VertexIndex.index[j];
			
			outVertexBuffer[index].Position = Vertices[index];
			outVertexBuffer[index].MasterVertIndex = index;
			outVertexBuffer[index].MaterialIndex = 0;

			auto& BasicAttr = outVertexBuffer[index].BasicAttributes;
			BasicAttr.Normal = Normals[NormalIndex.index[j]];

			Vector2 TexC = TexCoords[TexCoordIndex.index[j]];
			BasicAttr.Color = m->modelTexture->texture2D(TexC._x, TexC._y);
			BasicAttr.TexCoords[0] = TexC;


			outIndexBuffer[3 * i + j] = index;
		}
	}
	outMesh.Compact();
}

void SceneManager::ConvertToModelMesh(SkinnedMesh& inMesh, Model* m)
{
	unsigned int* inIndexBuffer = inMesh.IndexBuffer;
	MeshVertType* inVertexBuffer = inMesh.VertexBuffer;

	std::vector<Vector3>& Vertices = m->Vertices;
	std::vector<Vector2>& TexCoords = m->TexCoords;
	std::vector<Vector3>& Normals = m->Normals;

	std::vector<Index3I>& VerticesIndices = m->VerticesIndices; 
	std::vector<Index3I>& TexCoordsIndices = m->TexCoordsIndices;
	std::vector<Index3I>& NormalsIndices = m->NormalsIndices;

	Vertices.clear();
	TexCoords.clear();
	Normals.clear();
	VerticesIndices.clear();
	TexCoordsIndices.clear();
	NormalsIndices.clear();

	int TrisNum = inMesh.NumTris();
	int VertsNum = inMesh.NumVertices();
	Vertices.resize(VertsNum);
	TexCoords.resize(VertsNum);
	Normals.resize(VertsNum);

	VerticesIndices.resize(TrisNum);
	TexCoordsIndices.resize(TrisNum);
	NormalsIndices.resize(TrisNum);

	for (int i = 0; i < TrisNum; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			int index = inIndexBuffer[3 * i + j];
			Vertices[index] = inVertexBuffer[index].Position;

			auto& BasicAttr = inVertexBuffer[index].BasicAttributes;
			Normals[index] = BasicAttr.Normal;
			TexCoords[index] = BasicAttr.TexCoords[0];

			VerticesIndices[i].index[j] = index;
			TexCoordsIndices[i].index[j] = index;
			NormalsIndices[i].index[j] = index;
		}
	}
	std::cout << "After simplify, the tris Num:   " << TrisNum << std::endl;
	std::cout << "After simplify, the vert Num:   " << VertsNum << std::endl;
}


