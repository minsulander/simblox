// This file is part of Eigen, a lightweight C++ template library
// for linear algebra. Eigen itself is part of the KDE project.
//
// Copyright (C) 2008 Benoit Jacob <jacob@math.jussieu.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_FLAGGED_H
#define EIGEN_FLAGGED_H

/** \class Flagged
  *
  * \brief Expression with modified flags
  *
  * \param ExpressionType the type of the object of which we are modifying the flags
  * \param Added the flags added to the expression
  * \param Removed the flags removed from the expression (has priority over Added).
  *
  * This class represents an expression whose flags have been modified.
  * It is the return type of MatrixBase::flagged()
  * and most of the time this is the only way it is used.
  *
  * \sa MatrixBase::flagged()
  */
template<typename ExpressionType, unsigned int Added, unsigned int Removed>
struct ei_traits<Flagged<ExpressionType, Added, Removed> >
{
  typedef typename ExpressionType::Scalar Scalar;

  enum {
    RowsAtCompileTime = ExpressionType::RowsAtCompileTime,
    ColsAtCompileTime = ExpressionType::ColsAtCompileTime,
    MaxRowsAtCompileTime = ExpressionType::MaxRowsAtCompileTime,
    MaxColsAtCompileTime = ExpressionType::MaxColsAtCompileTime,
    Flags = (ExpressionType::Flags | Added) & ~Removed,
    CoeffReadCost = ExpressionType::CoeffReadCost
  };
};

template<typename ExpressionType, unsigned int Added, unsigned int Removed> class Flagged
  : public MatrixBase<Flagged<ExpressionType, Added, Removed> >
{
  public:

    EIGEN_GENERIC_PUBLIC_INTERFACE(Flagged)
    typedef typename ei_meta_if<ei_must_nest_by_value<ExpressionType>::ret,
        ExpressionType, const ExpressionType&>::ret ExpressionTypeNested;

    inline Flagged(const ExpressionType& matrix) : m_matrix(matrix) {}

    /** \internal */
    inline const ExpressionType& _expression() const { return m_matrix; }

  private:

    inline int _rows() const { return m_matrix.rows(); }
    inline int _cols() const { return m_matrix.cols(); }
    inline int _stride() const { return m_matrix.stride(); }

    inline const Scalar _coeff(int row, int col) const
    {
      return m_matrix.coeff(row, col);
    }

    inline Scalar& _coeffRef(int row, int col)
    {
      return m_matrix.const_cast_derived().coeffRef(row, col);
    }

    template<int LoadMode>
    inline const PacketScalar _packetCoeff(int row, int col) const
    {
      return m_matrix.template packetCoeff<LoadMode>(row, col);
    }

    template<int LoadMode>
    inline void _writePacketCoeff(int row, int col, const PacketScalar& x)
    {
      m_matrix.const_cast_derived().template writePacketCoeff<LoadMode>(row, col, x);
    }

  protected:
    ExpressionTypeNested m_matrix;
};

/** \returns an expression of *this with added flags
  *
  * Example: \include MatrixBase_marked.cpp
  * Output: \verbinclude MatrixBase_marked.out
  *
  * \sa class Flagged, extract(), part()
  */
template<typename Derived>
template<unsigned int Added>
inline const Flagged<Derived, Added, 0>
MatrixBase<Derived>::marked() const
{
  return derived();
}

/** \returns an expression of *this with the following flags removed:
  * EvalBeforeNestingBit and EvalBeforeAssigningBit.
  *
  * Example: \include MatrixBase_lazy.cpp
  * Output: \verbinclude MatrixBase_lazy.out
  *
  * \sa class Flagged, marked()
  */
template<typename Derived>
inline const Flagged<Derived, 0, EvalBeforeNestingBit | EvalBeforeAssigningBit>
MatrixBase<Derived>::lazy() const
{
  return derived();
}

#endif // EIGEN_FLAGGED_H
