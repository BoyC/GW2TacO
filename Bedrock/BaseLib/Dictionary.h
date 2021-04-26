#pragma once
#include "CriticalSection.h"
#include "Array.h"

#define HASHEXPANSIONTHRESHOLD 0.7f
#define HASHCOLLAPSETHRESHOLD 0.2f

template <typename KeyType, typename ItemType> class CDictionary
{
public:

  class KDPair
  {
  public:
    KeyType Key;
    TU32 Hash;
    ItemType Data;
    KDPair *Next;

    KDPair( const KeyType &k, const ItemType &i )
    {
      Key = k;
      Data = i;
      Hash = DictionaryHash( k );
    }

    KDPair( const KeyType &k )
    {
      Key = k;
      Hash = DictionaryHash( k );
    }

    virtual ~KDPair()
    {
      delete Next;
      Next = 0;
    }
  };

protected:

  TS32 ItemCount;
  TS32 TableSize;
  KDPair **HashTable;

  ItemType _dummy; //return item for out of bounds requests

  virtual void Insert( KDPair *p )
  {
    TS32 idx = p->Hash%TableSize;
    p->Next = HashTable[ idx ];
    HashTable[ idx ] = p;
  }

  virtual void ResizeTable( const TS32 NewSize )
  {
    if ( NewSize < 8 ) return; //that's small enough

    KDPair **OldTable = HashTable;
    TS32 OldSize = TableSize;

    //expand hash table
    TableSize = NewSize;
    HashTable = new KDPair*[ TableSize ];
    memset( HashTable, 0, sizeof( KDPair* )*TableSize );

    //re-add items
    for ( TS32 x = 0; x < OldSize; x++ )
    {
      while ( OldTable[ x ] )
      {
        KDPair *Next = OldTable[ x ]->Next;
        Insert( OldTable[ x ] );
        OldTable[ x ] = Next;
      }
    }

    delete[] OldTable; //no need to delete items as they have been moved
  }

  virtual KDPair *AddNew( const KeyType &Key )
  {
    TF32 Fill = ItemCount / (TF32)TableSize;
    if ( Fill >= HASHEXPANSIONTHRESHOLD )
      ResizeTable( TableSize * 2 );

    KDPair *p = new KDPair( Key );
    Insert( p );

    ItemCount++;

    return p;
  }

  virtual KDPair *Find( const KeyType &Key ) const
  {
    TU32 idx = DictionaryHash( Key ) % TableSize;

    KDPair *p = HashTable[ idx ];

    while ( p )
    {
      if ( p->Key == Key ) return p;
      p = p->Next;
    }

    return 0;
  }

public:
  typedef void( __cdecl *DICTIONARYPROCESSCALLBACK )( ItemType &a );

  CDictionary( TS32 tblsize = 8 )
  {
    TableSize = tblsize;
    ItemCount = 0;
    HashTable = new KDPair*[ TableSize ];
    memset( HashTable, 0, sizeof( KDPair* )*TableSize );
  }

  virtual ~CDictionary()
  {
    for ( TS32 x = 0; x < TableSize; x++ )
    {
      delete HashTable[ x ];
      HashTable[ x ] = 0;
    }
    delete[] HashTable;
  }

  CDictionary( const CDictionary<KeyType, ItemType> &dict )
  {
    TableSize = 8;
    ItemCount = 0;
    HashTable = new KDPair*[ TableSize ];
    memset( HashTable, 0, sizeof( KDPair* )*TableSize );

    *this += dict;
  }

  virtual ItemType &operator[]( const KeyType &Key )
  {
    KDPair *p = Find( Key );
    if ( p ) return p->Data;
    p = AddNew( Key );
    return p->Data;
  }

  ItemType &GetByKey( const KeyType &Key )
  {
    return ( *this )[ Key ];
  }

  ItemType GetExisting( const KeyType &Key ) //this can only be used on dictionaries where the ItemType is a pointer!!
  {
    KDPair *p = Find( Key );
    if ( p ) return p->Data;
    return 0;
  }

  virtual void Add( const KeyType &Key, const ItemType &Data )
  {
    ( *this )[ Key ] = Data;
  }

  virtual void Delete( const KeyType &Key )
  {
    TU32 idx = DictionaryHash( Key ) % TableSize;

    KDPair *p = HashTable[ idx ];
    KDPair *Previous = 0;

    while ( p )
    {
      KDPair *PNext = p->Next;

      if ( p->Key == Key )
      {
        if ( Previous )
          Previous->Next = p->Next;
        else
          HashTable[ idx ] = p->Next;

        p->Next = 0;
        delete p;
        ItemCount--;
      }
      else
        Previous = p;
      p = PNext;
    }

    //////////////////////////////////////////////////////////////////////////
    // The following resize functionality was originally at the beginning of 
    // the function. However that resulted in some yet to be explained 
    // crashes. Moving it to after the delete occurred fixed the issue but
    // further investigation might be required.

    TF32 Fill = ItemCount / (TF32)TableSize;
    if ( Fill < HASHCOLLAPSETHRESHOLD )
      ResizeTable( TableSize / 2 );
  }

  void Free( const KeyType &Key )
  {
    if ( HasKey( Key ) )
    {
      ItemType i = GetByKey( Key );
      if ( i ) delete i;
    }
    Delete( Key );
  }

  void FreeA( const KeyType &Key )
  {
    if ( HasKey( Key ) )
    {
      ItemType i = GetByKey( Key );
      if ( i ) delete[] i;
    }
    Delete( Key );
  }

  void FreeByIndex( const TS32 idx )
  {
    ItemType i = GetByIndex( idx );
    if ( i ) delete i;
    DeleteByIndex( idx );
  }

  void FreeAByIndex( const TS32 idx )
  {
    ItemType i = GetByIndex( idx );
    if ( i ) delete[] i;
    DeleteByIndex( idx );
  }

  void FreeAll()
  {
    for ( TS32 x = NumItems() - 1; x >= 0; x-- )
      FreeByIndex( x );
  }

  void FreeAllA()
  {
    for ( TS32 x = NumItems() - 1; x >= 0; x-- )
      FreeAByIndex( x );
  }

  virtual void DeleteByIndex( const TS32 idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );

    TS32 cntr = 0;

    for ( TS32 x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        if ( cntr == idx )
        {
          Delete( p->Key );
          return;
        }
        p = p->Next;
        cntr++;
      }
    }
  }

  TBOOL HasKey( const KeyType &Key ) const
  {
    return Find( Key ) != 0;
  }

  virtual TS32 NumItems() const
  {
    return ItemCount;
  }

  virtual ItemType &GetByIndex( TS32 idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );

    TS32 cntr = 0;

    for ( TS32 x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        if ( cntr == idx )
          return p->Data;
        p = p->Next;
        cntr++;
      }
    }

    //out of bounds, undefined behavior:
    return _dummy;
  }

  virtual KDPair *GetKDPair( TS32 idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );

    TS32 cntr = 0;

    for ( TS32 x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        if ( cntr == idx )
          return p;
        p = p->Next;
        cntr++;
      }
    }

    //out of bounds, undefined behavior:
    return 0;
  }

  virtual ItemType &GetByIndex( TS32 idx, KeyType &Key )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );

    TS32 cntr = 0;

    for ( TS32 x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        if ( cntr == idx )
        {
          Key = p->Key;
          return p->Data;
        }
        p = p->Next;
        cntr++;
      }
    }

    //out of bounds, undefined behavior:
    return _dummy;
  }

  virtual void ForEach( const DICTIONARYPROCESSCALLBACK Callback ) const
  {
    for ( TS32 x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        Callback( p->Data );
        p = p->Next;
      }
    }
  }

  virtual void Flush()
  {
    for ( TS32 x = 0; x < TableSize; x++ )
    {
      delete HashTable[ x ];
      HashTable[ x ] = 0;
    }
    ItemCount = 0;
  }

  virtual CDictionary<KeyType, ItemType> &operator+= ( const CDictionary<KeyType, ItemType> &i )
  {
    for ( TS32 x = 0; x < i.TableSize; x++ )
    {
      KDPair *p = i.HashTable[ x ];
      while ( p )
      {
        ( *this )[ p->Key ] = p->Data;
        p = p->Next;
      }
    }
    return *this;
  }

  CDictionary<KeyType, ItemType> operator+( const CDictionary<KeyType, ItemType> &d1 ) const
  {
    CDictionary<KeyType, ItemType> dr;
    dr += *this;
    dr += d1;
    return dr;
  }

  const CDictionary<KeyType, ItemType> &operator=( const CDictionary<KeyType, ItemType> &dict )
  {
    if ( &dict == this ) return *this;
    Flush();
    ( *this ) += dict;
    return *this;
  }

};

template <typename KeyType, typename ItemType> class CDictionaryThreadSafe
{
  LIGHTWEIGHT_CRITICALSECTION critsec;
  CDictionary<KeyType, ItemType> Dictionary;

public:

  typedef void( __cdecl *DICTIONARYPROCESSCALLBACK )( ItemType &a );

  CDictionaryThreadSafe( TS32 tblsize = 8 )
  {
    InitializeLightweightCS( &critsec );
  }

  virtual ~CDictionaryThreadSafe()
  {
  }

  CDictionaryThreadSafe( const CDictionary<KeyType, ItemType> &dict )
  {
    InitializeLightweightCS( &critsec );
  }

  virtual ItemType &operator[]( const KeyType &Key )
  {
    CLightweightCriticalSection cs( &critsec );
    return Dictionary[ Key ];
  }

  ItemType &GetByKey( const KeyType &Key )
  {
    CLightweightCriticalSection cs( &critsec );
    return Dictionary[ Key ];
  }

  ItemType GetExisting( const KeyType &Key ) //this can only be used on dictionaries where the ItemType is a pointer!!
  {
    CLightweightCriticalSection cs( &critsec );
    return Dictionary.GetExisting( Key );
  }

  virtual void Add( const KeyType &Key, const ItemType &Data )
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.Add( Key, Data );
  }

  virtual void Delete( const KeyType &Key )
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.Delete( Key );
  }

  void Free( const KeyType &Key )
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.Free( Key );
  }

  void FreeA( const KeyType &Key )
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.FreeA( Key );
  }

  void FreeByIndex( const TS32 idx )
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.FreeByIndex( idx );
  }

  void FreeAByIndex( const TS32 idx )
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.FreeAByIndex( idx );
  }

  void FreeAll()
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.FreeAll();
  }

  void FreeAllA()
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.FreeAllA();
  }

  virtual void DeleteByIndex( const TS32 idx )
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.DeleteByIndex( idx );
  }

  TBOOL HasKey( const KeyType &Key )
  {
    CLightweightCriticalSection cs( &critsec );
    return Dictionary.HasKey( Key );
  }

  virtual TS32 NumItems()
  {
    CLightweightCriticalSection cs( &critsec );
    return Dictionary.NumItems();
  }

  virtual ItemType &GetByIndex( TS32 idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Dictionary.GetByIndex( idx );
  }

  virtual ItemType &GetByIndex( TS32 idx, KeyType &Key )
  {
    CLightweightCriticalSection cs( &critsec );
    return Dictionary.GetByIndex( idx, Key );
  }

  virtual void ForEach( const DICTIONARYPROCESSCALLBACK Callback )
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.ForEach( Callback );
  }

  virtual void Flush()
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary.Flush();
  }

  virtual CDictionaryThreadSafe<KeyType, ItemType> &operator+= ( const CDictionaryThreadSafe<KeyType, ItemType> &i )
  {
    CLightweightCriticalSection cs( &critsec );
    Dictionary += i.Dictionary;
    return *this;
  }

  CDictionaryThreadSafe<KeyType, ItemType> operator+( const CDictionaryThreadSafe<KeyType, ItemType> &d1 )
  {
    CLightweightCriticalSection cs( &critsec );
    CDictionaryThreadSafe<KeyType, ItemType> dr;
    dr += *this;
    dr += d1;
    return dr;
  }

  const CDictionaryThreadSafe<KeyType, ItemType> &operator=( const CDictionaryThreadSafe<KeyType, ItemType> &dict )
  {
    CLightweightCriticalSection cs( &critsec );
    if ( &dict == this ) return *this;
    Flush();
    ( *this ) += dict;
    return *this;
  }

};


template <typename KeyType, typename ItemType> class CDictionaryEnumerable : public CDictionary<KeyType, ItemType>
{

#define SORTSTACKSIZE (8*sizeof(void*) - 2)

public:
  typedef TS32( *KEYSORTCALLBACK )( const KeyType &a, const KeyType &b );
  typedef TS32( *VALUESORTCALLBACK )( const ItemType &a, const ItemType &b );

private:

  CArray<KDPair*> IndexMap;

  void swap( TS32 a, TS32 b )
  {
    KDPair *k = IndexMap[ a ];
    IndexMap[ a ] = IndexMap[ b ];
    IndexMap[ b ] = k;
  }

  virtual KDPair *AddNew( const KeyType &Key )
  {
    KDPair *p = CDictionary<KeyType, ItemType>::AddNew( Key );
    IndexMap.Add( p );
    return p;
  }

public:

  CDictionaryEnumerable() : CDictionary<KeyType, ItemType>()
  {
  }


  CDictionaryEnumerable( const CDictionaryEnumerable<KeyType, ItemType> &dict )
  {
    *this += dict;
  }

  virtual void Add( const KeyType &Key, const ItemType &Data )
  {
    KDPair *p = Find( Key );
    CDictionary<KeyType, ItemType>::Add( Key, Data );
    if ( !p )
    {
      p = Find( Key );
      IndexMap.Add( p );
    }
  }

  virtual void Delete( const KeyType &Key )
  {
    KDPair *p = Find( Key );
    CDictionary<KeyType, ItemType>::Delete( Key );
    if ( p )
      IndexMap.Delete( p );
  }

  virtual ItemType &GetByIndex( TS32 idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );
    return ( (KDPair*)IndexMap[ idx ] )->Data;
  }

  virtual void DeleteByIndex( const TS32 idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );
    Delete( IndexMap[ idx ]->Key );
  }

  virtual ItemType &GetByIndex( TS32 idx, KeyType &Key )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );
    Key = ( (KDPair*)IndexMap[ idx ] )->Key;
    return ( (KDPair*)IndexMap[ idx ] )->Data;
  }

  virtual KDPair* GetKDPairByIndex( TS32 idx )
  {
    return (KDPair*)IndexMap[ idx ];
  }

  virtual void SortByKey( KEYSORTCALLBACK SortCallback )
  {

    if ( !SortCallback ) return;
    if ( ItemCount < 2 ) return;

    //qsort implementation - calling crt qsort isn't viable here due to template hackery
    //implementation taken from crt

    TU32 lostk[ SORTSTACKSIZE ], histk[ SORTSTACKSIZE ];
    TS32 stkptr = 0;

    TU32 lo = 0;
    TU32 hi = ItemCount - 1;

  recurse:

    TU32 size = ( hi - lo ) + 1;

    if ( size <= 8 ) //cutoff = 8
    {
      TU32 _hi = hi, _lo = lo;

      while ( _hi > _lo )
      {
        TU32 max = _lo;
        for ( TU32 p = _lo + 1; p <= _hi; p++ )
          if ( SortCallback( IndexMap[ p ]->Key, IndexMap[ max ]->Key ) > 0 )
            max = p;
        swap( max, _hi );
        _hi--;
      }
    }
    else
    {
      TU32 mid = lo + ( size / 2 );

      if ( SortCallback( IndexMap[ lo ]->Key, IndexMap[ mid ]->Key ) > 0 ) swap( lo, mid );
      if ( SortCallback( IndexMap[ lo ]->Key, IndexMap[ hi ]->Key ) > 0 ) swap( lo, hi );
      if ( SortCallback( IndexMap[ mid ]->Key, IndexMap[ hi ]->Key ) > 0 ) swap( mid, hi );

      TU32 loguy = lo;
      TU32 higuy = hi;

      for ( ;;)
      {
        if ( mid > loguy ) do { loguy++; } while ( loguy < mid && SortCallback( IndexMap[ loguy ]->Key, IndexMap[ mid ]->Key ) <= 0 );
        if ( mid <= loguy ) do { loguy++; } while ( loguy <= hi && SortCallback( IndexMap[ loguy ]->Key, IndexMap[ mid ]->Key ) <= 0 );
        do { higuy--; } while ( higuy > mid && SortCallback( IndexMap[ higuy ]->Key, IndexMap[ mid ]->Key ) > 0 );
        if ( higuy < loguy ) break;
        swap( loguy, higuy );
        if ( mid == higuy ) mid = loguy;
      }

      higuy++;

      if ( mid < higuy ) do { higuy--; } while ( higuy > mid && SortCallback( IndexMap[ higuy ]->Key, IndexMap[ mid ]->Key ) == 0 );
      if ( mid >= higuy ) do { higuy--; } while ( higuy > lo && SortCallback( IndexMap[ higuy ]->Key, IndexMap[ mid ]->Key ) == 0 );

      if ( higuy - lo >= hi - loguy )
      {
        if ( lo < higuy ) { lostk[ stkptr ] = lo; histk[ stkptr ] = higuy; ++stkptr; }
        if ( loguy < hi ) { lo = loguy; goto recurse; }
      }
      else
      {
        if ( loguy < hi ) { lostk[ stkptr ] = loguy; histk[ stkptr ] = hi; ++stkptr; }
        if ( lo < higuy ) { hi = higuy; goto recurse; }
      }
    }

    --stkptr;
    if ( stkptr >= 0 ) { lo = lostk[ stkptr ]; hi = histk[ stkptr ]; goto recurse; }

  }

  virtual void SortByValue( VALUESORTCALLBACK SortCallback )
  {
    if ( !SortCallback ) return;

    //qsort implementation - calling crt qsort isn't viable here due to template hackery
    //implementation taken from crt

    TU32 lostk[ SORTSTACKSIZE ], histk[ SORTSTACKSIZE ];
    TS32 stkptr = 0;

    TU32 lo = 0;
    TU32 hi = ItemCount - 1;

  recurse:

    TU32 size = ( hi - lo ) + 1;

    if ( size <= 8 ) //cutoff = 8
    {
      TU32 _hi = hi, _lo = lo;

      while ( _hi > _lo )
      {
        TU32 max = _lo;
        for ( TU32 p = _lo + 1; p <= _hi; p++ )
          if ( SortCallback( IndexMap[ p ]->Data, IndexMap[ max ]->Data ) > 0 )
            max = p;
        swap( max, _hi );
        _hi--;
      }
    }
    else
    {
      TU32 mid = lo + ( size / 2 );

      if ( SortCallback( IndexMap[ lo ]->Data, IndexMap[ mid ]->Data ) > 0 ) swap( lo, mid );
      if ( SortCallback( IndexMap[ lo ]->Data, IndexMap[ hi ]->Data ) > 0 ) swap( lo, hi );
      if ( SortCallback( IndexMap[ mid ]->Data, IndexMap[ hi ]->Data ) > 0 ) swap( mid, hi );

      TU32 loguy = lo;
      TU32 higuy = hi;

      for ( ;;)
      {
        if ( mid > loguy ) do { loguy++; } while ( loguy < mid && SortCallback( IndexMap[ loguy ]->Data, IndexMap[ mid ]->Data ) <= 0 );
        if ( mid <= loguy ) do { loguy++; } while ( loguy <= hi && SortCallback( IndexMap[ loguy ]->Data, IndexMap[ mid ]->Data ) <= 0 );
        do { higuy--; } while ( higuy > mid && SortCallback( IndexMap[ higuy ]->Data, IndexMap[ mid ]->Data ) > 0 );
        if ( higuy < loguy ) break;
        swap( loguy, higuy );
        if ( mid == higuy ) mid = loguy;
      }

      higuy++;

      if ( mid < higuy ) do { higuy--; } while ( higuy > mid && SortCallback( IndexMap[ higuy ]->Data, IndexMap[ mid ]->Data ) == 0 );
      if ( mid >= higuy ) do { higuy--; } while ( higuy > lo && SortCallback( IndexMap[ higuy ]->Data, IndexMap[ mid ]->Data ) == 0 );

      if ( higuy - lo >= hi - loguy )
      {
        if ( lo < higuy ) { lostk[ stkptr ] = lo; histk[ stkptr ] = higuy; ++stkptr; }
        if ( loguy < hi ) { lo = loguy; goto recurse; }
      }
      else
      {
        if ( loguy < hi ) { lostk[ stkptr ] = loguy; histk[ stkptr ] = hi; ++stkptr; }
        if ( lo < higuy ) { hi = higuy; goto recurse; }
      }
    }

    --stkptr;
    if ( stkptr >= 0 ) { lo = lostk[ stkptr ]; hi = histk[ stkptr ]; goto recurse; }

  }

  virtual void Flush()
  {
    CDictionary<KeyType, ItemType>::Flush();
    IndexMap.Flush();
  }

  virtual CDictionaryEnumerable<KeyType, ItemType> &operator+= ( const CDictionaryEnumerable<KeyType, ItemType> &i )
  {
    for ( TS32 x = 0; x < i.NumItems(); x++ )
    {
      KDPair *p = i.IndexMap[ x ];
      ( *this )[ p->Key ] = p->Data;
    }
    return *this;
  }

  CDictionaryEnumerable<KeyType, ItemType> operator+( const CDictionaryEnumerable<KeyType, ItemType> &d1 ) const
  {
    CDictionaryEnumerable<KeyType, ItemType> dr;
    dr += *this;
    dr += d1;
    return dr;
  }

  const CDictionaryEnumerable<KeyType, ItemType> &operator=( const CDictionaryEnumerable<KeyType, ItemType> &dict )
  {
    Flush();
    ( *this ) += dict;
    return *this;
  }
};


TU32 DictionaryHash( const TS32 &i );
TU32 DictionaryHash( const void *i );