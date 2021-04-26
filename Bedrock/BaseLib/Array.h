#pragma once
#include "CriticalSection.h"
#include "Assert.h"

//dynamic array class

#define EXPANSIONRATIO 0.2f

template <typename ItemType> class CArray
{
protected:

  TS32 Capacity;
  TS32 ItemCount;
  ItemType *Array;

  TS32 GetCapacity() const
  {
    return Capacity;
  }

public:
  typedef TS32( __cdecl *ARRAYSORTCALLBACK )( ItemType *a, ItemType *b );

  CArray()
  {
    Capacity = 0;
    ItemCount = 0;
    Array = 0;
  }

  CArray( TS32 Size )
  {
    Array = new ItemType[ Size ];
    Capacity = Size;
    ItemCount = 0;
  }

  CArray( const CArray<ItemType> &original )
  {
    ItemCount = Capacity = original.NumItems();
    Array = new ItemType[ ItemCount ];
    for ( TS32 x = 0; x < ItemCount; x++ )
      Array[ x ] = original[ x ];
  }

  ~CArray()
  {
    if ( Array ) delete[] Array;
    Capacity = ItemCount = 0;
    Array = 0;
  }

  TS32 NumItems() const
  {
    return ItemCount;
  }

  void Flush()
  {
    if ( Array ) delete[] Array;
    Array = new ItemType[ Capacity ];
    ItemCount = 0;
  }

  void FlushFast()
  {
    ItemCount = 0;
  }

  void Add( const ItemType &Item )
  {
    if ( ItemCount == Capacity || !Array ) Expand( (TS32)( Capacity*EXPANSIONRATIO + 1 ) );
    Array[ ItemCount ] = Item;
    ItemCount++;
  }

  void InsertFirst( const ItemType &Item )
  {
    Add( Item );
    for ( int x = ItemCount - 2; x >= 0; x-- )
      Array[ x + 1 ] = Array[ x ];
    Array[ 0 ] = Item;
  }

  void AllocateNewUninitialized( const TS32 Count )
  {
    if ( ItemCount + Count > Capacity || !Array ) Expand( (TS32)( Capacity*EXPANSIONRATIO + Count ) );
    ItemCount += Count;
  }

  CArray<ItemType> &operator= ( const CArray<ItemType> &a )
  {
    if ( &a == this ) return *this;
    Flush();
    for ( int i = 0; i < a.NumItems(); i++ )
      Add( a[ i ] );
    return *this;
  }

  CArray<ItemType> &operator+= ( const ItemType &i )
  {
    Add( i );
    return *this;
  }

  CArray<ItemType> &operator+= ( const CArray<ItemType> &i )
  {
    for ( TS32 x = 0; x < i.NumItems(); x++ )
      Add( i[ x ] );
    return *this;
  }

  CArray<ItemType> &operator-= ( const ItemType &i )
  {
    Delete( i );
    return *this;
  }

  TS32 AddUnique( const ItemType &Item )
  {
    auto idx = Find( Item );
    if ( idx != -1 ) return idx;
    Add( Item );
    return ItemCount - 1;
  }

  ItemType const operator[]( const TS32 idx ) const
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );
    return (const ItemType)Array[ idx ];
  }

  ItemType &operator[]( const TS32 idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );
    return Array[ idx ];
  }

  ItemType &Last()
  {
    BASEASSERT( ItemCount > 0 );
    return Array[ ItemCount - 1 ];
  }

  TS32 const Find( const ItemType &i ) const
  {
    for ( TS32 x = 0; x < ItemCount; x++ )
      if ( Array[ x ] == i ) return x;
    return -1;
  }

  void DeleteByIndex( const TS32 idx )
  {
    if ( idx < 0 || idx >= ItemCount ) return;
    ItemCount--;
    for ( TS32 x = idx; x < ItemCount; x++ )
      Array[ x ] = Array[ x + 1 ];
  }

  void Delete( const ItemType &i )
  {
    DeleteByIndex( Find( i ) );
  }

  void FreeByIndex( const TS32 idx )
  {
    if ( idx < 0 || idx >= ItemCount ) return;
    delete Array[ idx ];
    ItemCount--;
    for ( TS32 x = idx; x < ItemCount; x++ )
      Array[ x ] = Array[ x + 1 ];
  }

  void Free( const ItemType &i )
  {
    FreeByIndex( Find( i ) );
  }

  void FreeAByIndex( const TS32 idx )
  {
    if ( idx < 0 || idx >= ItemCount ) return;
    delete[] Array[ idx ];
    ItemCount--;
    for ( TS32 x = idx; x < ItemCount; x++ )
      Array[ x ] = Array[ x + 1 ];
  }

  void FreeA( const ItemType &i )
  {
    FreeAByIndex( Find( i ) );
  }

  void Swap( const TS32 a, const TS32 b )
  {
    ItemType temp = Array[ a ];
    Array[ a ] = Array[ b ];
    Array[ b ] = temp;
  }

  void Sort( ARRAYSORTCALLBACK SortFunct )
  {
    qsort( Array, ItemCount, sizeof( ItemType ), ( TS32( _cdecl* )( const void*, const void* ) )SortFunct );
  }

  ItemType *GetPointer( const TS32 idx ) const
  {
    return &Array[ idx ];
  }

  void FreeArray()
  {
    for ( TS32 x = NumItems() - 1; x >= 0; x-- )
      if ( Array[ x ] )
        delete Array[ x ];
    FlushFast();
  }

  void FreeArrayA()
  {
    for ( TS32 x = NumItems() - 1; x >= 0; x-- )
      if ( Array[ x ] )
        delete[] Array[ x ];
    FlushFast();
  }

  void TrimHead( TS32 count )
  {
    if ( count < 0 ) return;

    if ( count >= ItemCount )
    {
      FlushFast();
      return;
    }

    for ( TS32 x = 0; x < ItemCount - count; x++ )
      Array[ x ] = Array[ x + count ];
    ItemCount -= count;
  }

  void Expand( TU32 AddedItemCount )
  {
    if ( !Array )
    {
      Array = new ItemType[ AddedItemCount ];
      Capacity = AddedItemCount;
      ItemCount = 0;
      return;
    }

    ItemType *NewArray = new ItemType[ ItemCount + AddedItemCount ];
    for ( TS32 x = 0; x < ItemCount; x++ )
      NewArray[ x ] = Array[ x ];

    delete[] Array;
    Array = NewArray;
    Capacity = ItemCount + AddedItemCount;
  }
};

template<typename ItemType> __inline void SimulateAddItem( ItemType *&dataArray, TS32 &numItems, ItemType newItem )
{
  if ( !dataArray )
  {
    dataArray = new ItemType[ 1 ];
    dataArray[ 0 ] = newItem;
    numItems++;
    return;
  }

  ItemType *n = new ItemType[ numItems + 1 ];
  for ( TS32 x = 0; x < numItems; x++ )
    n[ x ] = dataArray[ x ];
  n[ numItems ] = newItem;

  if ( dataArray )
    delete[] dataArray;

  dataArray = n;

  numItems++;
  return;
}

template<typename ItemType> __inline void SimulateDeleteByIndex( ItemType *&dataArray, TS32 &numItems, TS32 index )
{
  if ( !dataArray )
    return;
  if ( index < 0 || index >= numItems )
    return;

  numItems--;
  for ( int x = index; x < numItems; x++ )
    dataArray[ x ] = dataArray[ x + 1 ];

  return;
}

template<typename ItemType> __inline void SimulateFreeArray( ItemType *&dataArray, TS32 &numItems )
{
  if ( !dataArray )
    return;

  for ( TS32 x = numItems - 1; x >= 0; x-- )
    if ( dataArray[ x ] )
      delete dataArray[ x ];

  return;
}


template <typename ItemType> class CArrayThreadSafe
{
  LIGHTWEIGHT_CRITICALSECTION critsec;
  CArray<ItemType> Array;

protected:

  TS32 GetCapacity()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.GetCapacity();
  }

public:
  typedef TS32( __cdecl *ARRAYSORTCALLBACK )( ItemType *a, ItemType *b );

	CArrayThreadSafe() //: CArray<ItemType>()
  {
    InitializeLightweightCS( &critsec );
  }

	CArrayThreadSafe(TS32 Size) //: CArray<ItemType>(Size)
  {
    InitializeLightweightCS( &critsec );
    Array.Expand( Size );
  }

  CArrayThreadSafe( const CArray<ItemType> &original ) : CArray<ItemType>( original )
  {
    InitializeLightweightCS( &critsec );
  }

  CArrayThreadSafe( const CArrayThreadSafe<ItemType> &original ) : CArray<ItemType>( original )
  {
    InitializeLightweightCS( &critsec );
  }

  ~CArrayThreadSafe()
  {
  }

  TS32 NumItems()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.NumItems();
  }

  void Flush()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Flush();
  }

  void FlushFast()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.FlushFast();
  }

  void Add( const ItemType &Item )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Add( Item );
  }

  CArrayThreadSafe<ItemType> &operator= ( const CArrayThreadSafe<ItemType> &a )
  {
    for ( int i = 0; i < a.NumItems(); i++ )
      Add( a[ i ] );
    return *this;
  }

  CArrayThreadSafe<ItemType> &operator+= ( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.operator +=( i );
    return *this;
  }

  CArrayThreadSafe<ItemType> &operator+= ( const CArray<ItemType> &i )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.operator +=( i );
    return *this;
  }

  CArrayThreadSafe<ItemType> &operator-= ( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.operator -=( i );
    return *this;
  }

  TS32 AddUnique( const ItemType &Item )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.AddUnique( Item );
  }

  ItemType &operator[]( const TS32 idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.operator[]( idx );
  }

  ItemType &Last()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Last();
  }

  TS32 const Find( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Find( i );
  }

  void DeleteByIndex( const TS32 idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.DeleteByIndex( idx );
  }

  void Delete( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Delete( i );
  }

  void FreeByIndex( const TS32 idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.FreeByIndex( idx );
  }

  void Free( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Free( i );
  }

  void FreeAByIndex( const TS32 idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.FreeAByIndex( idx );
  }

  void FreeA( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.FreeA( i );
  }

  void Swap( const TS32 a, const TS32 b )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.Swap( a, b );
  }

  void Sort( ARRAYSORTCALLBACK SortFunct )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.Sort( SortFunct );
  }

  ItemType *GetPointer( const TS32 idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.GetPointer( idx );
  }

  void FreeArray()
  {
    CLightweightCriticalSection cs( &critsec );
    FreeArray();
  }

  void FreeArrayA()
  {
    CLightweightCriticalSection cs( &critsec );
    FreeArrayA();
  }

  void Expand( TS32 size )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.Expand( size );
  }
};
