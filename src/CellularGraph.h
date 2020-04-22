#ifndef BCCLEAN_CELLGRAPH_H
#define BCCLEAN_CELLGRAPH_H
#include <Eigen/Dense>
#include <vector>
#include <map>
#include <unordered_map>
#include "edge.h"
namespace bcclean{
    class CellularGraph{
        public:
        int label_num;
        int root_cell;
        Eigen::MatrixXd V;
        Eigen::MatrixXi F;
        Eigen::VectorXi FL;
        std::vector<Eigen::RowVector3d> _vertices;
        std::vector<Eigen::RowVector3d> _normals;
        std::vector<int> _nodes; // indices into _vertices
        std::vector<edge> _edge_list;
        std::unordered_map<int, std::vector<int> >  _patch_edge_dict; // Counter clock wise
        std::unordered_map<int, std::vector<bool> > _patch_edge_direction_dict;
        std::map<int, int> _vmap;
        std::map<int, int> _ivmap;
        std::map<int, std::vector<int> > _node_edge_dict; // counter clock wise
        std::map<int, double> _patch_area_dict;
        static CellularGraph GenCellularGraph(
            const Eigen::MatrixXd & V, 
            const Eigen::MatrixXi & F, 
            const Eigen::VectorXi & FL);

    };

    void _gen_node_CCedges_dict(
        const Eigen::MatrixXd & V_bad,
        const Eigen::MatrixXi & F_bad,
        const std::vector<edge> & edge_list,
        const std::vector<int> & node_list_bad,
        std::map<int, std::vector<int> > &  node_edge_dict   
    );

    void CCfaces_per_node(
        const Eigen::MatrixXd & V,
        const Eigen::MatrixXi & F,
        const std::vector<int> & node_list,
        std::map<int, std::vector<int> > & node_faces_dict
    );

    void CCfaces_per_node(
        const std::vector<Eigen::RowVector3d> & V,
        const std::vector<Eigen::RowVector3i> & F,
        const std::vector<std::vector<int> > & VF,
        const std::vector<int> & node_list,
        std::map<int, std::vector<int> > & node_faces_dict
    );

}

#endif //