#pragma once

struct Mat4x4{
	float val[4][4] = {0};

	//Matrix Matrix
	friend Mat4x4 operator*(const Mat4x4 &lhs, const Mat4x4 &rhs){
		/*Vec3 a1 = {lhs.val[0][0],lhs.val[0][1],lhs.val[0][2],lhs.val[0][3]};
		Vec3 a2 = {lhs.val[1][0],lhs.val[1][1],lhs.val[1][2],lhs.val[1][3]};
		Vec3 a3 = {lhs.val[2][0],lhs.val[2][1],lhs.val[2][2],lhs.val[2][3]};
		Vec3 a4 = {lhs.val[3][0],lhs.val[3][1],lhs.val[3][2],lhs.val[3][3]};
		Vec3 b1 = {rhs.val[0][0],rhs.val[1][0],rhs.val[2][0],rhs.val[3][0]};
		Vec3 b2 = {rhs.val[0][1],rhs.val[1][1],rhs.val[2][1],rhs.val[3][1]};
		Vec3 b3 = {rhs.val[0][2],rhs.val[1][2],rhs.val[2][2],rhs.val[3][2]};
		Vec3 b4 = {rhs.val[0][3],rhs.val[1][3],rhs.val[2][3],rhs.val[3][3]};*/
		Mat4x4 m;
		
		for (int r=0;r<4;r++){
			for (int c=0;c<4;c++){
				for (int i=0;i<4;i++){
					m.val[r][c] += lhs.val[r][i]*rhs.val[i][c];
				}
			}
		}
		return m;
		/*return {a1.Dot(b1)+a1.w*b1.w,a1.Dot(b2)+a1.w*b2.w,a1.Dot(b3)+a1.w*b3.w,a1.Dot(b4)+a1.w*b4.w,
			a2.Dot(b1)+a2.w*b1.w,a2.Dot(b2)+a2.w*b2.w,a2.Dot(b3)+a2.w*b3.w,a2.Dot(b4)+a2.w*b4.w,
			a3.Dot(b1)+a3.w*b1.w,a3.Dot(b2)+a3.w*b2.w,a3.Dot(b3)+a3.w*b3.w,a3.Dot(b4)+a3.w*b4.w,
			a4.Dot(b1)+a4.w*b1.w,a4.Dot(b2)+a4.w*b2.w,a4.Dot(b3)+a4.w*b3.w,a4.Dot(b4)+a4.w*b4.w};*/
		
	}

	Mat4x4& operator*=(const Mat4x4 &rhs){
		(*this) = (*this)*rhs;
		return *this;
	}
};

struct Vec2{
	float u = 0;
	float v = 0;


	//by constant
	friend Vec2 operator*(const Vec2 &lhs,const float &rhs){
		return {lhs.u*rhs,lhs.v*rhs};
	}
	friend Vec2 operator*(const float &lhs,const Vec2 &rhs){
		return {rhs.u*lhs,rhs.v*lhs};
	}
	Vec2& operator*=(const float &rhs){
		(*this) = (*this)*rhs;
		return *this;
	}
	friend Vec2 operator/(const Vec2 &lhs,const float &rhs){
		return {lhs.u/rhs,lhs.v/rhs};
	}
	friend Vec2 operator/(const float &lhs,const Vec2 &rhs){
		return {rhs.u/lhs,rhs.v/lhs};
	}
	Vec2& operator/=(const float &rhs){
		(*this) = (*this)/rhs;
		return *this;
	}

	friend Vec2 operator+(Vec2 lhs, const Vec2 &rhs){
		return {lhs.u+rhs.u,lhs.v+rhs.v};
	}
	Vec2& operator+=(const Vec2 &lhs){
		*this = (*this)+lhs;
		return *this;
	}
	friend Vec2 operator-(Vec2 lhs, const Vec2 &rhs){
		return {lhs.u-rhs.u,lhs.v-rhs.v};
	}
	Vec2& operator-=(const Vec2 &lhs){
		*this = (*this)-lhs;
		return *this;
	}
};

struct Vec3{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;

	//by constant
	friend Vec3 operator*(const Vec3 &lhs,const float &rhs){
		return {lhs.x*rhs,lhs.y*rhs,lhs.z*rhs,lhs.w};
	}
	friend Vec3 operator*(const float &lhs,const Vec3 &rhs){
		return {rhs.x*lhs,rhs.y*lhs,rhs.z*lhs,rhs.w};
	}
	Vec3& operator*=(const float &rhs){
		(*this) = (*this)*rhs;
		return *this;
	}
	friend Vec3 operator/(const Vec3 &lhs,const float &rhs){
		return {lhs.x/rhs,lhs.y/rhs,lhs.z/rhs,lhs.w};
	}
	friend Vec3 operator/(const float &lhs,const Vec3 &rhs){
		return {rhs.x/lhs,rhs.y/lhs,rhs.z/lhs,rhs.w};
	}
	Vec3& operator/=(const float &rhs){
		(*this) = (*this)/rhs;
		return *this;
	}

	friend Vec3 operator+(const Vec3 &lhs,const float &rhs){
		return {lhs.x+rhs,lhs.y+rhs,lhs.z+rhs,lhs.w};
	}
	friend Vec3 operator+(const float &lhs,const Vec3 &rhs){
		return {rhs.x+lhs,rhs.y+lhs,rhs.z+lhs,rhs.w};
	}
	Vec3& operator+=(const float &rhs){
		(*this) = (*this)+rhs;
		return *this;
	}

	friend Vec3 operator-(const Vec3 &lhs,const float &rhs){
		return {lhs.x-rhs,lhs.y-rhs,lhs.z-rhs,lhs.w};
	}
	friend Vec3 operator-(const float &lhs,const Vec3 &rhs){
		return {rhs.x-lhs,rhs.y-lhs,rhs.z-lhs,rhs.w};
	}
	Vec3& operator-=(const float &rhs){
		(*this) = (*this)-rhs;
		return *this;
	}


	//Vector-Vector (Element-wise)
	friend Vec3 operator+(Vec3 lhs, const Vec3 &rhs){
		return {lhs.x+rhs.x,lhs.y+rhs.y,lhs.z+rhs.z};
	}
	Vec3& operator+=(const Vec3 &lhs){
		(*this) = (*this)+lhs;
		return *this;
	}
	friend Vec3 operator*(Vec3 lhs,const Vec3 &rhs){
		return {lhs.x*rhs.x,lhs.y*rhs.y,lhs.z*rhs.z};
	}
	Vec3& operator*=(const Vec3 &lhs){
		*this = (*this)*lhs;
		return *this;
	}
	friend Vec3 operator-(Vec3 lhs, const Vec3 &rhs){
		return {lhs.x-rhs.x,lhs.y-rhs.y,lhs.z-rhs.z};
	}
	Vec3& operator-=(const Vec3 &lhs){
		*this = (*this)-lhs;
		return *this;
	}

	//Matrix-Vector
	friend Vec3 operator*(Vec3 lhs,const Mat4x4 &rhs){
		float x = lhs.Dot({rhs.val[0][0],rhs.val[0][1],rhs.val[0][2]})+lhs.w*rhs.val[0][3];
		float y = lhs.Dot({rhs.val[1][0],rhs.val[1][1],rhs.val[1][2]})+lhs.w*rhs.val[1][3];
		float z = lhs.Dot({rhs.val[2][0],rhs.val[2][1],rhs.val[2][2]})+lhs.w*rhs.val[2][3];
		float w = lhs.Dot({rhs.val[3][0],rhs.val[3][1],rhs.val[3][2]})+lhs.w*rhs.val[3][3];
		return {x,y,z,w};
	}
	friend Vec3 operator*(Mat4x4 lhs, const Vec3 &rhs){
		return rhs*lhs;
	}
	Vec3& operator*=(const Mat4x4 &rhs){
		*this = (*this)*rhs;
		return *this;
	}

	float Dot(const Vec3 &operand){
		return x*operand.x+y*operand.y+z*operand.z;
	}
	Vec3 Cross(const Vec3 &operand){
		float tmpX = y*operand.z - z*operand.y;
		float tmpY = z*operand.x - x*operand.z;
		float tmpZ = x*operand.y - y*operand.x;
		return {tmpX,tmpY,tmpZ};
	}
	void Normalize(){
		float l = std::sqrt(x*x+y*y+z*z);
		if (l == 0)
			return;
		x /= l;
		y /= l;
		z /= l;
	}
};

Vec3 LinePlaneIntersect(Vec3 planeP,Vec3 planeN,Vec3 lineStart,Vec3 lineEnd,float &t){
	float planeD = -planeN.Dot(planeP);
	float ad = planeN.Dot(lineStart);
	float bd = planeN.Dot(lineEnd);
	t = (-planeD-ad)/(bd-ad);
	Vec3 lineStartEnd = lineEnd-lineStart;
	Vec3 lineIntersect = lineStartEnd*t;
	Vec3 final = lineIntersect + lineStart;
	final.w = t*(lineEnd.w-lineStart.w)+lineStart.w;
	return final;
}
