#ifndef MATHTOOLS_H
#define MATHTOOLS_H

#include <vector>
#include "DataBase.h"
// #include <glm/vec3.hpp>

// TODO: make a function converting batches of EventData from input into
//       things that can be rendered in OpenGL
//       (don't worry about rotation matricies for now)
//
// std::vector<glm::vec4> hits convertBatch(EventData* input) {
//     std::vector<glm::vec4*> ret;
//     ret.reserve(2*input->size());
//     for(Event event : input->events) {
//         for(Hit hit : event.hits) {
//             hit.row, hit.col // access with these variables
//         }
//     }

//     return ret;
// }

#endif



/*
// Reference python function: 

def to_space(rvec, pos, theta, dim=None, zero_center=False):
    """Convert pixel vector <rvec> (2xN) to space points (3xN), using chip position <pos> (3) and euler angles <theta> (3)
    """
    
    theta = np.array(theta)*np.pi/180.0
    # pad with zeros
    if zero_center:
        assert dim is not None, "Must pass dimension if zero-centering!"
        rvec = np.array(rvec) - (np.array(dim)/2)[:,None]
        
    rvec = np.pad(np.array(rvec), [[0,1], [0,0]])*0.05 # mm/pixel
    rot_matrix = Rzyx(*theta)
    rot = np.matmul(rot_matrix, rvec)
    p = np.array(pos)[:,None] + rot
    
    return p

*/