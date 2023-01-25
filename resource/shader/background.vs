#version 330 core

out vec2 v_uv;

void main() {
    uint idx = uint(gl_VertexID);
    gl_Position = vec4( idx & 1U, idx >> 1U, 0.0, 0.5 ) * 4.0 - 1.0;
    v_uv = vec2( gl_Position.xy * 0.5 + 0.5 );
}
