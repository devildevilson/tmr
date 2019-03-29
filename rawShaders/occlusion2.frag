#version 450

layout(early_fragment_tests) in;

layout(std430, set = 1, binding = 0) buffer visibleBuffer {
  uint visibles[];
};

layout(location = 0) in flat uint objId;

//layout(location = 0) out vec4 out_Color;

void main() {
	visibles[objId] = 1;
	
	//out_Color = vec4(1.0, 0.0, 0.0, 1.0);
	//out_Color = unpackUnorm4x8(uint(objId));
}
