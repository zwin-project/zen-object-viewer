#version 320 es

precision mediump float;

uniform sampler2D in_texture;

uniform vec2 Offset;
uniform vec2 Scale;
uniform float Rotation;

in vec3 normal;
in vec2 uv;

out vec4 outputColor;

void
main()
{
  // outputColor = vec4(0.5 * normalize(normal) + 0.5, 1.0);

  mat3 translation = mat3(1,0,0, 0,1,0, Offset.x, Offset.y, 1);
  mat3 rotation = mat3(
      cos(Rotation), sin(Rotation), 0,
     -sin(Rotation), cos(Rotation), 0,
                  0,             0, 1
  );
  mat3 scale = mat3(Scale.x,0,0, 0,Scale.y,0, 0,0,1);

  mat3 matrix = translation * rotation * scale;
  vec2 uvTransformed = ( matrix * vec3(uv.xy, 1) ).xy;

  // outputColor = vec4(uv, 1.0, 1.0);
  // outputColor = texture(in_texture, uv);
  outputColor = texture(in_texture, uvTransformed);
}
