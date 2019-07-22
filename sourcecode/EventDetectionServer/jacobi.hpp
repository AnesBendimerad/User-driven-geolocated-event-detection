//
//  jacobi.hpp
//  jacobiMethod
//
//  Created by Anes Bendimerad on 19/12/2016.
//  Copyright © 2016 Anes Bendimerad. All rights reserved.
//

#ifndef jacobi_hpp
#define jacobi_hpp
#include <vector>
#include <map>
#define MAX_ITER_JACOBI 100
#define MIN_APPROX_JACOBI 0.001
using namespace::std;
void solveEquation(vector<map<int,double> > & matrixA,map<int,double> & bMatrix,vector<double> & result);

#endif /* jacobi_hpp */
