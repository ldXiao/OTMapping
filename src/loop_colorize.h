#ifndef BCCLEAN_LOOP_COLORIZE_H
#define BCCLEAN_LOOP_COLORIZE_H
#include <Eigen/Core>
#include <vector>
#include <igl/vertex_triangle_adjacency.h> 
#include <queue>
#include <igl/triangle_triangle_adjacency.h>
#include <utility>
#include <tuple>
#include "TraceComplex.h"
namespace bcclean
{
    std::pair<int, double> loop_colorize(
    const std::vector<Eigen::RowVector3d> & V, 
    const std::vector<Eigen::RowVector3i> & F, 
    const std::vector<std::vector<int> > & TEdges,
    const int face_seed,
    const int lb,
    std::vector<int> & FL);

    std::tuple<int, int, double> loop_colorize_top(
    const std::vector<Eigen::RowVector3d> & V, 
    const std::vector<Eigen::RowVector3i> & F, 
    const std::vector<std::vector<int> > & TEdges,
    const int face_seed,
    const int lb,
    std::vector<int> & FL);
}
#endif //BCCLEAN_LOOP_COLORIZE_H