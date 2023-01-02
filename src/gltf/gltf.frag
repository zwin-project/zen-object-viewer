#version 320 es

precision mediump float;

// uniform vec3 focus_color_diff;

in vec3 normal;

out vec4 outputColor;

void
main()
{
  // outputColor = vec4(0.5 * normalize(normal) + 0.4 + focus_color_diff, 1.0);
  outputColor = vec4(0.5 * normalize(normal) + 0.5, 1.0);
}
