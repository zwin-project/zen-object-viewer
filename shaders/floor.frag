#version 320 es
precision mediump float;

in vec2 uv;
out vec4 out_color;

void
main()
{
  // out_color = texture(floor_texture, uv);
  // primary: #111F4D
  out_color = vec4(17.0/255.0, 31.0/255.0, 77.0/255.0, 1.0);
}
