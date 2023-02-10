#include <iostream>
#include <cmath>
#include <fstream>
#include <list>
#include <chrono>
#include "GraphicsEngine/graphicsEngine.h"
#include "PNGReader.h"
#include "SoftwareRenderer.h"
#include "Math.h"

const float PI = 3.141592f;

class MainWindow : public Window {
	public:
		MainWindow(int w, int h, std::string name) : Window(w,h,name){};

		double currTime = 0;

		double currMouseX = 0;
		double currMouseY = 0;

		float canvasWidth;
		float canvasHeight;
		int pixelScale = 2;

		bool showDebugText = false;
		bool showTimings = false;

		SoftRend::Mesh meshObj;
		std::string objFilePath = "Models/cube.obj";

		PNGImage texture;

		int maxTexId = 209;

		std::shared_ptr<Canvas> canvas;

		void OnStartup(){
			glClearColor(1.0f,0.0f,0.0f,1.0f);

			canvasWidth = baseWidth/pixelScale;
			canvasHeight = baseHeight/pixelScale;

			SoftRend::depthBuffer = new float[(int)(canvasWidth*canvasHeight)];			
			SoftRend::renderState = SoftRend::BACKFACE_CULLING|SoftRend::VIEW_SHADED|SoftRend::USE_TEXTURES|SoftRend::USE_NORMAL_MAPS; 
			
			meshObj.ReadOBJFile(objFilePath);

			canvas = std::make_shared<Canvas>(0,0,baseWidth,baseHeight,pixelScale);
			canvas->ChangeTextureParamater(GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			canvas->ChangeTextureParamater(GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			canvas->SetBackgroundColor({25,25,25,0});
			//canvas->visable = false;
			guiMan.AddElement(canvas,100);

			std::shared_ptr<ListLayout> debugTextList = std::make_shared<ListLayout>(ListLayout::ListMode::Vertical,0,baseHeight-50);
			debugTextList->SetPadding(0,10);

			std::shared_ptr<Text> meshName = std::make_shared<Text>(0,0,15,"[Tab]Mesh Name: " + meshObj.name);
			meshName->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(meshName,200);

			std::shared_ptr<Text> numVertices = std::make_shared<Text>(0,0,15,"\tNumber of Vertices: " + std::to_string(meshObj.tris.size()*3));
			numVertices->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(numVertices,201);

			std::shared_ptr<Text> numTris = std::make_shared<Text>(0,0,15,"\tNumber of Triangles: " + std::to_string(meshObj.tris.size()));
			numTris->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(numTris,202);

			std::shared_ptr<Text> numTrisRend = std::make_shared<Text>(0,0,15,"\tNumber of Rendered Triangles: " + std::to_string(meshObj.tris.size()));
			numTrisRend->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(numTrisRend,203);

			std::shared_ptr<Text> bfCullingText = std::make_shared<Text>(0,0,15,"\t[C]Backface Culling: " + std::string((SoftRend::renderState&SoftRend::BACKFACE_CULLING)?"TRUE":"FALSE"));
			bfCullingText->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(bfCullingText,204);

			std::shared_ptr<Text> viewShadedText = std::make_shared<Text>(0,0,15,"\t[1]View Shaded: " + std::string((SoftRend::renderState&SoftRend::VIEW_SHADED)?"TRUE":"FALSE"));
			viewShadedText->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(viewShadedText,205);

			std::shared_ptr<Text> viewWireframeText = std::make_shared<Text>(0,0,15,"\t[2]View Wireframe: " + std::string((SoftRend::renderState&SoftRend::VIEW_WIREFRAME)?"TRUE":"FALSE"));
			viewWireframeText->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(viewWireframeText,206);

			std::shared_ptr<Text> viewTexturedText = std::make_shared<Text>(0,0,15,"\t[3]View Textured: " + std::string((SoftRend::renderState&SoftRend::USE_TEXTURES)?"TRUE":"FALSE"));
			viewTexturedText->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(viewTexturedText,207);

			std::shared_ptr<Text> viewDepthText = std::make_shared<Text>(0,0,15,"\t[4]View Depth: " + std::string((SoftRend::renderState&SoftRend::VIEW_DEPTH)?"TRUE":"FALSE"));
			viewDepthText->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(viewDepthText,208);
			
			std::shared_ptr<Text> viewNormalsText = std::make_shared<Text>(0,0,15,"\t[n]View Normal Map: " + std::string((SoftRend::renderState&SoftRend::USE_NORMAL_MAPS)?"TRUE":"FALSE"));
			viewNormalsText->SetTextColor(1.0f,1.0f,1.0f);
			debugTextList->AddElement(viewNormalsText,209);

			std::shared_ptr<Text> timeText = std::make_shared<Text>(baseWidth-175,baseHeight-50,15,"Setup: 0.000000\nVertex: 0.000000\nRender: 0.000000\n\tClipping: 0.000000\n\tRasterize: 0.000000");
			timeText->SetTextColor(1.0f,1.0f,1.0f);
			timeText->visable = showTimings;
			guiMan.AddElement(timeText,300);

			guiMan.AddLayout(debugTextList);

			for (int i=200;i<=maxTexId;i++){
				auto debugText = guiMan.GetElement<Text>(i);
				debugText->visable = showDebugText;
			}

			SoftRend::zFar = 1000.0f;
			SoftRend::zNear = 0.1f;
			float fov = 1.0f/std::tan(45*PI/180.0f);
			float aspectRatio = canvasHeight/canvasWidth;

			//Projection Matrix
			SoftRend::projMat.val[0][0] = aspectRatio*fov;
			SoftRend::projMat.val[1][1] = fov;
		    //projMat.val[2][2] = (zFar/(zFar-zNear));
			//projMat.val[2][3] = -(zFar*zNear/(zFar-zNear));
			SoftRend::projMat.val[2][2] = (SoftRend::zFar+SoftRend::zNear)/(SoftRend::zFar-SoftRend::zNear);
			SoftRend::projMat.val[2][3] = -(2*SoftRend::zFar*SoftRend::zNear)/(SoftRend::zFar-SoftRend::zNear);
			SoftRend::projMat.val[3][2] = 1;
			
		}

		void OnUpdate(double dTime){
			glClear(GL_COLOR_BUFFER_BIT);

			auto setupTp1 = std::chrono::high_resolution_clock::now();

			//auto canvas = guiMan.GetElement<Canvas>(100);
			canvas->ClearCanvas();
			//rotD.y += 100*(dTime-currTime);
			float xOff = canvasWidth/2;
			float yOff = canvasHeight/2;
			int scale = canvasWidth;
			float meshScale = 1;
			

			//std::printf("Camera Pos = [%f,%f]\r",cameraPos.x,cameraPos.y);

			for (int i=0;i<canvasWidth*canvasHeight;i++){
				SoftRend::depthBuffer[i] = 1;
			}

			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_UP) == GLFW_PRESS)
				SoftRend::cameraPos.y += SoftRend::cameraMoveSpeed*(dTime-currTime);
			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_DOWN) == GLFW_PRESS)
				SoftRend::cameraPos.y -= SoftRend::cameraMoveSpeed*(dTime-currTime);
			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				SoftRend::cameraRot.y -= SoftRend::cameraRotSpeed*(dTime-currTime);
			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				SoftRend::cameraRot.y += SoftRend::cameraRotSpeed*(dTime-currTime);

			Vec3 cameraRight = SoftRend::cameraDir.Cross({0,1,0});

			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_W) == GLFW_PRESS)
				SoftRend::cameraPos += SoftRend::cameraDir*SoftRend::cameraMoveSpeed*(dTime-currTime);
			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_S) == GLFW_PRESS)
				SoftRend::cameraPos -= SoftRend::cameraDir*SoftRend::cameraMoveSpeed*(dTime-currTime);
			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_A) == GLFW_PRESS)
				SoftRend::cameraPos -= SoftRend::cameraRight*SoftRend::cameraMoveSpeed*(dTime-currTime);
			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_D) == GLFW_PRESS)
				SoftRend::cameraPos += SoftRend::cameraRight*SoftRend::cameraMoveSpeed*(dTime-currTime);
			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_E) == GLFW_PRESS)
				SoftRend::rotD.z -= SoftRend::rotSpeed*(dTime-currTime);
			if (GraphicsEngine::input.GetKeyState(window, GLFW_KEY_Q) == GLFW_PRESS)
				SoftRend::rotD.z += SoftRend::rotSpeed*(dTime-currTime);

			currTime = dTime;
			Vec3 rotR = {SoftRend::rotD.x*PI/180,SoftRend::rotD.y*PI/180,SoftRend::rotD.z*PI/180};

			//Rotation Matrices
			SoftRend::rotMatY.val[0][0] = std::cos(rotR.y);
			SoftRend::rotMatY.val[0][2] = std::sin(rotR.y);
			SoftRend::rotMatY.val[1][1] = 1;
			SoftRend::rotMatY.val[2][0] = -std::sin(rotR.y);
			SoftRend::rotMatY.val[2][2] = std::cos(rotR.y);

			SoftRend::rotMatX.val[0][0] = 1;
			SoftRend::rotMatX.val[1][1] = std::cos(rotR.x);
			SoftRend::rotMatX.val[1][2] = -std::sin(rotR.x);
			SoftRend::rotMatX.val[2][1] = std::sin(rotR.x);
			SoftRend::rotMatX.val[2][2] = std::cos(rotR.x);

			SoftRend::rotMatZ.val[0][0] = std::cos(rotR.z);
			SoftRend::rotMatZ.val[0][1] = -std::sin(rotR.z);
			SoftRend::rotMatZ.val[1][0] = std::sin(rotR.z);
			SoftRend::rotMatZ.val[1][1] = std::cos(rotR.z);
			SoftRend::rotMatZ.val[2][2] = 1;

			Mat4x4 worldMat = SoftRend::rotMatY*SoftRend::rotMatX*SoftRend::rotMatZ;

			//Point at
			Vec3 forward = {0,0,1};
			Vec3 up = {0,1,0};
			Mat4x4 camRotMat = {std::cos(SoftRend::cameraRot.y),0,std::sin(SoftRend::cameraRot.y),0,
				0,1,0,0,
				-std::sin(SoftRend::cameraRot.y),0,std::cos(SoftRend::cameraRot.y),0};
			camRotMat *= {1,0,0,0,
						  0,std::cos(SoftRend::cameraRot.x),-std::sin(SoftRend::cameraRot.x),0,
						  0,std::sin(SoftRend::cameraRot.x),std::cos(SoftRend::cameraRot.x),0};
			forward *= camRotMat;
			up *= camRotMat;
			Vec3 right = forward.Cross(up);
			SoftRend::cameraDir = forward;
			SoftRend::cameraUp = up;
			SoftRend::cameraRight = right;

			Mat4x4 viewMat = {right.x,right.y,right.z,-SoftRend::cameraPos.Dot(right),
				up.x,up.y,up.z,-SoftRend::cameraPos.Dot(up),
				forward.x,forward.y,forward.z,-SoftRend::cameraPos.Dot(forward),
				0,0,0,1};
           
			SoftRend::lights[0].dir = SoftRend::cameraDir;
			SoftRend::lights[0].brightness = 0.40f;
		    SoftRend::lights[1].dir = SoftRend::cameraUp*-1;
			SoftRend::lights[1].brightness = 0.30f;		
			SoftRend::lights[2].dir = -1*SoftRend::cameraRight;
			SoftRend::lights[2].brightness = 0.20f;

			std::vector<SoftRend::Triangle> trisToRender;
			std::vector<SoftRend::Triangle> trisToRenderTrans;


			auto setupTp2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> setupTime = setupTp2 - setupTp1;

			auto vertexTp1 = std::chrono::high_resolution_clock::now();

			for (auto &triangle : meshObj.tris){
				Vec3 p1 = triangle.p[0];
				Vec3 p2 = triangle.p[1];
				Vec3 p3 = triangle.p[2];
				Vec2 t1 = triangle.t[0];
				Vec2 t2 = triangle.t[1];
				Vec2 t3 = triangle.t[2];
				Vec3 n1 = triangle.n[0];
				Vec3 n2 = triangle.n[1];
				Vec3 n3 = triangle.n[2];

				//scale mesh
				p1 *= meshScale;
				p2 *= meshScale;
				p3 *= meshScale;

				//rotate mesh
				p1 *= worldMat;
				p2 *= worldMat;
				p3 *= worldMat;
				n1 *= worldMat;
				n2 *= worldMat;
				n3 *= worldMat;

				//translate mesh
				p1 += SoftRend::transDir;
				p2 += SoftRend::transDir;
				p3 += SoftRend::transDir;

				Vec3 line1 = p2-p1;
				Vec3 line2 = p3-p1;
				Vec3 normal;
				if (!meshObj.hasNormals){
					normal = line1.Cross(line2);
					triangle.n[0] = normal;
					triangle.n[1] = normal;
					triangle.n[2] = normal;
				}
				else{
					normal = n1;
					normal += n2;
					normal += n3;
				}

				normal.Normalize();
				Vec2 dUV1 = t2-t1;
				Vec2 dUV2 = t3-t1;
				Vec3 dPos1 = p2-p1;
				Vec3 dPos2 = p3-p1;
				float f = 1.0f/(dUV1.u*dUV2.v - dUV2.u*dUV1.v);
				triangle.tangent = (dPos1*dUV2.v - dPos2*dUV1.v)*f;
				triangle.tangent.Normalize();
				Vec3 vecToCamera = p1-SoftRend::cameraPos;
				vecToCamera.Normalize();

				if (normal.Dot(vecToCamera) < 0.0f || !(SoftRend::renderState&SoftRend::BACKFACE_CULLING)){
					                                    
					//Transform to view space           
					p1 *= viewMat;
					p2 *= viewMat;
					p3 *= viewMat;

					//Clip against near plane
					Vec3 planeP = {0,0,SoftRend::zNear};
					Vec3 planeN = {0,0,1};
					SoftRend::Triangle origTri = triangle;
					origTri.p[0] = p1;
					origTri.p[1] = p2;
					origTri.p[2] = p3;
					origTri.t[0] = t1;
					origTri.t[1] = t2;
					origTri.t[2] = t3;
					origTri.n[0] = n1;
					origTri.n[1] = n2;
					origTri.n[2] = n3;
					SoftRend::Triangle clippedTris[2];
					int numClippedTris = SoftRend::ClipTriangle(planeP,planeN,origTri,clippedTris[0],clippedTris[1]);
					for (int i=0;i<numClippedTris;i++){
						SoftRend::Triangle &tri = clippedTris[i];
						//Project to screen
						tri.p[0] *= SoftRend::projMat;
						tri.p[1] *= SoftRend::projMat;
						tri.p[2] *= SoftRend::projMat;

						tri.p[0] /= tri.p[0].w;
						tri.p[1] /= tri.p[1].w;
						tri.p[2] /= tri.p[2].w;

						tri.t[0] /= tri.p[0].w;
						tri.t[1] /= tri.p[1].w; 
						tri.t[2] /= tri.p[2].w;

						//scale viewport
						tri.p[0] = {tri.p[0].x*canvasWidth,tri.p[0].y*canvasHeight,tri.p[0].z,1.0f/tri.p[0].w};
						tri.p[1] = {tri.p[1].x*canvasWidth,tri.p[1].y*canvasHeight,tri.p[1].z,1.0f/tri.p[1].w};
						tri.p[2] = {tri.p[2].x*canvasWidth,tri.p[2].y*canvasHeight,tri.p[2].z,1.0f/tri.p[2].w};

						//translate viewport
						tri.p[0] = {tri.p[0].x+xOff,tri.p[0].y+yOff,tri.p[0].z,tri.p[0].w};
						tri.p[1] = {tri.p[1].x+xOff,tri.p[1].y+yOff,tri.p[1].z,tri.p[1].w};
						tri.p[2] = {tri.p[2].x+xOff,tri.p[2].y+yOff,tri.p[2].z,tri.p[2].w};
						if (tri.material.hasTransparency)
							trisToRenderTrans.push_back(tri);
						else
							trisToRender.push_back(tri);
					}
				}
			}

			//append trisToRenderTrans to end of trisToRender
			trisToRender.insert(trisToRender.end(),trisToRenderTrans.begin(),trisToRenderTrans.end());

			auto vertexTp2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> vertexTime = vertexTp2 - vertexTp1;

			int numRendTris = 0;

			auto renderTp1 = std::chrono::high_resolution_clock::now();

			std::chrono::time_point<std::chrono::system_clock> clippingTp1,clippingTp2,rasterizeTp1,rasterizeTp2;
			std::chrono::duration<double> clippingTime = std::chrono::duration<double>::zero();
			std::chrono::duration<double> rasterizeTime = std::chrono::duration<double>::zero();
			for (auto &tri : trisToRender){
				clippingTp1 = std::chrono::high_resolution_clock::now();
				SoftRend::Triangle clippedTris[2];
				std::vector<SoftRend::Triangle> triList;
				int numTris = 1;
				triList.push_back(tri);
				int triListStart = 0;
				for (int i=0;i<4;i++){
					while(numTris > 0){
						SoftRend::Triangle newTri = triList[triListStart++];
						numTris--;

						int newTris = 0;
						switch(i){
							case 0:newTris = SoftRend::ClipTriangle({0,0,0},{1,0,0},newTri,clippedTris[0],clippedTris[1]);break;
							case 1:newTris = SoftRend::ClipTriangle({0,0,0},{0,1,0},newTri,clippedTris[0],clippedTris[1]);break;
							case 2:newTris = SoftRend::ClipTriangle({canvasWidth-1,0,0},{-1,0,0},newTri,clippedTris[0],clippedTris[1]);break;
							case 3:newTris = SoftRend::ClipTriangle({0,canvasHeight-1,0},{0,-1,0},newTri,clippedTris[0],clippedTris[1]);break;
						}
						for (int w=0;w<newTris;w++){
							triList.push_back(clippedTris[w]);
						}
					}
					numTris = triList.size()-triListStart;
				}
				clippingTp2 = std::chrono::high_resolution_clock::now();
				clippingTime += clippingTp2 - clippingTp1;

				rasterizeTp1 = std::chrono::high_resolution_clock::now();
				for (auto it=triList.begin()+triListStart;it != triList.end();it++){
					uint8_t wireColor[3] = {0};
					if (!(SoftRend::renderState&SoftRend::VIEW_SHADED)){
						wireColor[0] = 255;
						wireColor[1] = 255;
						wireColor[2] = 255;
					}
					if(SoftRend::renderState&SoftRend::VIEW_WIREFRAME)DrawTriangle(canvas,*it,1,{wireColor[0],wireColor[1],wireColor[2]});
					if(SoftRend::renderState&SoftRend::VIEW_SHADED)DrawTriangleFilled(canvas,meshObj,*it);
					numRendTris++;
				}
				rasterizeTp2 = std::chrono::high_resolution_clock::now();
				rasterizeTime += rasterizeTp2 - rasterizeTp1;
			}

			auto renderTp2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> renderTime = renderTp2 - renderTp1;

			if (showTimings){
				auto timeText = guiMan.GetElement<Text>(300);
				timeText->SetSubText(7,8,std::to_string(setupTime.count()));
				timeText->SetSubText(23,8,std::to_string(vertexTime.count()));
				timeText->SetSubText(39,8,std::to_string(renderTime.count()));
				timeText->SetSubText(57,8,std::to_string(clippingTime.count()));
				timeText->SetSubText(76,8,std::to_string(rasterizeTime.count()));
			}
			if (showDebugText)
				guiMan.GetElement<Text>(203)->SetSubText(30,8,std::to_string(numRendTris)+"        ");

			float ar = (float)canvasWidth/canvasHeight;

			if (SoftRend::renderState&SoftRend::VIEW_DEPTH){
				for (int y=0;y<canvasHeight;y++)
					for (int x=0;x<canvasWidth;x++){
						int index = x+y*canvasWidth;
						uint8_t val = SoftRend::depthBuffer[index]*255*10;
						canvas->SetPixel(x,y,{val,val,val});
					}
			}


			guiMan.DrawElements();
		}

		void OnShutdown(){

		}

		void OnEvent(Event &ev){
			guiMan.HandleEvent(ev);
			switch(ev.GetType()){
				case Event::Key:
					{
						int keyCode = static_cast<KeyEvent*>(&ev)->GetKeyCode();
						int keyState = static_cast<KeyEvent*>(&ev)->GetKeyState();
						if (keyCode == GLFW_KEY_C && keyState == GLFW_PRESS){
							SoftRend::renderState ^= SoftRend::BACKFACE_CULLING;
							auto bfCullingText = guiMan.GetElement<Text>(204);
							std::string text = "\t[C]Backface Culling: ";
							bfCullingText->SetText(text + (SoftRend::renderState&SoftRend::BACKFACE_CULLING?"TRUE":"FALSE"));
						}
						if (keyCode == GLFW_KEY_1 && keyState == GLFW_PRESS){
							SoftRend::renderState ^= SoftRend::VIEW_SHADED;
							auto textObj = guiMan.GetElement<Text>(205);
							std::string text = "\t[1]View Shaded: ";
							textObj->SetText(text + (SoftRend::renderState&SoftRend::VIEW_SHADED?"TRUE":"FALSE"));
						}
						if (keyCode == GLFW_KEY_2 && keyState == GLFW_PRESS){
							SoftRend::renderState ^= SoftRend::VIEW_WIREFRAME;
							auto textObj = guiMan.GetElement<Text>(206);
							std::string text = "\t[2]View Wireframe: ";
							textObj->SetText(text + (SoftRend::renderState&SoftRend::VIEW_WIREFRAME?"TRUE":"FALSE"));
						}
						if (keyCode == GLFW_KEY_3 && keyState == GLFW_PRESS){
							SoftRend::renderState ^= SoftRend::USE_TEXTURES;
							auto textObj = guiMan.GetElement<Text>(207);
							std::string text = "\t[3]View Textured: ";
							textObj->SetText(text + (SoftRend::renderState&SoftRend::USE_TEXTURES?"TRUE":"FALSE"));
						}
						if (keyCode == GLFW_KEY_4 && keyState == GLFW_PRESS){
							SoftRend::renderState ^= SoftRend::VIEW_DEPTH;
							auto textObj = guiMan.GetElement<Text>(208);
							std::string text = "\t[4]View Depth: ";
							textObj->SetText(text + (SoftRend::renderState&SoftRend::VIEW_DEPTH?"TRUE":"FALSE"));
						}
						if (keyCode == GLFW_KEY_N && keyState == GLFW_PRESS){
							SoftRend::renderState ^= SoftRend::USE_NORMAL_MAPS;
							auto textObj = guiMan.GetElement<Text>(209);
							std::string text = "\t[N]View Normal Map: ";
							textObj->SetText(text + (SoftRend::renderState&SoftRend::USE_NORMAL_MAPS?"TRUE":"FALSE"));
						}
						if (keyCode == GLFW_KEY_TAB && keyState == GLFW_PRESS){
							showDebugText = !showDebugText;
							for (int i=200;i<=maxTexId;i++){
								auto debugText = guiMan.GetElement<Text>(i);
								debugText->visable = showDebugText;
							}
						}
						if (keyCode == GLFW_KEY_T && keyState == GLFW_PRESS){
							showTimings = !showTimings;
							auto timeText = guiMan.GetElement<Text>(300);
							timeText->visable = showTimings;

						}
						if (keyCode == GLFW_KEY_R && keyState == GLFW_PRESS){
							SoftRend::cameraPos = { 0 };
							SoftRend::cameraDir = {0,0,1};
							SoftRend::cameraRot = {0,0,0};
							SoftRend::rotD = {0,0,0};
						}
						break;
					}
				case Event::MouseScroll:
					{
						double scrollOffset = static_cast<MouseScrollEvent*>(&ev)->GetScrollOffset();
						float scrollMul = 0.25f;
						int lShiftBtn = GraphicsEngine::input.GetKeyState(window,GLFW_KEY_LEFT_SHIFT); 
						int rShiftBtn = GraphicsEngine::input.GetKeyState(window,GLFW_KEY_RIGHT_SHIFT); 
						SoftRend::cameraPos += SoftRend::cameraDir*SoftRend::cameraMoveSpeed*scrollOffset*((lShiftBtn | rShiftBtn == GLFW_PRESS)?scrollMul/10.0f:scrollMul);
						break;
					}
				case Event::MouseButton:
					{
						MouseButtonEvent::ButtonType btn = static_cast<MouseButtonEvent*>(&ev)->GetButtonType();
						MouseButtonEvent::ButtonState state =  static_cast<MouseButtonEvent*>(&ev)->GetButtonState();
						double mx = static_cast<MouseButtonEvent*>(&ev)->GetMouseX();
						double my = static_cast<MouseButtonEvent*>(&ev)->GetMouseY();
						if ((btn == MouseButtonEvent::ButtonType::Middle || btn == MouseButtonEvent::ButtonType::Right) && state == MouseButtonEvent::ButtonState::Pressed){
							currMouseX = mx;
							currMouseY = my;	
						}
						break;
					}
				case Event::MouseCursor:
					{
						double mx = static_cast<MouseMoveEvent*>(&ev)->GetMouseX();
						double my = static_cast<MouseMoveEvent*>(&ev)->GetMouseY();
						int mouseBtnMid = glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_MIDDLE);
						int mouseBtnRight = glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT);
						int lShiftBtn = GraphicsEngine::input.GetKeyState(window,GLFW_KEY_LEFT_SHIFT); 
						int rShiftBtn = GraphicsEngine::input.GetKeyState(window,GLFW_KEY_RIGHT_SHIFT); 
						float speedMul = (lShiftBtn | rShiftBtn == GLFW_PRESS)?0.001f/10.0f:0.001f;
						if (mouseBtnMid == GLFW_PRESS){
							Vec3 cameraRight = SoftRend::cameraDir.Cross(SoftRend::cameraUp);
							SoftRend::cameraPos += (currMouseX-mx)*SoftRend::cameraRight*SoftRend::cameraMoveSpeed*speedMul;
							SoftRend::cameraPos += (currMouseY-my)*SoftRend::cameraUp*SoftRend::cameraMoveSpeed*speedMul;
						}	
						if (mouseBtnRight == GLFW_PRESS){
							SoftRend::cameraRot.x += (currMouseY-my)*SoftRend::cameraRotSpeed*speedMul;
							SoftRend::cameraRot.y += (currMouseX-mx)*SoftRend::cameraRotSpeed*speedMul;
						}
					 	currMouseX = mx;	
					 	currMouseY = my;	
						break;
					}
			}
		}
};

int main(int argc, char** argv){

	GraphicsEngine::Init();
	MainWindow *mainWin = new MainWindow(1280,720,"Software Renderer");
	
	if (argc > 1)
		mainWin->objFilePath = argv[1]; 
	
	
	GraphicsEngine::AddWindow(mainWin);
	GraphicsEngine::Run();

	return 0;
}
