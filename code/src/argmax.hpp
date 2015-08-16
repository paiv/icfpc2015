
#ifndef __paiv__argmax_hpp
#define __paiv__argmax_hpp

namespace paiv {

template <class InputIterator, class Function>
InputIterator
argmax(InputIterator first, InputIterator last, Function fn)
{
    if (first == last)
        return last;

    auto r = first;
    auto x = fn(*first);
    for (InputIterator it = ++first; it != last; it++)
    {
        auto y = fn(*it);
        if (y > x)
        {
            x = y;
            r = it;
        }
    }
    return move(r);
}

}

#endif
