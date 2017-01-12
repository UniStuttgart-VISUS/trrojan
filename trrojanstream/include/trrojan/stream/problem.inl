/// <copyright file="problem.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::stream::problem::allocate
 */
template<trrojan::stream::problem::scalar_type_t T>
void trrojan::stream::problem::allocate(size_t cnt) {
    typedef typename scalar_type_traits<T>::type type;

    if (this->_parallelism < 1) {
        this->_parallelism = 1;
    }
    if (cnt < 1) {
        cnt = 1;
    }

    this->_scalar_size = sizeof(type);

    cnt = cnt * this->_parallelism;
    this->_a.resize(cnt * this->_scalar_size);
    this->_b.resize(cnt * this->_scalar_size);
    this->_c.resize(cnt * this->_scalar_size);

    std::srand(std::time(nullptr));
    std::generate(this->a<type>(), this->a<type>() + cnt, std::rand);
    std::generate(this->b<type>(), this->b<type>() + cnt, std::rand);
}
