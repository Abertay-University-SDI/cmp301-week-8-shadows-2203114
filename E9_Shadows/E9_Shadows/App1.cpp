// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());

	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());


	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 2048;
	int shadowmapHeight = 2048;
	int sceneWidth = 100;
	int sceneHeight = 100;


	cubePos = XMFLOAT3(0, 0, 0);

	lightPos1 = XMFLOAT3(0, 0, -10);

	lightDir1 = XMFLOAT3(0, -0.7, 0.7);

	lightPos2 = XMFLOAT3(0, 0, -10);

	lightDir2 = XMFLOAT3(0, -0.7, 0.7);


	// This is your shadow map
	shadowMap[0] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	shadowMap[1] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Configure directional light
	light[0] = new Light();
	light[0]->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	light[0]->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light[0]->setDirection(lightDir1.x,lightDir1.y,lightDir1.z);
	light[0]->setPosition(lightPos1.x,lightPos1.y,lightPos1.z);
	light[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);


	// Configure directional light
	light[1] = new Light();
	light[1]->setAmbientColour(1, 0, 0, 1.0f);
	light[1]->setDiffuseColour(1.0f, 0, 0, 1.0f);
	light[1]->setDirection(lightDir2.x, lightDir2.y, lightDir2.z);
	light[1]->setPosition(lightPos2.x, lightPos2.y, lightPos2.z);
	light[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{

	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	for (int i = 0; i < 2; i++)
	{
		// Set the render target to be the render to texture.
		shadowMap[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

		// get the world, view, and projection matrices from the camera and d3d objects.
		light[i]->generateViewMatrix();
		XMMATRIX lightViewMatrix = light[i]->getViewMatrix();
		XMMATRIX lightProjectionMatrix = light[i]->getOrthoMatrix();
		XMMATRIX worldMatrix = renderer->getWorldMatrix();

		worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
		// Render floor
		mesh->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
		XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
		worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
		// Render model
		model->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), model->getIndexCount());



		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(lightPos2.x, lightPos2.y, lightPos2.z);
		cube->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), model->getIndexCount());



		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(lightPos1.x, lightPos1.y, lightPos1.z);
		sphere->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), model->getIndexCount());
		// Set back buffer as render target and reset view port.
		renderer->setBackBufferRenderTarget();
		renderer->resetViewport();
	}
	
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();



	light[0]->setDirection(lightDir1.x, lightDir1.y, lightDir1.z);
	light[0]->setPosition(lightPos1.x, lightPos1.y, lightPos1.z);


	light[1]->setDirection(lightDir2.x, lightDir2.y, lightDir2.z);
	light[1]->setPosition(lightPos2.x, lightPos2.y, lightPos2.z);

	ID3D11ShaderResourceView* sm[2];
	sm[0] = shadowMap[0]->getDepthMapSRV();
	sm[1] = shadowMap[1]->getDepthMapSRV();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), sm, light);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), sm, light);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());




	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(lightPos2.x,lightPos2.y,lightPos2.z);
	cube->sendData(renderer->getDeviceContext());

	/*textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
	textureShader->render(renderer->getDeviceContext(), cube->getIndexCount());*/

	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), sm, light);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(lightPos1.x, lightPos1.y, lightPos1.z);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), sm, light);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	gui();
	renderer->endScene();
}



void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);


	ImGui::SliderFloat("cube pos x", &cubePos.x, -100, 100);
	ImGui::SliderFloat("cube pos y", &cubePos.y, -100, 100);
	ImGui::SliderFloat("cube pos z", &cubePos.z, -100, 100);


	ImGui::SliderFloat("Light pos x", &lightPos1.x, -100, 100);
	ImGui::SliderFloat("Light pos y", &lightPos1.y, -100, 100);
	ImGui::SliderFloat("Light pos z", &lightPos1.z, -100, 100);


	ImGui::SliderFloat("Light dir x", &lightDir1.x, -100, 100);
	ImGui::SliderFloat("Light dir y", &lightDir1.y, -100, 100);
	ImGui::SliderFloat("Light dir z", &lightDir1.z, -100, 100);



	ImGui::SliderFloat("Light2 pos x", &lightPos2.x, -100, 100);
	ImGui::SliderFloat("Light2 pos y", &lightPos2.y, -100, 100);
	ImGui::SliderFloat("Light22 pos z", &lightPos2.z, -100, 100);


	ImGui::SliderFloat("Light2 dir x", &lightDir2.x, -100, 100);
	ImGui::SliderFloat("Light2 dir y", &lightDir2.y, -100, 100);
	ImGui::SliderFloat("Light2 dir z", &lightDir2.z, -100, 100);
	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

