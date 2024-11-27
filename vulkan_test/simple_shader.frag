#version 450


layout (location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;


layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec3 directionToLight;
} ubo;

layout(push_constant) uniform Push {
    mat4 transform;
    mat4 modelMatrix;
} push;

void main() {
 outColor = vec4(fragColor, 1.0);
}