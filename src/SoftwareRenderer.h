#pragma once
#include "Math.h"

namespace SoftRend {
	struct Material{
		std::string name = "unnamed";
		int texIndex = -1;
		int normalMapIndex = -1;
		uint8_t diffuseColor[4] = {255,255,255,255};
		bool hasTexture = false;
		bool hasNormalMap = false;
		bool hasTransparency = false;
	};

	struct Light{
		Vec3 dir = {0,-1,0};
		Vec3 color = {1,1,1};
		float brightness = 1.0f;
	};

	struct Triangle{
		Vec3 p[3] = {0,0,0};
		Vec2 t[3] = {0,0,0};
		Vec3 n[3] = {0,0,0};
		Vec3 tangent;

		Material material;
		//Vec3 color;

		Triangle() = default;
		Triangle(const Triangle &tri){
			p[0] = tri.p[0];
			p[1] = tri.p[1];
			p[2] = tri.p[2];
			t[0] = tri.t[0];
			t[1] = tri.t[1];
			t[2] = tri.t[2];
			n[0] = tri.n[0];
			n[1] = tri.n[1];
			n[2] = tri.n[2];
			tangent = tri.tangent;
			material = tri.material;
		}
	};

	struct Mesh{
		std::vector<Triangle> tris;
		std::unordered_map<std::string,Material> materials;
		std::vector<PNGImage> textures;

		bool hasNormals = false;
		bool hasTexCoords = false;

		std::string name = "untitled";

		bool ReadOBJFile(std::string path){
			std::ifstream file;
			char junk[10];
			file.open(path);
			if (!file)
				return false;

			name = path;

			std::string currMat = "";
			std::vector<Vec3> vertices;
			std::vector<Vec3> normals;
			std::vector<Vec2> texCoords;

			while(!file.eof()){
				std::string type;
				file >> type;
				if (!type.compare("v")){
					float vx,vy,vz;
					file >> vx >> vy >> vz;
					vertices.push_back({-vx,vy,-vz});
				}
				else if(!type.compare("vn")){
					float nx,ny,nz;
					file >> nx >> ny >> nz;
					normals.push_back({-nx,ny,-nz});
				}
				else if(!type.compare("vt")){
					float u,v;
					file >> u >> v;
					texCoords.push_back({u,v});
				}
				else if(!type.compare("f")){
					float index,normal,tex;
					Triangle tri;
					for (int i=0;i<3;i++){
						file >> index;
						tri.p[i] = vertices[index-1];
						if (file.peek() == '/'){
							file.get();
							if (file.peek() != '/'){
								hasTexCoords = true;
								file >> tex;
								tri.t[i] = texCoords[tex-1];
							}
							if (file.peek() == '/'){
								file.get();
								hasNormals = true;
								file >> normal;
								tri.n[i] = normals[normal-1];
							}
						}
					}
					if (materials.count(currMat))
						tri.material = materials[currMat];
					tris.push_back(tri);
					if (file.peek() != '\n'){
						Triangle tri2;
						tri2.p[0] = tri.p[2];
						tri2.p[2] = tri.p[0];
						tri2.t[0] = tri.t[2];
						tri2.t[2] = tri.t[0];
						tri2.n[0] = tri.n[2];
						tri2.n[2] = tri.n[0];
						if (materials.count(currMat))
							tri2.material = materials[currMat];
						float p,n,t;
						file >> p;
						tri2.p[1] = vertices[p-1]; 
						if (file.peek() == '/'){
							file.get();
							if (file.peek() != '/'){
								hasTexCoords = true;
								file >> t;
								tri2.t[1] = texCoords[tex-1];
							}
							if (file.peek() == '/'){
								file.get();
								hasNormals = true;
								file >> n;
								tri2.n[1] = normals[normal-1];
							}
						}
						tris.push_back(tri2);
					}
				}
				else if (!type.compare("mtllib")){
					std::string mtlFile;
					file >> mtlFile;
					int pos = path.find_last_of('/');
					if (pos != std::string::npos)
						mtlFile = path.substr(0,pos+1)+mtlFile;
					ReadMTLFile(mtlFile);
				}
				else if (!type.compare("usemtl")){
					std::string matName;
					file >> matName;
					if (materials.count(matName)){
						currMat = matName;
					}
				}
				else
					file.ignore(1024,'\n');
			}
			file.close();
			return true;
		}

		bool ReadMTLFile(std::string path){
			std::ifstream file;
			file.open(path);
			if (!file)
				return false;

			std::string currMat = "";

			while(!file.eof()){
				std::string type;
				file >> type;
				if (!type.compare("newmtl")){
					std::string name;
					file >> name;
					Material mat;
					mat.name = name;
					currMat = name;
					materials[name] = mat;
				}
				else if (!type.compare("Kd")){
					if (materials.count(currMat)){
						float r,g,b;
						file >> r >> g >> b;
						materials[currMat].diffuseColor[0] = r*255;
						materials[currMat].diffuseColor[1] = g*255;
						materials[currMat].diffuseColor[2] = b*255;
					}
				}
				else if (!type.compare("map_Kd")){
					if (materials.count(currMat)){
						std::string texPath;
						file >> texPath;
						int pos = path.find_last_of('/');
						if (pos != std::string::npos)
							texPath = path.substr(0,pos+1)+texPath;
						PNGImage tex;
						if (ReadPNGFile(texPath,tex)){
							materials[currMat].hasTexture = true;
							materials[currMat].hasTransparency = tex.hasTransparency;
							textures.push_back(tex);
							materials[currMat].texIndex = textures.size()-1;
						}
					}
				}
				else if (!type.compare("map_Bump")){
					if (materials.count(currMat)){
						std::string texPath;
						file >> texPath;
						int pos = path.find_last_of('/');
						if (pos != std::string::npos)
							texPath = path.substr(0,pos+1)+texPath;
						PNGImage tex;
						if (ReadPNGFile(texPath,tex)){
							materials[currMat].hasNormalMap = true;
							textures.push_back(tex);
							materials[currMat].normalMapIndex = textures.size()-1;		
						}
					}
				}
				else
					file.ignore(1024,'\n');
			}
			file.close();
			return true;
		}
	};
	
	enum RenderState{
		BACKFACE_CULLING = (1<<0),
		//view modes
		VIEW_WIREFRAME = (1<<1),
		VIEW_SHADED = (1<<2),
		VIEW_DEPTH = (1<<3),
		USE_TEXTURES = (1<<4), 
		USE_NORMAL_MAPS = (1<<5)
	};

	int renderState; 
	
	float zFar,zNear; //Renderer

	Mat4x4 projMat; //Renderer
					
	float *depthBuffer; //Renderer

	Vec3 rotD = {0,0,0}; //Model

	float rotSpeed = 50; //Model
	float moveSpeed = 8; //Model

	Vec3 transDir = {0,0,10}; //Model
	float transX = 0; //Model
	float transY = 0; //Model
	float transZ = 10; //Model

	Mat4x4 rotMatY; //Model
	Mat4x4 rotMatX; //Model 
	Mat4x4 rotMatZ; //Model
					
	Vec3 cameraPos = {0,0,0}; //Renderer
	Vec3 cameraDir = {0,0,1}; //Renderer
	Vec3 cameraUp =  {0,1,0}; //Renderer
	Vec3 cameraRight = {1,0,0}; //Renderer
	Vec3 cameraRot = {0,0,0}; //Renderer
	float cameraMoveSpeed = 16; //Renderer
	float cameraRotSpeed = 4; //Renderer

	std::vector<Light> lights = std::vector<Light>(3); //Renderer
	
	int ClipTriangle(Vec3 planeP, Vec3 planeN, Triangle &inTri, Triangle &outTri1, Triangle &outTri2){
		float dP1 = planeN.Dot(inTri.p[0]-planeP);
		float dP2 = planeN.Dot(inTri.p[1]-planeP);
		float dP3 = planeN.Dot(inTri.p[2]-planeP);

		int numClippedTris = 0;
		int numInsidePoints = 0;
		int numOutsidePoints = 0;
		Vec3 insidePoints[3],outsidePoints[3];
		Vec2 insideTex[3],outsideTex[3];
		if (dP1 >= 0){
			insideTex[numInsidePoints] = inTri.t[0];
			insidePoints[numInsidePoints++] = inTri.p[0];
		}
		else{
			outsideTex[numOutsidePoints] = inTri.t[0];
			outsidePoints[numOutsidePoints++] = inTri.p[0];
		}
		if (dP2 >= 0){
			insideTex[numInsidePoints] = inTri.t[1];
			insidePoints[numInsidePoints++] = inTri.p[1];
		}
		else{
			outsideTex[numOutsidePoints] = inTri.t[1];
			outsidePoints[numOutsidePoints++] = inTri.p[1];
		}
		if (dP3 >= 0){
			insideTex[numInsidePoints] = inTri.t[2];
			insidePoints[numInsidePoints++] = inTri.p[2];
		}
		else{
			outsideTex[numOutsidePoints] = inTri.t[2];
			outsidePoints[numOutsidePoints++] = inTri.p[2];
		}

		float t = 0;

		if (numInsidePoints == 3){
			outTri1 = inTri;
			numClippedTris++;
		}
		else if (numInsidePoints == 1){
			outTri1.p[0] = insidePoints[0];
			outTri1.t[0] = insideTex[0];
			outTri1.p[1] = LinePlaneIntersect(planeP,planeN,insidePoints[0],outsidePoints[0],t);
			outTri1.t[1] = t*(outsideTex[0]-insideTex[0])+insideTex[0];
			outTri1.p[2] = LinePlaneIntersect(planeP,planeN,insidePoints[0],outsidePoints[1],t);
			outTri1.t[2] = t*(outsideTex[1]-insideTex[0])+insideTex[0];
			outTri1.n[0] = inTri.n[0];
			outTri1.n[1] = inTri.n[1];
			outTri1.n[2] = inTri.n[2];
			outTri1.tangent = inTri.tangent;
			outTri1.material = inTri.material;
			numClippedTris++;
		}
		else if (numInsidePoints == 2){
			outTri1.p[0] = insidePoints[0];
			outTri1.t[0] = insideTex[0];
			outTri1.p[1] = insidePoints[1];
			outTri1.t[1] = insideTex[1];
			outTri1.p[2] = LinePlaneIntersect(planeP,planeN,insidePoints[0],outsidePoints[0],t);
			outTri1.t[2] = t*(outsideTex[0]-insideTex[0])+insideTex[0];
			outTri1.n[0] = inTri.n[0];
			outTri1.n[1] = inTri.n[1];
			outTri1.n[2] = inTri.n[2];
			outTri1.tangent = inTri.tangent;
			outTri1.material = inTri.material;
			numClippedTris++;

			outTri2.p[0] = insidePoints[1];
			outTri2.t[0] = insideTex[1];
			outTri2.p[1] = outTri1.p[2];
			outTri2.t[1] = outTri1.t[2];
			outTri2.p[2] = LinePlaneIntersect(planeP,planeN,insidePoints[1],outsidePoints[0],t);
			outTri2.t[2] = t*(outsideTex[0]-insideTex[1])+insideTex[1];
			outTri2.n[0] = inTri.n[0];
			outTri2.n[1] = inTri.n[1];
			outTri2.n[2] = inTri.n[2];
			outTri2.tangent = inTri.tangent;
			outTri2.material = inTri.material;
			numClippedTris++;
		}
		return numClippedTris;
	}

	void DrawLine(std::shared_ptr<Canvas> canvas,Vec3 p1, Vec3 p2,int lWidth,Canvas::Pixel data){
		if (p1.x == p2.x && p1.y == p2.y){
			return;
		}

		int canvasWidth = canvas->width/canvas->pixelScale;
		int canvasHeight = canvas->height/canvas->pixelScale;

		int dx = std::abs(p1.x - p2.x);
		int dy = std::abs(p1.y - p2.y);
		if (dx >= dy){
			float yStep = 0;
			float wStep = 0;
			Vec3 start = (p1.x<p2.x)?p1:p2;
			Vec3 end = (p1.x<p2.x)?p2:p1;
			if (start.x != end.x) yStep = (end.y-start.y)/std::abs(end.x-start.x);
			float tStep = 1.0f/(end.x-start.x);
			float t = 0;
			for (int x=start.x;x<=end.x;x++){
				int y = start.y+(x-start.x)*yStep;
				float dz = ((1.0f-t)*start.z + t*end.z);
				int index = (y)*canvasWidth+(x);
				index = std::clamp(index,0,(int)(canvasWidth*canvasHeight-1));
				if (dz < depthBuffer[index]){
					depthBuffer[index] = dz*0.999f;
					for (int w=lWidth/-2;w<=lWidth/2;w++){
						for (int h=lWidth/-2;h<=lWidth/2;h++){
							canvas->SetPixel(x+w,y+h,data);
						}
					}
				}
				t += tStep;
			}
		}
		else{
			float xStep = 0;
			float wStep = 0;
			Vec3 start = (p1.y<p2.y)?p1:p2;
			Vec3 end = (p1.y<p2.y)?p2:p1;
			if (start.y != end.y) xStep = (end.x-start.x)/std::abs(end.y-start.y);
			float tStep = 1.0f/(end.y-start.y);
			float t = 0;
			for (int y=start.y;y<=end.y;y++){
				int x = start.x+(y-start.y)*xStep;
				float dz = ((1.0f-t)*start.z + t*end.z);
				int index = (y)*canvasWidth+(x);
				index = std::clamp(index,0,(int)(canvasWidth*canvasHeight-1));
				if (dz < depthBuffer[index]){
					depthBuffer[index] = dz*0.999f;
					for (int w=lWidth/-2;w<=lWidth/2;w++){
						for (int h=lWidth/-2;h<=lWidth/2;h++){
							canvas->SetPixel(x+w,y+h,data);
						}
					}
				}
				t += tStep;
			}
		}
	}

	void DrawTriangle(std::shared_ptr<Canvas> canvas,Triangle tri, int lWidth, Canvas::Pixel data){
		DrawLine(canvas,tri.p[0],tri.p[1],lWidth,data);
		DrawLine(canvas,tri.p[1],tri.p[2],lWidth,data);
		DrawLine(canvas,tri.p[2],tri.p[0],lWidth,data);
	}

	void DrawTriangleFilled(std::shared_ptr<Canvas> canvas,Mesh &meshObj,Triangle tri){
		int canvasWidth = canvas->width/canvas->pixelScale;
		int canvasHeight = canvas->height/canvas->pixelScale;

		Vec3 p[3] = {tri.p[0],tri.p[1],tri.p[2]};
		Vec2 t[3] = {tri.t[0],tri.t[1],tri.t[2]};
		Vec3 n[3] = {tri.n[0],tri.n[1],tri.n[2]};
		if (p[1].y < p[0].y){
			std::swap(p[0],p[1]);
			std::swap(t[0],t[1]);
		}
		if (p[2].y < p[0].y){
			std::swap(p[0],p[2]);
			std::swap(t[0],t[2]);
		}
		if (p[2].y < p[1].y){
			std::swap(p[1],p[2]);
			std::swap(t[1],t[2]);
		}
		int x1 = p[0].x;
		int x2 = p[1].x;
		int x3 = p[2].x;
		int y1 = p[0].y;
		int y2 = p[1].y;
		int y3 = p[2].y;
		float z1 = p[0].z;
		float z2 = p[1].z;
		float z3 = p[2].z;

		int texIndex,nMapIndex;
		int texWidth,nMapWidth;
		int texHeight,nMapHeight;
		uint8_t *tData,*nData;
		if (tri.material.hasTexture){
			texIndex = tri.material.texIndex;
			texWidth = meshObj.textures[texIndex].width;
			texHeight = meshObj.textures[texIndex].height;
			tData = meshObj.textures[texIndex].data;
		}
		if (tri.material.hasNormalMap){
			nMapIndex = tri.material.normalMapIndex;
			nMapWidth = meshObj.textures[nMapIndex].width;
			nMapHeight = meshObj.textures[nMapIndex].height;
			nData = meshObj.textures[nMapIndex].data;
		}

		float x1Step = 0;
		float w1Step = 0;
		float u1Step = 0;
		float v1Step = 0;
		float x2Step = 0;
		float w2Step = 0;
		float u2Step = 0;
		float v2Step = 0;
		float x3Step = 0;
		float w3Step = 0;
		float u3Step = 0;
		float v3Step = 0;
		float z1Step = 0;
		float z2Step = 0;
		float z3Step = 0;

		int xs,xe;
		float zs,ze,ws,we,us,ue,vs,ve;
		if (y1 != y2){
			x1Step = (float)(x2-x1)/(y2-y1);
			z1Step = (float)(z2-z1)/(y2-y1);
			w1Step = (p[1].w-p[0].w)/(y2-y1);
			u1Step = (t[1].u-t[0].u)/(y2-y1);
			v1Step = (t[1].v-t[0].v)/(y2-y1);
		}
		if (y2 != y3){
			x2Step = (float)(x3-x2)/(y3-y2);
			z2Step = (float)(z3-z2)/(y3-y2);
			w2Step = (p[2].w-p[1].w)/(y3-y2);
			u2Step = (t[2].u-t[1].u)/(y3-y2);
			v2Step = (t[2].v-t[1].v)/(y3-y2);
		}

		if (y1 != y3){
			x3Step = (float)(x3-x1)/(y3-y1);
			z3Step = (float)(z3-z1)/(y3-y1);
			w3Step = (p[2].w-p[0].w)/(y3-y1);
			u3Step = (t[2].u-t[0].u)/(y3-y1);
			v3Step = (t[2].v-t[0].v)/(y3-y1);
		}

		Vec3 normal = (n[0]+n[1]+n[2]); 
		normal.Normalize();
		Vec3 tangent = tri.tangent; 
		Vec3 bitangent = normal.Cross(tangent);
		bitangent.Normalize();
		Mat4x4 tbn = {tangent.x,bitangent.x,normal.x,0,
			tangent.y,bitangent.y,normal.y,0,
			tangent.z,bitangent.z,normal.z,0,
			0,0,0,0};
		for (int y=y1;y<=y3;y++){
			xe = x1+(y-y1)*x3Step;
			ze = z1+(y-y1)*z3Step;
			we = p[0].w+(y-y1)*w3Step;
			ue = t[0].u+(y-y1)*u3Step;
			ve = t[0].v+(y-y1)*v3Step;

			if (y<=y2){
				xs = x1+(y-y1)*x1Step;
				zs = z1+(y-y1)*z1Step;
				ws = p[0].w+(y-y1)*w1Step;
				us = t[0].u+(y-y1)*u1Step;
				vs = t[0].v+(y-y1)*v1Step;
			}	
			else{
				xs = x2+(y-y2)*x2Step;
				zs = z2+(y-y2)*z2Step;
				ws = p[1].w+(y-y2)*w2Step;
				us = t[1].u+(y-y2)*u2Step;
				vs = t[1].v+(y-y2)*v2Step;
			}
			//DrawTexturedLine(y,xs,xe,zs,ze,w1,w2,u1,u2,v1,v2,texWidth,texHeight,data,tri);
			if (xs > xe){
				std::swap(xs,xe);
				std::swap(zs,ze);
				std::swap(ws,we);
				std::swap(us,ue);
				std::swap(vs,ve);
			}
			float tStep = 1.0f/((float)xe-xs);
			float t = 0.0f;
			for (int x=xs;x<xe;x++){
				float dw = ((1.0f-t)*ws + t*we);
				float dz = ((1.0f-t)*zs + t*ze);
				float du = ((1.0f-t)*us + t*ue)/dw;
				float dv = ((1.0f-t)*vs + t*ve)/dw;
				//std::printf("dz: %f dw: %f du: %f dv: %f\n",dz,dw,du,dv);
				du -= (int)du;
				if (du < 0) du = 1+du;
				dv -= (int)dv;
				if (dv < 0) dv = 1+dv;
				int index = (y)*canvasWidth+(x);
				if (index >= canvasWidth*canvasHeight || index < 0)
					std::printf("Depth Buffer Index out of Range [%d, %d] Index: %d Size: %d\n",x,y,index,canvasWidth*canvasHeight);

				int canvasIndex = index*4;

				if (dz < depthBuffer[index]){
					float lightColor[3] = {0.5f,0.5f,0.5f};
					Vec3 nDir;
					if (tri.material.hasNormalMap){
						int nx = (int)(du*nMapWidth);
						int ny = (int)(dv*nMapHeight);
						int nIndex = (ny*nMapWidth+nx)*4;
						nDir = {nData[nIndex],nData[nIndex+1],nData[nIndex+2]};
						nDir /= 255.0f;
						nDir = {2*nDir.x-1,2*nDir.y-1,nDir.z};
						nDir *= tbn;
						nDir.Normalize();
					}
					for (auto& light : lights){
						float brightness = 1.0f;
						float ldp = 0;
						if (tri.material.hasNormalMap && renderState&USE_NORMAL_MAPS){
							ldp = -nDir.Dot(light.dir); 
						}
						else{
							ldp = -normal.Dot(light.dir);
						}
						brightness = std::max(0.0f,ldp*light.brightness);
						lightColor[0] += brightness;
						lightColor[1] += brightness;
						lightColor[2] += brightness;

					}	
					lightColor[0] = std::min(lightColor[0],1.0f);
					lightColor[1] = std::min(lightColor[1],1.0f);
					lightColor[2] = std::min(lightColor[2],1.0f);
					if (tri.material.hasTexture && renderState&USE_TEXTURES){
						int tx = (int)(du*(texWidth));
						int ty = (int)(dv*(texHeight));
						int texIndex = (ty*texWidth+tx)*4;
						float alpha = tData[texIndex+3]/255.0f;
						uint8_t red = tData[texIndex]*lightColor[0]*alpha;
						uint8_t green = tData[texIndex+1]*lightColor[1]*alpha;
						uint8_t blue = tData[texIndex+2]*lightColor[2]*alpha;
						red += canvas->pixels[canvasIndex]*(1-alpha);
						green += canvas->pixels[canvasIndex+1]*(1-alpha);
						blue += canvas->pixels[canvasIndex+2]*(1-alpha);
						canvas->SetPixel(x,y,{red,green,blue});
						if (alpha == 1)depthBuffer[index] = dz;
					}
					else{
						canvas->SetPixel(x,y,{tri.material.diffuseColor[0]*lightColor[0],tri.material.diffuseColor[1]*lightColor[1],tri.material.diffuseColor[2]*lightColor[2]});
						depthBuffer[index] = dz;
					}
				}
				t += tStep;
			}
		}
	}

}
