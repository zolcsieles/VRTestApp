#pragma once

#include "CommonRenderer.h"

template<RENDERER xRenderer>
struct TexturedBox
{
	typename MyTypes<xRenderer>::Model* model;
	typename MyTypes<xRenderer>::VertexBuffer* vertexBuffer;
	typename MyTypes<xRenderer>::IndexBuffer* indexBuffer;

	TexturedBox() : model(nullptr), vertexBuffer(nullptr), indexBuffer(nullptr)
	{
	}

	static FormatDesc<float> position;
	static FormatDesc<float> uv;
	static Layout layout;

	struct VertexFormat
	{
		zls::math::vec3 pos;
		zls::math::vec2 uv;
	};

	static const int nVertices = 8;
	static VertexFormat vertices[];

	static const int nIndices = 6 /*6 plane*/ * 2 /*2 triangle/side*/ * 3 /*3 index/triangle*/;
	static unsigned int indices[];

	static void Init()
	{
		layout.AddElement(&position);
		layout.AddElement(&uv);
		layout.Update();
	}
};

template<RENDERER xRenderer> FormatDesc<float> TexturedBox<xRenderer>::position(3, "pos", FDS_POSITION, 0, 0, 0);
template<RENDERER xRenderer> FormatDesc<float> TexturedBox<xRenderer>::uv(2, "uv", FDS_TEXCOORD, 0, 0, position.GetEndOffset());
template<RENDERER xRenderer> Layout TexturedBox<xRenderer>::layout;
template<RENDERER xRenderer> typename TexturedBox<xRenderer>::VertexFormat TexturedBox<xRenderer>::vertices[nVertices] =
{
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } }, //0
	{ { +0.5f, -0.5f, -0.5f }, { 2.0f, 0.0f } }, //1
	{ { +0.5f, +0.5f, -0.5f }, { 2.0f, 2.0f } }, //2
	{ { -0.5f, +0.5f, -0.5f }, { 0.0f, 2.0f } }, //3
	{ { -0.5f, -0.5f, +0.5f }, { 0.0f, 0.0f } }, //4
	{ { +0.5f, -0.5f, +0.5f }, { 2.0f, 0.0f } }, //5
	{ { +0.5f, +0.5f, +0.5f }, { 2.0f, 2.0f } }, //6
	{ { -0.5f, +0.5f, +0.5f }, { 0.0f, 2.0f } }, //7
};

template<RENDERER xRenderer> unsigned int TexturedBox<xRenderer>::indices[nIndices] =
{
	2, 0, 1,
	0, 2, 3,
	5, 4, 6,
	6, 4, 7,
	6, 1, 5,
	2, 1, 6,
	7, 0, 3,
	4, 0, 7,
	6, 7, 2,
	7, 3, 2,
	4, 1, 0,
	5, 1, 4,
};
