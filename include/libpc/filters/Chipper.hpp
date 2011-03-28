/******************************************************************************
 * $Id$
 *
 * Project:  libPC - http://libpc.org - A BSD library for point cloud data.
 * Purpose:  libPC chipper class
 * Author:   Howard Butler, hobu.inc@gmail.com
 *
 ******************************************************************************
 * Copyright (c) 2011, Howard Butler
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following
 * conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of the Martin Isenburg or Iowa Department
 *       of Natural Resources nor the names of its contributors may be
 *       used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 ****************************************************************************/

#ifndef LIBPC_CHIPPER_H
#define LIBPC_CHIPPER_H

#include <libpc/libpc.hpp>

#include <vector>

#include <libpc/Filter.hpp>
#include <libpc/Bounds.hpp>
#include <libpc/PointBuffer.hpp>

namespace libpc
{

class Stage;

namespace filters

{



class LIBPC_DLL Chipper;

namespace chipper {

enum Direction
{
    DIR_X,
    DIR_Y,
    DIR_NONE
};

class LIBPC_DLL PtRef
{
public:
    double m_pos;
    boost::uint32_t m_ptindex;
    boost::uint32_t m_oindex;

    bool operator < (const PtRef& pt) const
        { return m_pos < pt.m_pos; }
};

struct LIBPC_DLL RefList
{
public:
    std::vector<PtRef> m_vec;
    Direction m_dir;

    RefList(Direction dir = DIR_NONE) : m_dir(dir)
        {}
    std::vector<PtRef>::size_type size() const
        { return m_vec.size(); }
    void reserve(std::vector<PtRef>::size_type n)
        { m_vec.reserve(n); }
    void resize(std::vector<PtRef>::size_type n)
        { m_vec.resize(n); }
    void push_back(const PtRef& ref)
        { m_vec.push_back(ref); }
    std::vector<PtRef>::iterator begin()
        { return m_vec.begin(); }
    std::vector<PtRef>::iterator end()
        { return m_vec.end(); }
    PtRef& operator[](boost::uint32_t pos)
        { return m_vec[pos]; }
    std::string Dir()
    {
        if (m_dir == DIR_X)
            return "X";
        else if (m_dir == DIR_Y)
            return "Y";
        else
            return "NONE";
    }
};


 
class LIBPC_DLL Block
{
    friend class libpc::filters::Chipper;

private:
    RefList *m_list_p;
    boost::uint32_t m_left;
    boost::uint32_t m_right;
    libpc::Bounds<double> m_bounds;
    // double m_xmin;
    // double m_ymin;
    // double m_xmax;
    // double m_ymax;

public:
    std::vector<boost::uint32_t> GetIDs() const; 
    libpc::Bounds<double> const& GetBounds() const {return m_bounds;} 
    void SetBounds(libpc::Bounds<double> const& bounds) {m_bounds = bounds;}
    void GetBuffer( Stage const& stage, PointBuffer& buffer, boost::uint32_t block_id) const;    
    // double GetXmin() const
    //     { return m_xmin; }
    // double GetYmin() const
    //     { return m_ymin; }
    // double GetXmax() const
    //     { return m_xmax; }
    // double GetYmax() const
    //     { return m_ymax; }
};

} // namespace chipper 

class LIBPC_DLL Chipper : public libpc::Filter
{
public:
    Chipper(Stage& prevStage, boost::uint32_t max_partition_size) :
        libpc::Filter(prevStage), m_threshold(max_partition_size),
        m_xvec(chipper::DIR_X), m_yvec(chipper::DIR_Y), m_spare(chipper::DIR_NONE) 
    {
        checkImpedance();
    }

    void Chip();
    std::vector<chipper::Block>::size_type GetBlockCount() const
        { return m_blocks.size(); }
    const chipper::Block& GetBlock(std::vector<chipper::Block>::size_type i) const
        { return m_blocks[i]; }

    const std::string& getName() const ;

    bool supportsIterator (StageIteratorType t) 
    {   
        if (t == StageIterator_Sequential ) return true;
        return false;
    }

    libpc::SequentialIterator* createSequentialIterator() const;
    libpc::RandomIterator* createRandomIterator() const;

private:
    void Load(chipper::RefList& xvec, chipper::RefList& yvec, chipper::RefList& spare);
    void Partition(boost::uint32_t size);
    void Split(chipper::RefList& xvec, chipper::RefList& yvec, chipper::RefList& spare);
    void DecideSplit(chipper::RefList& v1, chipper::RefList& v2, chipper::RefList& spare,
        boost::uint32_t left, boost::uint32_t right);
    void Split(chipper::RefList& wide, chipper::RefList& narrow,chipper::RefList& spare,
        boost::uint32_t left, boost::uint32_t right);
    void FinalSplit(chipper::RefList& wide, chipper::RefList& narrow,
        boost::uint32_t pleft, boost::uint32_t pcenter);
    void Emit(chipper::RefList& wide, boost::uint32_t widemin, boost::uint32_t widemax,
        chipper::RefList& narrow, boost::uint32_t narrowmin, boost::uint32_t narrowmax );

    void checkImpedance();

    boost::uint32_t m_threshold;
    std::vector<chipper::Block> m_blocks;
    std::vector<boost::uint32_t> m_partitions;
    chipper::RefList m_xvec;
    chipper::RefList m_yvec;
    chipper::RefList m_spare;

    Chipper& operator=(const Chipper&); // not implemented
    Chipper(const Chipper&); // not implemented
};

} // namespace filters

} // namespace liblas

#endif