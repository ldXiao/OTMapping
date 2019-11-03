#include "proj_node.h"
#include <igl/embree/line_mesh_intersection.h>
#include <igl/per_vertex_normals.h>
#include <cmath>
#include "kdtree_NN_Eigen.hpp"
#include <igl/barycentric_to_global.h>
namespace bcclean {

    int insertV_baryCord(
        Eigen::MatrixXd & baryentry,
        Eigen::MatrixXd & V,
        Eigen::MatrixXi & F
    )
    {
        int fidx = std::round(baryentry(0,0));
        assert(fidx != -1);
        assert(baryentry.rows()==1);
        int v0 = F(fidx,0);
        int v1 = F(fidx,1);
        int v2 = F(fidx,2);
        int nvidx = V.rows();

        Eigen::MatrixXd nVentry = igl::barycentric_to_global(V, F, baryentry);
        Eigen::MatrixXd nV = Eigen::MatrixXd::Zero(V.rows()+1, 3);
        nV.block(0,0, V.rows(), 3) = V;
        nV.row(V.rows()) = nVentry;

        Eigen::MatrixXi nF = Eigen::MatrixXi::Zero(F.rows()+2,3);
        nF.block(0,0,F.rows(),3) = F;
        /*
            v0

            nv
        v1      v2
        */
       // choose v0 v1 nv to inherit initial face
       nF.row(fidx) = Eigen::RowVector3i(v0, v1, nvidx);
       nF.row(F.rows()) = Eigen::RowVector3i(v1, v2, nvidx);
       nF.row(F.rows()+1) = Eigen::RowVector3i(v2, v0, nvidx);
       F = nF;
       V = nV;

       return nvidx;
    }

    void proj_node(
        const Eigen::MatrixXd & Vbad,
        const Eigen::MatrixXi & Fbad,
        const std::vector<int> & node_list_bad, // indices into Vbad
        Eigen::MatrixXd & Vgood,
        Eigen::MatrixXi & Fgood,
        std::map<int, int> & node_map
    )
    // try to project the nodes into good mesh and modify the good mesh correspondingly
    // if it can not find intersection use nearest neighbor instead 
    // always gaurantee that node_map is injective
    {
        node_map.clear();
        Eigen::MatrixXd node_normal;
        Eigen::MatrixXd node_v;
        {
            Eigen::MatrixXd N;
            igl::per_vertex_normals(Vbad, Fbad, N);
            //
            // slice the N into node_normal;
            node_normal=Eigen::MatrixXd::Zero(N.rows(), 3);
            node_v=Eigen::MatrixXd::Zero(N.rows(), 3);
            int nd_count = 0;
            for(auto nd: node_list_bad)
            {
                node_normal.row(nd_count) = N.row(nd);
                node_v.row(nd_count) = Vbad.row(nd);
                nd_count += 1;
            }
            node_normal.conservativeResize(nd_count,3);
            node_v.conservativeResize(nd_count, 3);
        }
        Eigen::MatrixXd Vgood_copy = Vgood;

        Eigen::MatrixXd RR = igl::embree::line_mesh_intersection(node_v, node_normal, Vgood, Fgood);
        std::vector<int> node_losers;
        // if R(j,0)==-1, it means that normal does not intersect with good mesh
        for(int jj = 0; jj < RR.rows(); ++ jj)
        {
            if(std::round(RR(jj,0))!= -1)
            {
                Eigen::MatrixXd bc = RR.row(jj);
                int nvidx = insertV_baryCord(bc, Vgood, Fgood);
                node_map[node_list_bad[jj]] = nvidx;
            } 
            else
            {
                node_losers.push_back(node_list_bad[jj]);
            }
        }

        if(node_losers.size()>0)
        {
            std::map<int, int> img_records;
            kd_tree_Eigen<double> kdt(Vgood_copy.cols(),std::cref(Vgood_copy),10);
            kdt.index->buildIndex();
            for(auto nd: node_losers)
            {
                Eigen::RowVector3d querypt = Vbad.row(nd);
                int img = kd_tree_NN_Eigen( kdt, querypt);
                assert(img_records.find(img)== img_records.end());
                img_records[img] = 1;
                node_map[nd] = img;
                // TODO remove detect images from kdtree and makesure all the resulting nodes are different
            }
        }

    }
}