// <copyright file="random_sphere_generator.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::random_sphere_generator::get_input_layout
 */
template<class TInputElement>
std::vector<TInputElement> trrojan::random_sphere_generator::get_input_layout(
        const sphere_type type) {
    std::vector<TInputElement> retval;

#if defined(_WIN32)
    TInputElement element;
    UINT offset = 0;

    ::memset(&element, 0, sizeof(element));
    element.SemanticName = "POSITION";
    element.AlignedByteOffset = offset;
//    element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rgba8:
        case sphere_type::pos_rgba32:
            element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            offset += 3 * sizeof(float);
            retval.push_back(element);
            break;

        case sphere_type::pos_rad_intensity:
        case sphere_type::pos_rad_rgba8:
        case sphere_type::pos_rad_rgba32:
            element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }

    ::memset(&element, 0, sizeof(element));
    element.SemanticName = "COLOR";
    element.AlignedByteOffset = offset;
    //element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rad_intensity:
            element.Format = DXGI_FORMAT_R32_FLOAT;
            offset += 1 * sizeof(float);
            retval.push_back(element);
            break;

        case sphere_type::pos_rgba8:
        case sphere_type::pos_rad_rgba8:
            element.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        case sphere_type::pos_rgba32:
        case sphere_type::pos_rad_rgba32:
            element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }
#endif /* defined(_WIN32) */

    return retval;
}
