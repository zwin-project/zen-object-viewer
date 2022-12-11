#version 320 es

precision mediump float;

in vec3 normal;

uniform vec4 color;

out vec4 outputColor;

void
main()
{
  outputColor = vec4(0.5 * normalize(normal) + 0.5, 1.0);
}
