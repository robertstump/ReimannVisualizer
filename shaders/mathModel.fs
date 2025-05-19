#version 330 core

in vec3 FragPos;
in float FragArg;

out vec4 FragColor;

vec3 hsv2rgb(float h, float s, float v) {
    float c = v * s;
    float h_prime = mod(h / (3.1415926 / 3.0), 6.0);
    float x = c * (1.0 - abs(mod(h_prime, 2.0) - 1.0));
    vec3 rgb;
    if (0.0 <= h_prime && h_prime < 1.0) rgb = vec3(c, x, 0);
    else if (1.0 <= h_prime && h_prime < 2.0) rgb = vec3(x, c, 0);
    else if (2.0 <= h_prime && h_prime < 3.0) rgb = vec3(0, c, x);
    else if (3.0 <= h_prime && h_prime < 4.0) rgb = vec3(0, x, c);
    else if (4.0 <= h_prime && h_prime < 5.0) rgb = vec3(x, 0, c);
    else rgb = vec3(c, 0, x);
    return rgb + vec3(v - c);
}

void main() { 
    float brightness = length(FragPos);
    float hue = FragArg;
    float sat = 1.0;

    vec3 color = hsv2rgb(hue, sat, brightness * 0.25);
    FragColor = vec4(color, 1.0);
}
