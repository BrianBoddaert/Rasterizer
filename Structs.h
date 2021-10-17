#pragma once
#include "EMath.h"
#include "ERGBColor.h"

struct SoftwareRasterizer_Vertex_Input
{
	Elite::FPoint4 position{};
	Elite::RGBColor color{};
	Elite::FPoint2 uv{};
	Elite::FVector3 normal{};
	Elite::FVector3 tangent{};
};

struct DirectX_Vertex_Input
{
	Elite::FPoint3 position{};
	Elite::RGBColor color{};
	Elite::FPoint2 uv{};
	Elite::FVector3 normal{};
	Elite::FVector3 tangent{};
};

struct Vertex_Output
{
	Elite::FPoint4 position{};
	Elite::FVector3 viewDirection{};
	Elite::RGBColor color{};
	Elite::FPoint2 uv{};
	Elite::FVector3 normal{};
	Elite::FVector3 tangent{};
	bool isValid = true;
};

struct InterpolatedData
{
	Elite::FVector3 normal = {};
	Elite::RGBColor color = {};
	Elite::FVector2 uv = {};
	Elite::FVector3 viewDirection = {};
};