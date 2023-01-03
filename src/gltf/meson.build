gltf_vert_glsl = custom_target(
  'gltf.vertex.h',
  command: [textify, 'gltf_vertex_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'gltf.vert',
  output: 'gltf.vertex.h',
)

gltf_texture_frag_glsl = custom_target(
  'gltf.texture.fragment.h',
  command: [textify, 'gltf_texture_fragment_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'gltf_texture.frag',
  output: 'gltf.texture.fragment.h',
)

gltf_color_frag_glsl = custom_target(
  'gltf.color.fragment.h',
  command: [textify, 'gltf_color_fragment_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'gltf_color.frag',
  output: 'gltf.color.fragment.h',
)

_sources = [
  '../../include/tiny_gltf.cc',
  'jpeg-texture.cc',
  'main.cc',
  gltf_vert_glsl,
  gltf_texture_frag_glsl,
  gltf_color_frag_glsl,
]

_viewer_dep = [
  glm_dep,
  zukou_dep,
  jpeg_dep,
]

executable(
  'zen-gltf-viewer',
  _sources,
  install: true,
  dependencies: [ _viewer_dep ],
  include_directories: public_inc,
)
