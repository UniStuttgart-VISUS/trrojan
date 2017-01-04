/// <copyright file="problem.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <vector>

#include "trrojan/stream/export.h"
#include "trrojan/stream/scalar_type.h"


namespace trrojan {
namespace stream {

    /// <summary>
    /// Provides all storage for a stream benchmark problem.
    /// </summary>
    class TRROJANSTREAM_API problem {

    public:

        typedef std::shared_ptr<problem> pointer_type;

        static const size_t default_problem_size = 2000000;

        /// <summary>
        /// Creates a new problem with the specified properties.
        /// </summary>
        problem(const trrojan::stream::scalar_type scalar
            = trrojan::stream::scalar_type::float32,
            const size_t parallelism = 1,
            const size_t size = default_problem_size);

        /// <summary>
        /// Gets the first input array.
        /// </summary>
        /// <returns></returns>
        template<class T> inline T *a(void) {
            return static_cast<T *>(static_cast<void *>(this->_a.data()));
        }

        /// <summary>
        /// Gets the second input array.
        /// </summary>
        /// <returns></returns>
        template<class T> inline T *b(void) {
            return static_cast<T *>(static_cast<void *>(this->_b.data()));
        }

        /// <summary>
        /// Gets the output array.
        /// </summary>
        /// <returns></returns>
        template<class T> inline T *c(void) {
            return static_cast<T *>(static_cast<void *>(this->_c.data()));
        }

        /// <summary>
        /// Answer for how many threads the problem is intended.
        /// </summary>
        /// <returns></returns>
        inline size_t parallelism(void) const {
            return this->_parallelism;
        }

        /// <summary>
        /// Answer the <see cref="trrojan::stream::scalar_type" /> which for the
        /// problem has been initialised.
        /// </summary>
        /// <returns></returns>
        inline trrojan::stream::scalar_type scalar_type(void) const {
            return this->_scalar_type;
        }

        /// <summary>
        /// Answer the problem size (in number of elements) for a single thread.
        /// </summary>
        /// <returns></returns>
        inline size_t size(void) const {
            assert(this->_a.size() == this->_b.size());
            assert(this->_a.size() == this->_c.size());
            assert(this->_a.size() % this->_parallelism == 0);
            return (this->_a.size() / this->_parallelism);
        }

    private:

        typedef std::vector<std::uint8_t> problem_type;

        template<trrojan::stream::scalar_type T>
        inline void allocate(size_t cnt) {
            typedef typename scalar_type_traits<T>::type type;

            if (this->_parallelism < 1) {
                this->_parallelism = 1;
            }
            if (cnt < 1) {
                cnt = 1;
            }

            cnt = cnt * this->_parallelism;
            this->_a.resize(cnt * sizeof(type));
            this->_b.resize(cnt * sizeof(type));
            this->_c.resize(cnt * sizeof(type));

            std::srand(std::time(nullptr));
            std::generate(this->a<type>(), this->a<type>() + cnt, std::rand);
            std::generate(this->b<type>(), this->b<type>() + cnt, std::rand);
        }

        /// <summary>
        /// The first input array.
        /// </summary>
        problem_type _a;

        /// <summary>
        /// The second input array.
        /// </summary>
        problem_type _b;

        /// <summary>
        /// The output array.
        /// </summary>
        problem_type _c;

        /// <summary>
        /// The number of threads the problem is for.
        /// </summary>
        size_t _parallelism;

        /// <summary>
        /// The type of a scalar.
        /// </summary>
        trrojan::stream::scalar_type _scalar_type;
    };

}
}
