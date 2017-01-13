/// <copyright file="io.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 *  trrojan::get_file_system_entries
 */
template<class I, class P>
void TRROJANCORE_API trrojan::get_file_system_entries(I oit,
        const std::string& path, const bool isRecursive, P predicate) {
    typedef std::string string_type;
    typedef std::stack<string_type> stack_type;

    stack_type stack;
    stack.push(path);

#ifdef _WIN32
    WIN32_FIND_DATAA fd;

    while (!stack.empty()) {
        auto cur = stack.top();
        stack.pop();

        auto query = combine_path(cur, "*");
        auto hFind = ::FindFirstFileA(query.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) {
            std::error_code ec(::GetLastError(), std::system_category());
            throw std::system_error(ec, "FindFirstFile failed.");
        }

        do {
            if ((current_directory_name != fd.cFileName)
                    && (parent_directory_name != fd.cFileName)) {
                auto fn = combine_path(cur, fd.cFileName);
                if (isRecursive && ((fd.dwFileAttributes
                        & FILE_ATTRIBUTE_DIRECTORY) != 0)) {
                    stack.push(fn);
                }
                if (predicate(fd)) {
                    *oit++ = fn;
                }
            }
        } while (::FindNextFileA(hFind, &fd) != 0);

        {
            auto e = ::GetLastError();
            if (e != ERROR_NO_MORE_FILES) {
                std::error_code ec(e, std::system_category());
                throw std::system_error(ec, "FindNextFile failed.");
            }
        }
    }

#else /* _WIN32 */
    struct dirent *fd;

    while (!stack.empty()) {
        auto cur = stack.top();
        stack.pop();

        auto dir = ::opendir(cur.c_str());
        if (dir == nullptr) {
            std::error_code ec(errno, std::system_category());
            throw std::system_error(ec, "opendir failed.");
        }

        while ((fd = ::readdir(dir)) != nullptr) {
            if ((current_directory_name != fd->d_name)
                    && (parent_directory_name != fd->d_name)) {
                auto fn = combine_path(cur, fd->d_name);
                if (isRecursive && (fd->d_type == DT_DIR)) {
                    stack.push(fn);
                }
                if (predicate(*fd)) {
                    *oit++ = fn;
                }
            }
        }

        ::closedir(dir);
    }
#endif /* _WIN32 */
}
