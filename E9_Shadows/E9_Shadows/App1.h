// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;

	Light* light[2];
	AModel* model;

	CubeMesh* cube;
	SphereMesh* sphere;


	XMFLOAT3 cubePos;

	XMFLOAT3 lightPos1;
	XMFLOAT3 lightDir1;

	XMFLOAT3 lightPos2;
	XMFLOAT3 lightDir2;

	ShadowShader* shadowShader;
	DepthShader* depthShader;




	ShadowMap* shadowMap[2];
};

#endif