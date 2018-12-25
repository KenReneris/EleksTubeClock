/*
 * Smooth.h
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


template<int SIZE>
class Smooth
{
public:
    Smooth();

    void        Reset();
    void        Push( int32 v );
    int32       Value() const;
    int32       operator [](int index) const;

private:
    uint32      _size;
    int32       _history[ SIZE ];
};


template<int SIZE>
Smooth<SIZE>::Smooth()
    : _size         ( 0 )
{
}


template<int SIZE>
void
Smooth<SIZE>::Reset()
{
    _size = 0;
}


template<int SIZE>
void
Smooth<SIZE>::Push( int32 v )
{
    _history[ _size % SIZE ] = v;
    _size += 1;
}


template<int SIZE>
int32
Smooth<SIZE>::Value() const
{
    uint        max     = MIN( _size, SIZE );
    int32       accum   = 0;

    for ( uint index=0; index < max; ++index )
    {
        accum += _history[ index ];
    }

    return ( accum / max );
}


template<int SIZE>
int32  
Smooth<SIZE>::operator []( int index ) const
{
    return _history[ uint(_size-index-1) % SIZE ];
}






