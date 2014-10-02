// Range v3 library
//
//  Copyright Eric Niebler 2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_VIEW_REVERSE_HPP
#define RANGES_V3_VIEW_REVERSE_HPP

#include <utility>
#include <iterator>
#include <range/v3/range_fwd.hpp>
#include <range/v3/size.hpp>
#include <range/v3/begin_end.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/range_adaptor.hpp>
#include <range/v3/utility/bindable.hpp>

namespace ranges
{
    inline namespace v3
    {
        template<typename Rng>
        struct reversed_view
          : range_adaptor<reversed_view<Rng>, Rng>
        {
        private:
            CONCEPT_ASSERT(BidirectionalIterable<Rng>());
            CONCEPT_ASSERT(BoundedIterable<Rng>());
            friend range_access;

            // A rather convoluted implementation to avoid the problem std::reverse_iterator
            // has adapting iterators that return references to internal data.
            struct adaptor : adaptor_base
            {
            private:
                reversed_view const *rng_;
            public:
                adaptor() = default;
                adaptor(reversed_view const &rng)
                  : rng_(&rng)
                {}
                range_iterator_t<Rng> begin(reversed_view const &rng) const
                {
                    auto it = ranges::end(rng.base());
                    if(it != ranges::begin(rng.base()))
                        --it;
                    return it;
                }
                void next(range_iterator_t<Rng> &it) const
                {
                    if(it == ranges::begin(rng_->base()))
                        it = ranges::end(rng_->base());
                    else
                        --it;
                }
                void prev(range_iterator_t<Rng> &it) const
                {
                    if(it == ranges::end(rng_->base()))
                        it = ranges::begin(rng_->base());
                    else
                        ++it;
                }
                CONCEPT_REQUIRES(RandomAccessIterable<Rng>())
                void advance(range_iterator_t<Rng> &it, range_difference_t<Rng> n) const
                {
                    if(n > 0)
                        ranges::advance(it, -n + 1), next(it);
                    else if(n < 0)
                        prev(it), ranges::advance(it, -n - 1);
                }
                CONCEPT_REQUIRES(RandomAccessIterable<Rng>())
                range_difference_t<Rng>
                distance_to(range_iterator_t<Rng> const &here, range_iterator_t<Rng> const &there,
                    adaptor const &other_adapt) const
                {
                    RANGES_ASSERT(rng_ == other_adapt.rng_);
                    if(there == ranges::end(rng_->base()))
                        return here == ranges::end(rng_->base())
                            ? 0 : (here - ranges::begin(rng_->base())) + 1;
                    else if(here == ranges::end(rng_->base()))
                        return (ranges::begin(rng_->base()) - there) - 1;
                    return here - there;
                }
            };
            adaptor begin_adaptor() const
            {
                return {*this};
            }
            adaptor end_adaptor() const
            {
                return {*this};
            }
        public:
            reversed_view() = default;
            reversed_view(Rng && rng)
              : range_adaptor_t<reversed_view>(std::forward<Rng>(rng))
            {}
            CONCEPT_REQUIRES(SizedIterable<Rng>())
            range_size_t<Rng> size() const
            {
                return ranges::size(this->base());
            }
        };

        namespace view
        {
            struct reverse_fn : bindable<reverse_fn>, pipeable<reverse_fn>
            {
                template<typename Rng>
                static reversed_view<Rng>
                invoke(reverse_fn, Rng && rng)
                {
                    CONCEPT_ASSERT(BidirectionalIterable<Rng>());
                    CONCEPT_ASSERT(BoundedIterable<Rng>());
                    return reversed_view<Rng>{std::forward<Rng>(rng)};
                }
            };

            RANGES_CONSTEXPR reverse_fn reverse {};
        }
    }
}

#endif
