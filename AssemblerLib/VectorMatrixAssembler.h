/**
 * \copyright
 * Copyright (c) 2012-2016, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#ifndef ASSEMBLERLIB_VECTORMATRIXASSEMBLER_H_
#define ASSEMBLERLIB_VECTORMATRIXASSEMBLER_H_

#include "LocalToGlobalIndexMap.h"

#include "NumLib/ODESolver/Types.h"

namespace AssemblerLib
{

// TODO doc
template<typename GlobalMatrix, typename GlobalVector,
         NumLib::ODESystemTag NLTag>
class VectorMatrixAssembler;

/// Adds result of local assembler into a global vector and a global matrix.
/// The VectorMatrixAssembler executes the local assembler for a given mesh item
/// and adds the local vector and matrix entries into the global vector and
/// the global matrix. The indices in global objects are provided by
/// the LocalToGlobalIndexMap in the construction.
template<typename GlobalMatrix, typename GlobalVector>
class VectorMatrixAssembler<GlobalMatrix, GlobalVector,
        NumLib::ODESystemTag::FirstOrderImplicitQuasilinear>
{
public:
    VectorMatrixAssembler(
        LocalToGlobalIndexMap const& data_pos)
    : _data_pos(data_pos)
    {}

    /// Executes local assembler for the given mesh item and adds the result
    /// into the global matrix and vector.
    /// The positions in the global matrix/vector are taken from
    /// the LocalToGlobalIndexMap provided in the constructor at index \c id.
    /// \attention The index \c id is not necesserily the mesh item's id.
    template <typename LocalAssembler_>
    void operator()(std::size_t const id,
        LocalAssembler_* const local_assembler,
        const double t, GlobalVector const& x,
        GlobalMatrix& M, GlobalMatrix& K, GlobalVector& b) const
    {
        assert(_data_pos.size() > id);

        std::vector<GlobalIndexType> indices;

        // Local matrices and vectors will always be ordered by component,
        // no matter what the order of the global matrix is.
        for (unsigned c=0; c<_data_pos.getNumComponents(); ++c)
        {
            auto const& idcs = _data_pos(id, c).rows;
            indices.reserve(indices.size() + idcs.size());
            indices.insert(indices.end(), idcs.begin(), idcs.end());
        }

        std::vector<double> local_x;
        local_x.reserve(indices.size());

        for (auto i : indices) {
            local_x.emplace_back(x.get(i));
        }

        LocalToGlobalIndexMap::RowColumnIndices const r_c_indices(
                    indices, indices);

        local_assembler->assemble(t, local_x);
        local_assembler->addToGlobal(r_c_indices, M, K, b);
    }

protected:
    LocalToGlobalIndexMap const& _data_pos;
};



template<typename GlobalMatrix, typename GlobalVector>
class VectorMatrixAssembler<GlobalMatrix, GlobalVector,
        NumLib::ODESystemTag::DUMMY>
{
public:
    VectorMatrixAssembler(
        LocalToGlobalIndexMap const& data_pos)
    : _data_pos(data_pos)
    {}

    /// Executes local assembler for the given mesh item and adds the result
    /// into the global matrix and vector.
    /// The positions in the global matrix/vector are taken from
    /// the LocalToGlobalIndexMap provided in the constructor at index \c id.
    /// \attention The index \c id is not necesserily the mesh item's id.
    template <typename LocalAssembler_>
    void operator()(std::size_t const id,
        LocalAssembler_* const local_assembler) const
    {
        assert(_data_pos.size() > id);

        std::vector<GlobalIndexType> indices;

        // Local matrices and vectors will always be ordered by component,
        // no matter what the order of the global matrix is.
        for (unsigned c=0; c<_data_pos.getNumComponents(); ++c)
        {
            auto const& idcs = _data_pos(id, c).rows;
            indices.reserve(indices.size() + idcs.size());
            indices.insert(indices.end(), idcs.begin(), idcs.end());
        }

        std::vector<double> localX;
        std::vector<double> localX_pts;

        if (_x)         localX.reserve(indices.size());
        if (_x_prev_ts) localX_pts.reserve(indices.size());

        for (auto i : indices)
        {
            if (_x)         localX.emplace_back(_x->get(i));
            if (_x_prev_ts) localX_pts.emplace_back(_x_prev_ts->get(i));
        }

        LocalToGlobalIndexMap::RowColumnIndices const r_c_indices(
                    indices, indices);

        /*
        local_assembler->assemble(localX, localX_pts);
        local_assembler->addToGlobal(_A, _rhs, r_c_indices);
        */
    }

protected:
    GlobalVector const *_x = nullptr;
    GlobalVector const *_x_prev_ts = nullptr;
    LocalToGlobalIndexMap const& _data_pos;
};

}   // namespace AssemblerLib

#endif  // ASSEMBLERLIB_VECTORMATRIXASSEMBLER_H_
