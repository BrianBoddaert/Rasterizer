#include "pch.h"
#include "ObjParser.h"
#include <fstream>
#include <iostream>

ObjParser* ObjParser::m_pInstance{ nullptr };

ObjParser* ObjParser::GetInstance()
{
	if (!m_pInstance)
	{
		m_pInstance = new ObjParser();
	}

	return m_pInstance;
}

void ObjParser::Parse(const std::string& path, std::vector<SoftwareRasterizer_Vertex_Input>& softwareRasterizerVertices, std::vector<DirectX_Vertex_Input>& directXVertices, std::vector<int>& indexes)
{
	std::ifstream input;

	std::string line;

	input.open(path, std::ios::in | std::ios::binary);

	std::vector < Elite::FVector3 > normals;
	std::vector < Elite::FPoint2 > uvs;
	std::vector < Elite::FPoint4 > positionsSoftwareRasterizer;
	std::vector < Elite::FPoint3 > positionsDirectX;
	if (input.is_open())
	{
		while (std::getline(input, line, '\r'))
		{
			if ((line.find('v') != std::string::npos || line.find('f') != std::string::npos) && line.find('#') == std::string::npos)
			{
				for (size_t i = 0; i < line.size(); i++)
				{
					if (line[i] == 'v' && line[i + 1] == ' ')
					{

						std::string lineOnlyNumbers = line;
						lineOnlyNumbers.erase(lineOnlyNumbers.begin(), lineOnlyNumbers.begin() + i + 3);

						std::string FirstNumber = lineOnlyNumbers;
						size_t NextSpaceIndex = FirstNumber.find_first_of(' ');
						FirstNumber.erase(FirstNumber.begin() + NextSpaceIndex, FirstNumber.end());

						std::string SecondNumber = lineOnlyNumbers;
						NextSpaceIndex = SecondNumber.find_first_of(' ');
						SecondNumber.erase(SecondNumber.begin(), SecondNumber.begin() + NextSpaceIndex + 1);
						NextSpaceIndex = SecondNumber.find_first_of(' ');
						SecondNumber.erase(SecondNumber.begin() + NextSpaceIndex, SecondNumber.end());

						std::string ThirdNumber = lineOnlyNumbers;
						NextSpaceIndex = ThirdNumber.find_first_of(' ');
						ThirdNumber.erase(ThirdNumber.begin(), ThirdNumber.begin() + NextSpaceIndex + 1);
						NextSpaceIndex = ThirdNumber.find_first_of(' ');
						ThirdNumber.erase(ThirdNumber.begin(), ThirdNumber.begin() + NextSpaceIndex + 1);

						Elite::FPoint4 newPosition{};
						newPosition = { std::stof(FirstNumber), std::stof(SecondNumber), std::stof(ThirdNumber),0 };
						positionsSoftwareRasterizer.push_back(newPosition);


					}
					else if (line[i] == 'v' && line[i + 1] == 'n' && line[i + 2] == ' ')
					{

						std::string lineOnlyNumbers = line;
						lineOnlyNumbers.erase(lineOnlyNumbers.begin(), lineOnlyNumbers.begin() + i + 3);

						std::string FirstNumber = lineOnlyNumbers;
						size_t NextSpaceIndex = FirstNumber.find_first_of(' ');
						FirstNumber.erase(FirstNumber.begin() + NextSpaceIndex, FirstNumber.end());

						std::string SecondNumber = lineOnlyNumbers;
						NextSpaceIndex = SecondNumber.find_first_of(' ');
						SecondNumber.erase(SecondNumber.begin(), SecondNumber.begin() + NextSpaceIndex + 1);
						NextSpaceIndex = SecondNumber.find_first_of(' ');
						SecondNumber.erase(SecondNumber.begin() + NextSpaceIndex, SecondNumber.end());

						std::string ThirdNumber = lineOnlyNumbers;
						NextSpaceIndex = ThirdNumber.find_first_of(' ');
						ThirdNumber.erase(ThirdNumber.begin(), ThirdNumber.begin() + NextSpaceIndex + 1);
						NextSpaceIndex = ThirdNumber.find_first_of(' ');
						ThirdNumber.erase(ThirdNumber.begin(), ThirdNumber.begin() + NextSpaceIndex + 1);

						Elite::FVector3 newNormal = { std::stof(FirstNumber), std::stof(SecondNumber), std::stof(ThirdNumber) };
						normals.push_back(newNormal);

					}
					else if (line[i] == 'v' && line[i + 1] == 't' && line[i + 2] == ' ') // UV
					{

						std::string lineOnlyNumbers = line;
						lineOnlyNumbers.erase(lineOnlyNumbers.begin(), lineOnlyNumbers.begin() + i + 3);

						std::string FirstNumber = lineOnlyNumbers;
						size_t NextSpaceIndex = FirstNumber.find_first_of(' ');
						FirstNumber.erase(FirstNumber.begin() + NextSpaceIndex, FirstNumber.end());

						std::string SecondNumber = lineOnlyNumbers;
						NextSpaceIndex = SecondNumber.find_first_of(' ');
						SecondNumber.erase(SecondNumber.begin(), SecondNumber.begin() + NextSpaceIndex + 1);
						NextSpaceIndex = SecondNumber.find_first_of(' ');
						SecondNumber.erase(SecondNumber.begin() + NextSpaceIndex, SecondNumber.end());

						Elite::FPoint2 newUv = { std::stof(FirstNumber), std::stof(SecondNumber) };
						uvs.push_back(newUv);

					}
					else if (line[i] == 'f' && line[i + 1] == ' ')
					{

						std::string lineOnlyNumbers = line;
						lineOnlyNumbers.erase(lineOnlyNumbers.begin(), lineOnlyNumbers.begin() + 3);

						for (size_t i = 0; i < 3; i++)
						{

							std::string FirstNumber = lineOnlyNumbers;
							size_t NextSpaceIndex = FirstNumber.find_first_of('/');
							FirstNumber.erase(FirstNumber.begin() + NextSpaceIndex, FirstNumber.end());

							std::string SecondNumber = lineOnlyNumbers;
							NextSpaceIndex = SecondNumber.find_first_of('/');
							SecondNumber.erase(SecondNumber.begin(), SecondNumber.begin() + NextSpaceIndex + 1);
							NextSpaceIndex = SecondNumber.find_first_of('/');
							SecondNumber.erase(SecondNumber.begin() + NextSpaceIndex, SecondNumber.end());

							std::string ThirdNumber = lineOnlyNumbers;
							NextSpaceIndex = ThirdNumber.find_first_of('/');
							ThirdNumber.erase(ThirdNumber.begin(), ThirdNumber.begin() + NextSpaceIndex + 1);
							NextSpaceIndex = ThirdNumber.find_first_of('/');
							ThirdNumber.erase(ThirdNumber.begin(), ThirdNumber.begin() + NextSpaceIndex + 1);
							NextSpaceIndex = ThirdNumber.find_first_of(' ');
							ThirdNumber.erase(ThirdNumber.begin() + NextSpaceIndex, ThirdNumber.end());

							//vertices[std::stoi(FirstNumber)].uv =
							SoftwareRasterizer_Vertex_Input newVertexSWR = SoftwareRasterizer_Vertex_Input();
							DirectX_Vertex_Input newVertexDX = DirectX_Vertex_Input();

							int index = std::stoi(FirstNumber, nullptr, 0) - 1;
							newVertexSWR.position = positionsSoftwareRasterizer[index];

							index = std::stoi(SecondNumber, nullptr, 0) - 1;
							newVertexSWR.uv = uvs[index];

							index = std::stoi(ThirdNumber, nullptr, 0) - 1;
							newVertexSWR.normal = normals[index];
							newVertexSWR.color = Elite::RGBColor{ 0,0,0 };

							softwareRasterizerVertices.push_back(newVertexSWR);

							index = std::stoi(FirstNumber, nullptr, 0) - 1;
							newVertexDX.position = positionsSoftwareRasterizer[index].xyz;
							newVertexDX.position.z = -newVertexDX.position.z;

							index = std::stoi(SecondNumber, nullptr, 0) - 1;
							newVertexDX.uv = uvs[index];
							newVertexDX.uv.y = 1 - newVertexDX.uv.y;

							index = std::stoi(ThirdNumber, nullptr, 0) - 1;
							newVertexDX.normal = normals[index];
							newVertexDX.normal.z = -newVertexDX.normal.z;

							newVertexDX.color = Elite::RGBColor{ 0,0,0 };

							directXVertices.push_back(newVertexDX);

							indexes.push_back((int)indexes.size());

							NextSpaceIndex = lineOnlyNumbers.find_first_of(' ');
							lineOnlyNumbers.erase(lineOnlyNumbers.begin(), lineOnlyNumbers.begin() + NextSpaceIndex + 1);
						}

					}
				}

			}


		}
		input.close();
	}
	for (uint32_t i = 0; i < indexes.size(); i += 3) // TANGENT MAP
	{
		uint32_t index0 = indexes[i];
		int index = i + 1;
		uint32_t index1 = indexes[index];
		index = i + 2;
		uint32_t index2 = indexes[index];
		const Elite::FPoint3& p0 = Elite::FPoint3(softwareRasterizerVertices[index0].position);
		const Elite::FPoint3& p1 = Elite::FPoint3(softwareRasterizerVertices[index1].position);
		const Elite::FPoint3& p2 = Elite::FPoint3(softwareRasterizerVertices[index2].position);
		const Elite::FPoint2& uv0 = softwareRasterizerVertices[index0].uv;
		const Elite::FPoint2& uv1 = softwareRasterizerVertices[index1].uv;
		const Elite::FPoint2& uv2 = softwareRasterizerVertices[index2].uv;
		const Elite::FVector3 edge0 = p1 - p0;
		const Elite::FVector3 edge1 = p2 - p0;
		const Elite::FVector2 diffX = Elite::FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
		const Elite::FVector2 diffY = Elite::FVector2(uv1.y - uv0.y, uv2.y - uv0.y);
		float r = 1.f / Cross(diffX, diffY);
		Elite::FVector3 tangent = (edge0 * diffY.y - edge1 * diffX.x) * r;

		softwareRasterizerVertices[index0].tangent += tangent;
		softwareRasterizerVertices[index1].tangent += tangent;
		softwareRasterizerVertices[index2].tangent += tangent;

		tangent.z = -tangent.z;

		directXVertices[index0].tangent += tangent;
		directXVertices[index1].tangent += tangent;
		directXVertices[index2].tangent += tangent;
		//Create the tangents (reject vector) + fix the tangents per vertex
	}

	for (auto& v : softwareRasterizerVertices)
		v.tangent = GetNormalized(Reject(v.tangent, v.normal));
}

