#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aArg;

out vec3 FragPos;
out float FragArg;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = aPos;
    FragArg = aArg;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
