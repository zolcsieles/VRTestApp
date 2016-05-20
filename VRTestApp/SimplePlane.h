#pragma once

#include "CommonRenderer.h"
#include "zls_math/zls_math.h"

template<RENDERER xRenderer>
struct SimplePlane
{
	typename MyTypes<xRenderer>::Model* model;
	typename MyTypes<xRenderer>::VertexBuffer* vertexBuffer;

	SimplePlane() : model(nullptr), vertexBuffer(nullptr)
	{
	}

	static FormatDesc<float> position;
	static FormatDesc<float> uv;
	static Layout layout;

	struct VertexFormat
	{
		zls::math::vec2 pos;
		zls::math::vec2 uv;
	};

	static const int nVertices = 4;
	static VertexFormat vertices[];

	static void Init()
	{
		layout.AddElement(&position);
		layout.AddElement(&uv);
		layout.Update();
	}
};

template<RENDERER xRenderer> FormatDesc<float> SimplePlane<xRenderer>::position(2, "pos", FDS_POSITION, 0, 0, 0);
template<RENDERER xRenderer> FormatDesc<float> SimplePlane<xRenderer>::uv(2, "uv", FDS_TEXCOORD, 0, 0, position.GetEndOffset());
template<RENDERER xRenderer> Layout SimplePlane<xRenderer>::layout;
template<RENDERER xRenderer> typename SimplePlane<xRenderer>::VertexFormat SimplePlane<xRenderer>::vertices[nVertices] =
{
	{ { -1.0f, -1.0f }, { 0.0f, 0.0f } }, //bl
	{ { -1.0f, 1.0f }, { 0.0f, 1.0f } }, //tl
	{ { 1.0f, -1.0f }, { 1.0f, 0.0f } }, //br
	{ { 1.0f, 1.0f }, { 1.0f, 1.0f } } //tr
};
