#include "loop_colorize.h"
#include "helper.h"
#include "Betti.h"
#include <igl/remove_unreferenced.h>
#include <tuple>
namespace bcclean
{
    std::pair<int, double> loop_colorize(
    const std::vector<Eigen::RowVector3d> & V, 
    const std::vector<Eigen::RowVector3i> & F, 
    const std::vector<std::vector<int> > & TEdges,
    const int face_seed,
    const int lb,
    std::vector<int> & FL)
    {
        // return a dble area of the the total colored region
        // for unlabeled faced FL should be -1
        //label the faces encompassed by the loop with lb in FL
        assert(FL.size() == F.size());
        assert(TEdges.size() == F.size());
        assert(FL[face_seed]== -1); //the seed faces has to be uninitialized at first
        // use BFS to search over TTi starting with face_seed
        Eigen::MatrixXi TT, TTi;
        {
            Eigen::MatrixXi F_temp;
            Helper::to_matrix(F, F_temp);
            igl::triangle_triangle_adjacency(F_temp, TT, TTi);
        }
        // start with face 0 use TT and TTi info to get connected components
        // traverse all faces connected to face_seed and do not cross cuts label them to be lb;
        int root_face = face_seed;
        double DblA = 0;
        Eigen::RowVector3d a_ = V.at(F[root_face](1)) - V.at(F[root_face](0));
        Eigen::RowVector3d b_ = V.at(F[root_face](2)) - V.at(F[root_face](0));
        DblA += (a_.cross(b_)).norm();
        std::queue<int> search_queue;
        search_queue.push(face_seed);
        int count = 1;
        std::map<int, bool> visit_dict;
        for(int fidx= 0 ; fidx < F.size(); ++fidx)
        {
            visit_dict[fidx] = false;
        }
        visit_dict[root_face] = true;
        while(search_queue.size()!=0){
            int cur_face = search_queue.front();
            search_queue.pop(); // remove head
            FL[cur_face] = lb;
            Eigen::RowVector3i adjs = TT.row(cur_face);
            for(int j =0 ; j <3 ; ++j){
                int face_j = adjs(j);
                if(face_j == -1) {
                    continue;
                }
                // int vidx, vidy;
                // vidx = Fraw(cur_face, j);
                // vidy  = Fraw(cur_face, (j+1)%3);
                if(TEdges[cur_face][j] == -1){
                    // the edge is not in VCuts
                    if(FL[face_j]== -1 && !(visit_dict[face_j])){
                        // not visited before;
                        search_queue.push(face_j);
                        visit_dict[face_j] = true;
                        FL[cur_face] = lb;
                        Eigen::RowVector3d a__ = V.at(F[face_j](1)) - V.at(F[face_j]( 0));
                        Eigen::RowVector3d b__ = V.at(F[face_j](2)) - V.at(F[face_j](0));
                        DblA += (a__.cross(b__)).norm();
                        count += 1;
                    }
                }
            }
        }

        return std::make_pair(count,DblA);
    }
    std::tuple<int, int, double> loop_colorize_top(
    const std::vector<Eigen::RowVector3d> & V, 
    const std::vector<Eigen::RowVector3i> & F, 
    const std::vector<std::vector<int> > & TEdges,
    const int face_seed,
    const int lb,
    std::vector<int> & FL)
    {
        // return a dble area of the the total colored region
        // for unlabeled faced FL should be -1
        //label the faces encompassed by the loop with lb in FL
        assert(FL.size() == F.size());
        assert(TEdges.size() == F.size());
        assert(FL[face_seed]== -1); //the seed faces has to be uninitialized at first
        // use BFS to search over TTi starting with face_seed
        Eigen::MatrixXi TT, TTi;
        {
            Eigen::MatrixXi F_temp;
            Helper::to_matrix(F, F_temp);
            igl::triangle_triangle_adjacency(F_temp, TT, TTi);
        }
        // start with face 0 use TT and TTi info to get connected components
        // traverse all faces connected to face_seed and do not cross cuts label them to be lb;
        int root_face = face_seed;
        double DblA = 0;
        Eigen::RowVector3d a_ = V.at(F[root_face](1)) - V.at(F[root_face](0));
        Eigen::RowVector3d b_ = V.at(F[root_face](2)) - V.at(F[root_face](0));
        DblA += (a_.cross(b_)).norm();
        std::vector<Eigen::RowVector3i> ColoredF;
        ColoredF.push_back(F[root_face]);
        std::queue<int> search_queue;
        search_queue.push(face_seed);
        int count = 1;
        std::map<int, bool> visit_dict;
        for(int fidx= 0 ; fidx < F.size(); ++fidx)
        {
            visit_dict[fidx] = false;
        }
        visit_dict[root_face] = true;
        while(search_queue.size()!=0){
            int cur_face = search_queue.front();
            search_queue.pop(); // remove head
            FL[cur_face] = lb;
            Eigen::RowVector3i adjs = TT.row(cur_face);
            for(int j =0 ; j <3 ; ++j){
                int face_j = adjs(j);
                if(face_j == -1) {
                    continue;
                }
                // int vidx, vidy;
                // vidx = Fraw(cur_face, j);
                // vidy  = Fraw(cur_face, (j+1)%3);
                if(TEdges[cur_face][j] == -1){
                    // the edge is not in VCuts
                    if(FL[face_j]== -1 && !(visit_dict[face_j])){
                        // not visited before;
                        search_queue.push(face_j);
                        visit_dict[face_j] = true;
                        FL[cur_face] = lb;
                        Eigen::RowVector3d a__ = V.at(F[face_j](1)) - V.at(F[face_j]( 0));
                        Eigen::RowVector3d b__ = V.at(F[face_j](2)) - V.at(F[face_j](0));
                        DblA += (a__.cross(b__)).norm();
                        ColoredF.push_back(F[face_j]);
                        count += 1;
                    }
                }
            }
        }
        int betti = 1;
        Eigen::MatrixXi CF_matrix;
        Eigen::MatrixXd CV_matrix, V_matrix;
        Eigen::VectorXi Ic;
        Helper::to_matrix(ColoredF, CF_matrix);
        Helper::to_matrix(V, V_matrix);
        igl::remove_unreferenced(V_matrix, CF_matrix,CV_matrix, CF_matrix, Ic);
        betti = Betti(CV_matrix, CF_matrix);
        return std::make_tuple(count,betti,DblA);
    }

}
