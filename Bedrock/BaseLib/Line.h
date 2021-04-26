#pragma once

class CLine2D
{
public:

  CVector2 Point;
  CVector2 Direction;

  CLine2D();
  CLine2D( const CVector2 &p, const CVector2 &dir );

  void CreateFromPoints( const CVector2 &p1, const CVector2 &p2 );
  void Normalize();
  CVector2 GetPoint( TF32 t ) const;
  virtual TF32 Distance( const CVector2 &v ) const;
  CVector2 Project( const CVector2 &v ) const;
  TF32 GetProjectionT( const CVector2 &v ) const;
};


class CLine
{
public:

  CVector3 Point;
  CVector3 Direction;

  CLine();
  CLine( const CVector3 &p, const CVector3 &dir );

  void CreateFromPoints( const CVector3 &p1, const CVector3 &p2 );
  void Normalize();
  CVector3 GetPoint( TF32 t ) const;
  virtual TF32 Distance( const CVector3 &v ) const;
  CVector3 Project( const CVector3 &v ) const;
  TF32 GetProjectionT( const CVector3 &v ) const;
};

class linesegment : public CLine
{
public:

  linesegment( const CVector3 &p1, const CVector3 &p2 );
  virtual TF32 Distance( const CVector3 &v ) const;
};