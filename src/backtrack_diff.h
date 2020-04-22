#ifndef BCCLEAN_BTDIFF_H
#define BCCLEAN_BTDIFF_H
#include <Eigen/Dense>
#include <map>
#include <unordered_map>
#include <vector>
#include "edge.h"
#include "CellularGraph.h"
namespace bcclean{
namespace MatchMaker{
    bool backtrack_diff(
        const Eigen::MatrixXd & V_good,
        const Eigen::MatrixXd & V_bad,
        const int pidx,
        const std::unordered_map<int, std::vector<int> > & patch_edge_dict,
        const std::vector<edge> & edge_list,
        const std::unordered_map<int, std::vector<bool> > & patch_edge_direction_dict,
        const std::map<int, std::vector<int> > & edge_path_map,
        const double threshold
    );

    double backtrack_diff(
        const std::vector<Eigen::RowVector3d> & V_good,
        const CellularGraph & cg,
        const int pidx,
        const std::map<int, std::vector<int> > & edge_path_map
    );

    double path_len(const std::vector<Eigen::RowVector3d> & V, 
                    const std::vector<int> & path
    );
}
}
#endif 