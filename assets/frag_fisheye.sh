#version 300 es
//#extension GL_OES_EGL_image_external : enable
#extension GL_OES_EGL_image_external_essl3 : require
precision highp float;

uniform mat3x3 pramK;       //畸变参数,前5个值
uniform float  centerX;     //归一化的圆心x坐标，应该在0.5左右
uniform float  centerY;     //归一化的圆心y坐标，应该在0.5左右
uniform float  radius;      //归一化的圆半径，应该在0.5左右

uniform samplerExternalOES sTexture; //纹理内容数据
in vec2 vTextureCoord;      //接收从顶点着色器过来的参数
out vec4 fragColor;

#define CV_PI  3.1415926535897932384626433832795

void main()
{
    /*
    mat3x3 pramK = mat3x3(
           1,0,0,
           0,0,0,
           0,0,0);

    float centerX = 521.0 / 980.0;
    float centerY = 455.0 / 980.0;
    float radius = 0.5;
    */

	mat3x3 mt= mat3x3(
		1.0,  0.0,                0.0,
		0.0,  -1.0,               1.224646799e-16,
		0.0,  -1.224646799e-16,  -1.0);

    vec3 m = vec3(0,0,0);
    vec3 v = vec3(0,0,0);
    vec2 newcoord =  vec2(0,0);

	float phi   =  vTextureCoord.t * CV_PI/2.0 + CV_PI/2.0;
	float theta =  2.0 * CV_PI - vTextureCoord.s * 2.0 * CV_PI;

	//空间坐标
	m[0] = sin(phi)*cos(theta);
	m[1] = sin(phi)*sin(theta);
	m[2] = cos(phi);

	//空间坐标旋转
    for (int i = 0; i < 3; i++)
    {
      v[i] = mt[0][i] * m[0] + mt[1][i] * m[1] + mt[2][i] * m[2];
    }

	phi = acos(v[2]);
	//theta = atan2(v[1], v[0]);

	if (phi < 0.0)
	{
		phi = -phi;
		//theta += PI;
	}
	if (phi > CV_PI)
	{
		phi = 2.0*CV_PI - phi;
		//theta += PI;
	}
	//根据等距离投影，投影成圆
	float r = sqrt(v[0] * v[0] + v[1] * v[1]);

	if (r == 0.0)
	{
		newcoord.s = 0.0;
		newcoord.t = 0.0;
	}
	else
	{
		newcoord.s = phi * v.x / r * 1. / CV_PI;
		newcoord.t = phi * v.y / r * 1. / CV_PI;
	}

	//根据畸变与角度大小进行缩放
	r = phi / CV_PI*2.0;
	float scale = (((pramK[1][1] * r + pramK[1][0])*r + pramK[0][2])*r + pramK[0][1]) *r + pramK[0][0];
	newcoord.s *= scale;
	newcoord.t *= scale;

	//计算半径缩放
	newcoord.s *= radius / 0.5;
	newcoord.t *= radius / 0.5;

	//输出结果
	newcoord.s += centerX;
	newcoord.t += centerY;

	//限制输出
	if(newcoord.s < 0.0)
	{
		newcoord.s = 0.0;
	}
	if(newcoord.s > 1.0)
	{
		newcoord.s = 1.0;
	}
	if(newcoord.t < 0.0)
	{
		newcoord.t = 0.0;
	}
	if(newcoord.t > 1.0)
	{
		newcoord.t = 1.0;
	}

   //进行纹理采样
   fragColor = texture(sTexture, newcoord);
}