/// <copyright file="excel_output.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


#ifdef _WIN32
/*
 * trrojan::excel_output::invoke
 */
template<class... P>
void trrojan::excel_output::invoke(VARIANT *outResult, const int automationType,
        IDispatch *dispatch, LPOLESTR name, P&&... params) {
    DISPPARAMS dp = { nullptr, nullptr, 0, 0 }; // Dispatch parameters.
    DISPID dispidNamed = DISPID_PROPERTYPUT;    // Input for property-puts.
    DISPID dispID = DISPID_UNKNOWN;             // The dispatch ID.
    HRESULT hr = S_OK;                          // Return value.

    assert(dispatch != nullptr);

    /* Initialise variable argument list. */
    /* Get DISPID for name input. */
    hr = dispatch->GetIDsOfNames(IID_NULL, &name, 1, 0x0407, //LOCALE_USER_DEFAULT,
        &dispID);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to get dispatch ID: " << hr << std::ends;
        throw std::runtime_error(msg.str());
    }

    /* Allocate memory for arguments. */
    std::vector<VARIANT> args = { params... };

    /* Build DISPPARAMS. */
    dp.cArgs = static_cast<UINT>(sizeof...(params));
    dp.rgvarg = args.data();

    /* Handle special-case for property-puts. */
    if ((automationType & DISPATCH_PROPERTYPUT) != NULL) {
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &dispidNamed;
    }

    /* Make the call. */
    hr = dispatch->Invoke(dispID, IID_NULL, 0x0407, //LOCALE_SYSTEM_DEFAULT, 
        automationType, &dp, outResult, nullptr, nullptr);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to invoke dispatch interface: " << hr << std::ends;
        throw std::runtime_error(msg.str());
    }
}
#endif /* _WIN32 */
