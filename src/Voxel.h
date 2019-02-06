#pragma once
#ifndef _VOXEL_H_
#define _VOXEL_H_

class Voxel
{
    vector<Anchor> _nodes;

public:
    Voxel()
    { }


    Voxel(vector<Anchor> nodes)
    {
        _nodes = nodes;
    }

    virtual ~Voxel()
    {
    }

    vector<Anchor> getPRMNodes() const { return _nodes; }
    void setPRMNodes(vector<Anchor> nodes) { _nodes = nodes; }
};

#endif
