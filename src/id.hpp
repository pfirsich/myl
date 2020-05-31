#include <cassert>
#include <limits>

namespace myl {

// maxUnderlyingVal is exclusive
template <typename TagType, typename UnderlyingType,
    UnderlyingType maxUnderlyingVal = std::numeric_limits<UnderlyingType>::max()>
class Id {
public:
    using Underlying = UnderlyingType;
    static constexpr Underlying maxUnderlyingValue = maxUnderlyingVal;

    static Id getNew()
    {
        static Underlying counter = 0;
        assert(counter < maxUnderlyingValue);
        return Id(counter++);
    }

    constexpr explicit Id(Underlying id)
        : id_(id)
    {
    }

    constexpr explicit operator Underlying() const
    {
        return id_;
    }

    constexpr bool operator==(Id other)
    {
        return id_ == static_cast<Underlying>(other.id_);
    }

    constexpr bool operator!=(Id other)
    {
        return id_ == static_cast<Underlying>(other.id_);
    }

    std::string toString() const
    {
        return std::to_string(id_);
    }

private:
    Underlying id_;
};

template <typename IdType>
constexpr IdType maxId()
{
    return IdType(IdType::maxUnderlyingValue);
}

// These are just here, so you can use Id in a map or something, but I don't
// want operator< and operator> for the Id itself (because it usually makes no sense)

template <typename IdType>
struct IdLess {
    constexpr bool operator()(IdType lhs, IdType rhs) const
    {
        return static_cast<typename IdType::Underlying>(lhs)
            < static_cast<typename IdType::Underlying>(rhs);
    }
};

template <typename IdType>
struct IdGreater {
    constexpr bool operator()(IdType lhs, IdType rhs) const
    {
        return static_cast<typename IdType::Underlying>(lhs)
            > static_cast<typename IdType::Underlying>(rhs);
    }
};

}
