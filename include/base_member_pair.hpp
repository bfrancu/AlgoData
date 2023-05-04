#pragma once

/*
 * C++ Templates: The Complete Guide (Vadervoorde, Josuttis, Douglas)
 * 21.1.2 Members as Base Classes
 */
namespace ts
{
template<typename Base, typename Member>
class BaseMemberPair : private Base
{
public: 
    BaseMemberPair() = default;
    BaseMemberPair(const Base & base, const Member & member) :
        Base(base),
        m_member(member)
    {}

public:
    inline Base & base() { return static_cast<Base&>(*this); }
    inline Member & member() { return m_member; }
    inline const Base & base() const { return static_cast<const Base&>(*this); }
    inline const Member & member() const { return m_member; }

private:
    Member m_member;
};
}//ts

