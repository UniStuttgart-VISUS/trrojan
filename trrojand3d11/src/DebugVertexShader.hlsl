/// <copyright file="DebugVertexShader.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "DebugPipeline.hlsli"


/// <summary>
/// Vertex shader that creates a triangle strip for a full screen quad from the
/// vertex ID.
/// </summary>
PsInput Main(VsInput input) {
    const float2 FLIP_Y = float2(1.0f, -1.0f);  // Flips the y-axis of a float2.
    PsInput retval = (PsInput) 0;

    // Compute texture coordinates within [0, 1] from vertex ID.
    retval.TexCoords = float2(input.Id % 2, input.Id % 4 / 2);

    // Compute initial position as rectangle [-0.5, 0.5f].
    float2 pos = FLIP_Y * (retval.TexCoords - 0.5f.xx);

    //// Scale rectangle to match size of the target rectangle of the stream.
    //// After that, we are working in pixel units of the virtual display, which
    //// are the pixel units of the local window, too. At this point, the centre
    //// of the virtual display, the centre of the window and the centre of the
    //// target are the same point.
    //pos *= TargetRect.zw;

    //// The pivot point of the target rectangle is still at its centre. However,
    //// the configuration file specifies the position with the left/top corner,
    //// so we translate the rectangle accordingly. Now, the left/top corner of
    //// the target rectangle is in the centre of the local window.
    //pos += 0.5f * FLIP_Y * TargetRect.zw;

    //// Undo the local translation within the local window, ie move the origin
    //// to the left/top corner of the window.
    //pos -= 0.5f * FLIP_Y * GlobalBounds.zw;

    //// After that, account for the position of the current tile (= window) in
    //// the virtual display, ie move the origin of the local window to virtual
    //// coordinates.
    //pos -= FLIP_Y * GlobalBounds.xy;

    //// Apply the user-defined translation of the target rectangle.
    //pos += FLIP_Y * TargetRect.xy;

    //// Scale from virtual display coordinates to NDC range of [-1, 1].
    //pos /= 0.5f * GlobalBounds.zw;

    retval.Position = float4(pos.x, pos.y, 0.5f, 1.0f);
    return retval;
}
