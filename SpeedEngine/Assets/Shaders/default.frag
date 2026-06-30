#version 330 core

in vec2 uv;
in vec3 normal;

out vec4 FragColor;

uniform vec4 color;
uniform vec2 uvScale;
uniform bool useTexture;
uniform float intensity;
uniform int frameIndex;
uniform float numRows;
uniform float numCols;

uniform sampler2D diffuse0;
uniform sampler2D diffuse1;

void main()
{
   float col = float(frameIndex % int(numCols));
   float row = float(frameIndex / int(numCols));

   vec2 uvScale = vec2(1.0f / numCols, 1.0f / numRows);
   vec2 uvOffset = vec2(col * uvScale.x, (numRows - 1.0 - row) * uvScale.y);
   vec2 finalUV = (uv * uvScale) + uvOffset;
   
   // overwrite use data to manually test on the shader
   // int testFrameIndex = 1;   
   // float testNumRows = 1.0f;
   // float testNumCols = 1.0f;
   // row = float(testFrameIndex % int(testNumRows));
   // col = float(testFrameIndex / int(testNumCols));
   // uvScale = vec2(1.0f / testNumRows, 1.0f / testNumCols);
   // uvOffset = vec2(col * uvScale.x, (testNumRows - 1.0 - row) * uvScale.y);
   // finalUV = (uv * uvScale) + uvOffset;

   vec4 texColor;
   if (useTexture) {
      texColor = texture(diffuse0, finalUV);
   } 
   else {
      texColor = texture(diffuse1, finalUV);
   }

   FragColor = texColor * color;
   
   if (FragColor.a < 0.01){
      discard;
   }

}