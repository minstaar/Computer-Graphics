#version 330

layout (location = 0) in vec3 Position_MC;
layout (location = 1) in vec3 Normal_MC;
layout (location = 2) in vec2 TexCoord_in;

out vec3 Position_EC;
out vec3 Normal_EC;
out vec2 TexCoord;

uniform mat4 ModelViewMatrix;
uniform mat3 ModelViewMatrixInvTrans;
uniform mat4 ModelViewProjectionMatrix;

void main() {
    Position_EC  = (ModelViewMatrix * vec4(Position_MC, 1.0)).xyz;
    Normal_EC = normalize(ModelViewMatrixInvTrans * Normal_MC);
    TexCoord = TexCoord_in;

    gl_Position = ModelViewProjectionMatrix * vec4(Position_MC, 1.0);
}

