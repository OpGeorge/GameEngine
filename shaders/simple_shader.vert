#version 450

layout(location = 0)  in vec3 position;

layout(location = 1) in vec3 color;

layout(location = 2) in vec3 normal;

layout(location = 3) in vec2 uv;

struct PointLight{
	vec4 position;
	vec4 color;

};


layout(set= 0,binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLightColor;
	PointLight pointLights[10];
	int numLights;

} ubo;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fargNormalWorld;
layout(location = 3) out vec2 fragUV;


layout(push_constant) uniform Push{
	mat4 modelMatrix; // stores porjection * view * model
	mat4 normalMatrix;
	

}push;

void main(){

	vec4 positionWorld = push.modelMatrix * vec4(position,1.0);

	gl_Position = ubo.projection * ubo.view * positionWorld;

	fargNormalWorld = normalize(mat3(push.normalMatrix) * normal);
	fragPosWorld = positionWorld.xyz;
	fragColor = color;
	fragUV = uv;
	
}