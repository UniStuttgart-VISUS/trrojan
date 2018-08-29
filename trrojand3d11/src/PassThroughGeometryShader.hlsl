/// <copyright file="PassThroughGeometryShader.hlsl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#define QUAD_INST (1)
#define PER_VERTEX_COLOUR (1)
#define FLOAT_COLOUR (1)
#define PER_VERTEX_RADIUS (1)
#define PER_VERTEX_RAY (1)
#define RAYCASTING (1)
#define INSTANCING (1)

#include "SpherePipeline.hlsli"


#define PsInput PsRaycastingInput


/// <summary>
/// Pass-through geometry shader for debugging purposes.
/// </summary>
/// <remarks>
/// You need to adapt the #defines to exactly the technique you want to debug!
/// </remarks>
[maxvertexcount(3)]
void Main(triangle PsInput input[3], inout TriangleStream<PsInput> triStream) {

    [unroll(3)]
    for (int i = 0; i < 3; ++i) {
        triStream.Append(input[i]);
    }

    triStream.RestartStrip();
}
