#pragma once

template <typename ItemType> class CRingBuffer
{
  ItemType *Array;
  TS32 Capacity;
  TS32 Count;

public:

	CRingBuffer(const TU32 Size = 500)
  {
    Capacity = Size;
    Array = new ItemType[ Size ];
    Count = 0;
  }

  virtual ~CRingBuffer()
  {
    SAFEDELETEA( Array );
  }

  virtual void Add( const ItemType &Item )
  {
    Array[ Count%Capacity ] = Item;
    Count++;
  }

  virtual CRingBuffer<ItemType> &operator+= ( const ItemType &i )
  {
    Add( i );
    return *this;
  }

  virtual ItemType const operator[]( const TS32 idx ) const
  {
    TS32 start = Count - Capacity;
    if ( start < 0 ) start = 0;
    TS32 realindex = start + idx;
    BASEASSERT( realindex < Count );
    return (const ItemType)Array[ realindex%Capacity ];
  }

  virtual ItemType &operator[]( const TS32 idx )
  {
    TS32 start = Count - Capacity;
    if ( start < 0 ) start = 0;
    TS32 realindex = start + idx;
    BASEASSERT( realindex < Count );
    return Array[ realindex%Capacity ];
  }

  TS32 NumItems()
  {
    if ( Capacity < Count ) return Capacity;
    return Count;
  }

  void Flush()
  {
    SAFEDELETEA( Array );
    Array = new ItemType[ Capacity ];
    Count = 0;
  }

};