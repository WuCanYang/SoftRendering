#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include "../Math/Vector2.h"
#include "../Model/Model.h"
#include "../Model/Texture.h"

// Print progress to console while loading (large models)
#define OBJL_CONSOLE_OUTPUT
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

// Namespace: Math
//
// Description: The namespace that holds all of the math
//	functions need for OBJL
namespace math
{
	// Vector3 Cross Product
	Vector3 CrossV3(const Vector3& a, const Vector3& b)
	{
		return Vector3(a.y() * b.z() - a.z() * b.y(),
			a.z() * b.x() - a.x() * b.z(),
			a.x() * b.y() - a.y() * b.x());
	}

	// Vector3 Magnitude Calculation
	float MagnitudeV3(const Vector3& in)
	{
		return (sqrtf(powf(in.x(), 2) + powf(in.y(), 2) + powf(in.z(), 2)));
	}

	// Vector3 DotProduct
	float DotV3(const Vector3 a, const Vector3 b)
	{
		return (a.x() * b.x()) + (a.y() * b.y()) + (a.z() * b.z());
	}

	// Angle between 2 Vector3 Objects
	float AngleBetweenV3(const Vector3 a, const Vector3 b)
	{
		float angle = DotV3(a, b);
		angle /= (MagnitudeV3(a) * MagnitudeV3(b));
		return angle = acosf(angle);
	}

	// Projection Calculation of a onto b
	Vector3 ProjV3(Vector3& a, Vector3& b)
	{
		Vector3 bn = b / MagnitudeV3(b);
		return bn * DotV3(a, bn);
	}
}

// Namespace: Algorithm
//
// Description: The namespace that holds all of the
// Algorithms needed for OBJL
namespace algorithm
{
	// Vector3 Multiplication Opertor Overload
	Vector3 operator*(const float& left, const Vector3& right)
	{
		return Vector3(right.x() * left, right.y() * left, right.z() * left);
	}

	// A test to see if P1 is on the same side as P2 of a line segment ab
	bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b)
	{
		Vector3 cp1 = math::CrossV3(b - a, p1 - a);
		Vector3 cp2 = math::CrossV3(b - a, p2 - a);

		if (math::DotV3(cp1, cp2) >= 0)
			return true;
		else
			return false;
	}

	// Generate a cross produect normal for a triangle
	Vector3 GenTriNormal(Vector3 t1, Vector3 t2, Vector3 t3)
	{
		Vector3 u = t2 - t1;
		Vector3 v = t3 - t1;

		Vector3 normal = math::CrossV3(u, v);

		return normal;
	}

	// Check to see if a Vector3 Point is within a 3 Vector3 Triangle
	bool inTriangle(Vector3 point, Vector3 tri1, Vector3 tri2, Vector3 tri3)
	{
		// Test to see if it is within an infinite prism that the triangle outlines.
		bool within_tri_prisim = SameSide(point, tri1, tri2, tri3) && SameSide(point, tri2, tri1, tri3)
			&& SameSide(point, tri3, tri1, tri2);

		// If it isn't it will never be on the triangle
		if (!within_tri_prisim)
			return false;

		// Calulate Triangle's Normal
		Vector3 n = GenTriNormal(tri1, tri2, tri3);

		// Project the point onto this normal
		Vector3 proj = math::ProjV3(point, n);

		// If the distance from the triangle to the point is 0
		//	it lies on the triangle
		if (math::MagnitudeV3(proj) == 0)
			return true;
		else
			return false;
	}

	// Split a String into a string array at a given token
	inline void split(const std::string& in,
		std::vector<std::string>& out,
		std::string token)
	{
		out.clear();

		std::string temp;

		for (int i = 0; i < int(in.size()); i++)
		{
			std::string test = in.substr(i, token.size());

			if (test == token)
			{
				if (!temp.empty())
				{
					out.push_back(temp);
					temp.clear();
					i += (int)token.size() - 1;
				}
				else
				{
					out.push_back("");
				}
			}
			else if (i + token.size() >= in.size())
			{
				temp += in.substr(i, token.size());
				out.push_back(temp);
				break;
			}
			else
			{
				temp += in[i];
			}
		}
	}

	// Get tail of string after first token and possibly following spaces
	inline std::string tail(const std::string& in)
	{
		size_t token_start = in.find_first_not_of(" \t");
		size_t space_start = in.find_first_of(" \t", token_start);
		size_t tail_start = in.find_first_not_of(" \t", space_start);
		size_t tail_end = in.find_last_not_of(" \t");
		if (tail_start != std::string::npos && tail_end != std::string::npos)
		{
			return in.substr(tail_start, tail_end - tail_start + 1);
		}
		else if (tail_start != std::string::npos)
		{
			return in.substr(tail_start);
		}
		return "";
	}

	// Get first token of string
	inline std::string firstToken(const std::string& in)
	{
		if (!in.empty())
		{
			size_t token_start = in.find_first_not_of(" \t");
			size_t token_end = in.find_first_of(" \t", token_start);
			if (token_start != std::string::npos && token_end != std::string::npos)
			{
				return in.substr(token_start, token_end - token_start);
			}
			else if (token_start != std::string::npos)
			{
				return in.substr(token_start);
			}
		}
		return "";
	}

	// Get element at given index position
	template <class T>
	inline const T& getElement(const std::vector<T>& elements, std::string& index)
	{
		int idx = std::stoi(index);
		if (idx < 0)
			idx = int(elements.size()) + idx;
		else
			idx--;
		return elements[idx];
	}
}

// Class: Loader
//
// Description: The OBJ Model Loader
class Loader
{
public:
	// Default Constructor
	Loader()
	{

	}
	~Loader()
	{

	}

	// Load a file into the loader
	//
	// If file is loaded return true
	//
	// If the file is unable to be found
	// or unable to be loaded return false
	static Model* LoadFile(std::string Path)
	{
		// If the file is not an .obj file return false
		if (Path.substr(Path.size() - 4, 4) != ".obj")
			return nullptr;


		std::ifstream file(Path);

		if (!file.is_open())
			return nullptr;

		Model* model = new Model;

		std::string curline;
		while (std::getline(file, curline))
		{
			// Generate a Vertex Position
			if (algorithm::firstToken(curline) == "v")
			{
				std::vector<std::string> spos;
				algorithm::split(algorithm::tail(curline), spos, " ");

				model->Vertices.push_back(Vector3(std::stof(spos[0]), std::stof(spos[1]), std::stof(spos[2])));
			}
			// Generate a Vertex Texture Coordinate
			if (algorithm::firstToken(curline) == "vt")
			{
				std::vector<std::string> stex;
				algorithm::split(algorithm::tail(curline), stex, " ");

				model->TexCoords.push_back(Vector2(std::stof(stex[0]), std::stof(stex[1])));
			}
			// Generate a Vertex Normal;
			if (algorithm::firstToken(curline) == "vn")
			{
				std::vector<std::string> snor;
				algorithm::split(algorithm::tail(curline), snor, " ");

				model->Normals.push_back(Vector3(std::stof(snor[0]), std::stof(snor[1]), std::stof(snor[2])));
			}
			// Generate a Face (vertices & indices)
			if (algorithm::firstToken(curline) == "f")
			{
				std::vector<std::string> fnor;
				algorithm::split(algorithm::tail(curline), fnor, " ");
				model->VerticesIndices.push_back(Index3I());
				model->TexCoordsIndices.push_back(Index3I());
				model->NormalsIndices.push_back(Index3I());

				for (int i = 0; i < fnor.size(); ++i)
				{
					std::vector<std::string> tmp;
					algorithm::split(fnor[i], tmp, "/");	//  P / T / N

					model->VerticesIndices.back().index[i] = stoi(tmp[0]) - 1;
					model->TexCoordsIndices.back().index[i] = stoi(tmp[1]) - 1;
					model->NormalsIndices.back().index[i] = stoi(tmp[2]) - 1;
				}
			}
		}

		std::cout << "Vertices Nums:  " << model->Vertices.size() << std::endl;
		std::cout << "Faces Nums:   " << model->VerticesIndices.size() << std::endl;


		stbi_set_flip_vertically_on_load(true);

		std::string texName = Path.substr(0, Path.length() - 4) + ".png";
		int width, heght, channel;
		unsigned char* data = stbi_load(texName.c_str(), &width, &heght, &channel, 0);
		if (data)
		{
			model->modelTexture = new Texture(data, width, heght, channel);
			std::cout << "Texture Channel:  " << channel << std::endl;
		}
		else
		{
			std::cout << "load texture failed" << std::endl;
		}


		file.close();
		return model;
	}

private:

	

	// Generate vertices from a list of positions,
	//	tcoords, normals and a face line
/*	static void GenVerticesFromRawOBJ(std::vector<Vertex>& oVerts,
		const std::vector<Vector3>& iPositions,
		const std::vector<Vector2>& iTCoords,
		const std::vector<Vector3>& iNormals,
		std::string icurline)
	{
		std::vector<std::string> sface, svert;
		Vertex vVert;
		algorithm::split(algorithm::tail(icurline), sface, " ");

		bool noNormal = false;

		// For every given vertex do this
		for (int i = 0; i < int(sface.size()); i++)
		{
			// See What type the vertex is.
			int vtype;

			algorithm::split(sface[i], svert, "/");

			// Check for just position - v1
			if (svert.size() == 1)
			{
				// Only position
				vtype = 1;
			}

			// Check for position & texture - v1/vt1
			if (svert.size() == 2)
			{
				// Position & Texture
				vtype = 2;
			}

			// Check for Position, Texture and Normal - v1/vt1/vn1
			// or if Position and Normal - v1//vn1
			if (svert.size() == 3)
			{
				if (svert[1] != "")
				{
					// Position, Texture, and Normal
					vtype = 4;
				}
				else
				{
					// Position & Normal
					vtype = 3;
				}
			}

			// Calculate and store the vertex
			switch (vtype)
			{
			case 1: // P
			{
				vVert.Position = algorithm::getElement(iPositions, svert[0]);
				vVert.TextureCoordinate = Vector2(0, 0);
				noNormal = true;
				oVerts.push_back(vVert);
				break;
			}
			case 2: // P/T
			{
				vVert.Position = algorithm::getElement(iPositions, svert[0]);
				vVert.TextureCoordinate = algorithm::getElement(iTCoords, svert[1]);
				noNormal = true;
				oVerts.push_back(vVert);
				break;
			}
			case 3: // P//N
			{
				vVert.Position = algorithm::getElement(iPositions, svert[0]);
				vVert.TextureCoordinate = Vector2(0, 0);
				vVert.Normal = algorithm::getElement(iNormals, svert[2]);
				oVerts.push_back(vVert);
				break;
			}
			case 4: // P/T/N
			{
				vVert.Position = algorithm::getElement(iPositions, svert[0]);
				vVert.TextureCoordinate = algorithm::getElement(iTCoords, svert[1]);
				vVert.Normal = algorithm::getElement(iNormals, svert[2]);
				oVerts.push_back(vVert);
				break;
			}
			default:
			{
				break;
			}
			}
		}

		// take care of missing normals
		// these may not be truly acurate but it is the
		// best they get for not compiling a mesh with normals
		if (noNormal)
		{
			Vector3 A = oVerts[0].Position - oVerts[1].Position;
			Vector3 B = oVerts[2].Position - oVerts[1].Position;

			Vector3 normal = math::CrossV3(A, B);

			for (int i = 0; i < int(oVerts.size()); i++)
			{
				oVerts[i].Normal = normal;
			}
		}
	}

	// Triangulate a list of vertices into a face by printing
	//	inducies corresponding with triangles within it
	void VertexTriangluation(std::vector<unsigned int>& oIndices,
		const std::vector<Vertex>& iVerts)
	{
		// If there are 2 or less verts,
		// no triangle can be created,
		// so exit
		if (iVerts.size() < 3)
		{
			return;
		}
		// If it is a triangle no need to calculate it
		if (iVerts.size() == 3)
		{
			oIndices.push_back(0);
			oIndices.push_back(1);
			oIndices.push_back(2);
			return;
		}

		// Create a list of vertices
		std::vector<Vertex> tVerts = iVerts;

		while (true)
		{
			// For every vertex
			for (int i = 0; i < int(tVerts.size()); i++)
			{
				// pPrev = the previous vertex in the list
				Vertex pPrev;
				if (i == 0)
				{
					pPrev = tVerts[tVerts.size() - 1];
				}
				else
				{
					pPrev = tVerts[i - 1];
				}

				// pCur = the current vertex;
				Vertex pCur = tVerts[i];

				// pNext = the next vertex in the list
				Vertex pNext;
				if (i == tVerts.size() - 1)
				{
					pNext = tVerts[0];
				}
				else
				{
					pNext = tVerts[i + 1];
				}

				// Check to see if there are only 3 verts left
				// if so this is the last triangle
				if (tVerts.size() == 3)
				{
					// Create a triangle from pCur, pPrev, pNext
					for (int j = 0; j < int(tVerts.size()); j++)
					{
						if (iVerts[j].Position == pCur.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pPrev.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pNext.Position)
							oIndices.push_back(j);
					}

					tVerts.clear();
					break;
				}
				if (tVerts.size() == 4)
				{
					// Create a triangle from pCur, pPrev, pNext
					for (int j = 0; j < int(iVerts.size()); j++)
					{
						if (iVerts[j].Position == pCur.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pPrev.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pNext.Position)
							oIndices.push_back(j);
					}

					Vector3 tempVec;
					for (int j = 0; j < int(tVerts.size()); j++)
					{
						if (tVerts[j].Position != pCur.Position
							&& tVerts[j].Position != pPrev.Position
							&& tVerts[j].Position != pNext.Position)
						{
							tempVec = tVerts[j].Position;
							break;
						}
					}

					// Create a triangle from pCur, pPrev, pNext
					for (int j = 0; j < int(iVerts.size()); j++)
					{
						if (iVerts[j].Position == pPrev.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pNext.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == tempVec)
							oIndices.push_back(j);
					}

					tVerts.clear();
					break;
				}

				// If Vertex is not an interior vertex
				float angle = math::AngleBetweenV3(pPrev.Position - pCur.Position, pNext.Position - pCur.Position) * (180 / 3.14159265359);
				if (angle <= 0 && angle >= 180)
					continue;

				// If any vertices are within this triangle
				bool inTri = false;
				for (int j = 0; j < int(iVerts.size()); j++)
				{
					if (algorithm::inTriangle(iVerts[j].Position, pPrev.Position, pCur.Position, pNext.Position)
						&& iVerts[j].Position != pPrev.Position
						&& iVerts[j].Position != pCur.Position
						&& iVerts[j].Position != pNext.Position)
					{
						inTri = true;
						break;
					}
				}
				if (inTri)
					continue;

				// Create a triangle from pCur, pPrev, pNext
				for (int j = 0; j < int(iVerts.size()); j++)
				{
					if (iVerts[j].Position == pCur.Position)
						oIndices.push_back(j);
					if (iVerts[j].Position == pPrev.Position)
						oIndices.push_back(j);
					if (iVerts[j].Position == pNext.Position)
						oIndices.push_back(j);
				}

				// Delete pCur from the list
				for (int j = 0; j < int(tVerts.size()); j++)
				{
					if (tVerts[j].Position == pCur.Position)
					{
						tVerts.erase(tVerts.begin() + j);
						break;
					}
				}

				// reset i to the start
				// -1 since loop will add 1 to it
				i = -1;
			}

			// if no triangles were created
			if (oIndices.size() == 0)
				break;

			// if no more vertices
			if (tVerts.size() == 0)
				break;
		}
	}

	*/
};
