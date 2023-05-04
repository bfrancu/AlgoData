#pragma once

template<typename Derived, typename Value, typename Category,
         typename Reference = Value&, typename Distance = std::ptrdiff_t>
class iterator_facade
{
public:
    using value_type = Value;//typename std::decay_t<Value>::type;
    using reference = Reference;
    using pointer = Value*; 
    using difference_type = Distance;
    using iterator_category = Category;

    // input iterator interface:
    reference operator *() const;
    pointer operator->() const;
    Derived & operator++();
    Derived operator++(int);

    friend bool operator==(const iterator_facade & lhs, const iterator_facade & rhs) {
        return lhs.equalTo(rhs);
    }

    friend bool operator !=(const iterator_facade & lhs, const iterator_facade & rhs) { return !(lhs == rhs); }

    // bidirectional iterator interface
    Derived & operator --();
    Derived operator --(int);

    // random access iterator interface
    reference operator [](difference_type n) const;
    Derived & operator+=(difference_type n);
    Derived operator+(difference_type n) const;

    difference_type operator-(const iterator_facade & rhs) const {
        return asDerived().measureDistance(rhs.asDerived());
    }

    friend bool operator<(const iterator_facade & lhs, const iterator_facade & rhs) {
        return 0 > lhs.operator-(rhs);
    }

    friend bool operator>(const iterator_facade & lhs, const iterator_facade & rhs) { 
        return 0 < lhs.operator-(rhs);
    }

    friend bool operator>=(const iterator_facade & lhs, const iterator_facade & rhs) { 
        return lhs > rhs || lhs == rhs;
    }

protected:
    pointer currentValueAddress() const { return std::addressof(asDerived().dereference()); }
    Derived & asDerived() { return *static_cast<Derived*>(this); }
    const Derived & asDerived() const { return *static_cast<const Derived*>(this); }

private:
    bool equalTo(const iterator_facade & other) const
    {
        return asDerived().equals(other.asDerived());
    }
};

template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
typename iterator_facade<Derived, Value, Category, Reference, Distance>::reference
iterator_facade<Derived, Value, Category, Reference, Distance>::operator*() const{
    return asDerived().dereference();
}

template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
typename iterator_facade<Derived, Value, Category, Reference, Distance>::pointer
iterator_facade<Derived, Value, Category, Reference, Distance>::operator->() const{
    return currentValueAddress();
}

template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
Derived & iterator_facade<Derived, Value, Category, Reference, Distance>::operator++() {
    asDerived().increment();
    return asDerived();
}

template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
Derived iterator_facade<Derived, Value, Category, Reference, Distance>::operator++(int) {
    Derived result(asDerived());
    asDerived().increment();
    return result;
}

/*
template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
bool operator==(const iterator_facade & lhs, const iterator_facade & rhs) {
    return lhs.asDerived().equals(rhs.asDerived());
}
*/

template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
Derived & iterator_facade<Derived, Value, Category, Reference, Distance>::operator--() {
    asDerived().decrement();
    return asDerived();
}

template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
Derived iterator_facade<Derived, Value, Category, Reference, Distance>::operator--(int) {
    Derived result(asDerived());
    asDerived().decrement();
    return result;
}

template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
typename iterator_facade<Derived, Value, Category, Reference, Distance>::reference
iterator_facade<Derived, Value, Category, Reference, Distance>::operator[](difference_type n) const {
    Derived iter = operator+(n);
    return iter.dereference();
}

template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
Derived iterator_facade<Derived, Value, Category, Reference, Distance>::operator+(difference_type n) const {
    Derived result(asDerived());
    result.advance(n);
    return result;
}

template<typename Derived, typename Value, typename Category, typename Reference, typename Distance>
Derived & iterator_facade<Derived, Value, Category, Reference, Distance>::operator+=(difference_type n) {
    asDerived().advance(n);
    return operator*();
}
